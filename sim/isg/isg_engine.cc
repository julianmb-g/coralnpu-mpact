// Copyright 2026 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "sim/isg/isg_engine.h"

#include <vector>

#include "sim/coralnpu_m3_user_decoder.h"
#include "sim/coralnpu_v2_state.h"
#include "sim/memory_config.h"
#include "absl/base/no_destructor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "mpact/sim/util/memory/flat_demand_memory.h"

namespace coralnpu {
namespace fuzzer {

std::string CanonicalizeMnemonic(absl::string_view name) {
  std::string lower = absl::AsciiStrToLower(name);

  bool is_nr = absl::StrContains(lower, "_nr") || absl::EndsWith(lower, "nr");

  size_t underscore_pos = lower.find('_');
  if (underscore_pos != std::string::npos) {
    lower = lower.substr(0, underscore_pos);
  }

  if (absl::EndsWith(lower, "nr")) {
    lower = lower.substr(0, lower.length() - 2);
  }

  if (is_nr) {
    if (lower == "csrrw") return "csrw";
    if (lower == "csrrs") return "csrs";
    if (lower == "csrrc") return "csrc";
    if (lower == "csrrwi") return "csrwi";
    if (lower == "csrrsi") return "csrsi";
    if (lower == "csrrci") return "csrci";
  }

  if (absl::EndsWith(lower, "vv") || absl::EndsWith(lower, "vx") ||
      absl::EndsWith(lower, "vi") || absl::EndsWith(lower, "vf") ||
      absl::EndsWith(lower, "wx")) {
    if (!absl::StrContains(lower, ".")) {
      lower.insert(lower.length() - 2, ".");
    }
  } else if (absl::EndsWith(lower, "vvm") || absl::EndsWith(lower, "vxm") ||
             absl::EndsWith(lower, "vim")) {
    if (!absl::StrContains(lower, ".")) {
      lower.insert(lower.length() - 3, ".");
    }
  } else if (absl::StartsWith(lower, "vls") ||
             absl::StartsWith(lower, "vlox") ||
             absl::StartsWith(lower, "vlux") ||
             absl::StartsWith(lower, "vss") ||
             absl::StartsWith(lower, "vsox") ||
             absl::StartsWith(lower, "vsux") ||
             absl::StartsWith(lower, "vle") ||
             (absl::StartsWith(lower, "vse") && lower != "vsetvli" &&
              lower != "vsetivli" && lower != "vsetvl")) {
    if (!absl::StrContains(lower, ".")) {
      lower = absl::StrCat(lower, ".v");
    }
  }
  return lower;
}

IsgEngine::IsgEngine(uint64_t seed)
    : seed_(seed), prng_(seed), watchdog_(8000) {
  memory_ = std::make_unique<::mpact::sim::util::FlatDemandMemory>();
  state_ = std::make_unique<::coralnpu::sim::CoralNPUV2State>(
      "CoralNPUM3", ::mpact::sim::riscv::RiscVXlen::RV32, memory_.get());
  state_->AddMemoryRegion(::coralnpu::sim::kDefaultRxRegionStart,
                          ::coralnpu::sim::kDefaultRxRegionLength,
                          ::coralnpu::sim::kDefaultRxRegionPermission);
  decoder_ = std::make_unique<::coralnpu::sim::CoralNPUM3UserDecoder>(
      state_.get(), memory_.get());
}

IsgEngine::~IsgEngine() = default;

static void EmitLi(IsgEngine* engine, absl::string_view reg, uint32_t val) {
  uint32_t upper = (val + 0x800) >> 12;
  int32_t lower = static_cast<int32_t>(val) - (upper << 12);
  engine->EmitInstructionFormat("lui %s, 0x%x", reg, upper);
  if (lower != 0 || upper == 0) {
    engine->EmitInstructionFormat("addi %s, %s, %d", reg, reg, lower);
  }
}

static const char* ToString(VectorSew sew) {
  switch (sew) {
    case VectorSew::e8:
      return "e8";
    case VectorSew::e16:
      return "e16";
    case VectorSew::e32:
      return "e32";
    case VectorSew::e64:
      return "e64";
  }
  return "e8";
}

static const char* ToString(VectorLmul lmul) {
  switch (lmul) {
    case VectorLmul::mf8:
      return "mf8";
    case VectorLmul::mf4:
      return "mf4";
    case VectorLmul::mf2:
      return "mf2";
    case VectorLmul::m1:
      return "m1";
    case VectorLmul::m2:
      return "m2";
    case VectorLmul::m4:
      return "m4";
    case VectorLmul::m8:
      return "m8";
  }
  return "m1";
}

IsgEngine& IsgEngine::EmitVsetvli(absl::string_view rd, absl::string_view rs1,
                                  VectorSew sew, VectorLmul lmul,
                                  bool tail_agnostic, bool mask_agnostic) {
  vector_state_.configured = true;
  vector_state_.vl = 0xffffffff;
  vector_state_.sew = sew;
  vector_state_.lmul = lmul;
  vector_state_.tail_agnostic = tail_agnostic;
  vector_state_.mask_agnostic = mask_agnostic;
  absl::string_view ta_str = tail_agnostic ? "ta" : "tu";
  absl::string_view ma_str = mask_agnostic ? "ma" : "mu";
  int written = absl::SNPrintF(
      fixed_assembly_buffer_.data() + assembly_length_,
      kAssemblyBufferSize - assembly_length_,
      "vsetvli %.*s, %.*s, %s, %s, %.*s, %.*s\n", static_cast<int>(rd.length()),
      rd.data(), static_cast<int>(rs1.length()), rs1.data(), ToString(sew),
      ToString(lmul), static_cast<int>(ta_str.length()), ta_str.data(),
      static_cast<int>(ma_str.length()), ma_str.data());
  CHECK_GE(written, 0);
  CHECK_LT(assembly_length_ + written, kAssemblyBufferSize);
  assembly_length_ += written;
  return *this;
}

IsgEngine& IsgEngine::EmitVsetivli(absl::string_view rd, uint32_t uimm,
                                   VectorSew sew, VectorLmul lmul,
                                   bool tail_agnostic, bool mask_agnostic) {
  vector_state_.configured = true;
  vector_state_.vl = uimm;
  vector_state_.sew = sew;
  vector_state_.lmul = lmul;
  vector_state_.tail_agnostic = tail_agnostic;
  vector_state_.mask_agnostic = mask_agnostic;
  absl::string_view ta_str = tail_agnostic ? "ta" : "tu";
  absl::string_view ma_str = mask_agnostic ? "ma" : "mu";
  int written = absl::SNPrintF(
      fixed_assembly_buffer_.data() + assembly_length_,
      kAssemblyBufferSize - assembly_length_,
      "vsetivli %.*s, %u, %s, %s, %.*s, %.*s\n", static_cast<int>(rd.length()),
      rd.data(), uimm, ToString(sew), ToString(lmul),
      static_cast<int>(ta_str.length()), ta_str.data(),
      static_cast<int>(ma_str.length()), ma_str.data());
  CHECK_GE(written, 0);
  CHECK_LT(assembly_length_ + written, kAssemblyBufferSize);
  assembly_length_ += written;
  return *this;
}

IsgEngine& IsgEngine::EmitSafePointerInit(
    absl::Span<const absl::string_view> regs) {
  for (absl::string_view reg : regs) {
    uint32_t val =
        kReservedDtcmBytes + (prng_() % (0x1000 - kReservedDtcmBytes));
    EmitLi(this, reg, ::coralnpu::sim::kDefaultRwRegionStart + val);
  }
  return *this;
}

IsgEngine& IsgEngine::EmitInstruction(absl::string_view inst) {
  CHECK_OK(watchdog_.Tick());
  absl::string_view trimmed = absl::StripAsciiWhitespace(inst);
  if (trimmed.length() >= 4 && trimmed.substr(0, 4) == "vset") {
    if (absl::StartsWith(trimmed, "vsetvli zero, zero") ||
        absl::StartsWith(trimmed, "vsetvli zero,zero")) {
      vector_state_.configured = true;
      vector_state_.vl = 0xffffffff;
    } else {
      LOG(WARNING) << "vset is disallowed in EmitInstruction. Use EmitVsetvli.";
      return *this;
    }
  } else if (!trimmed.empty() && trimmed[0] == 'v') {
    if (!vector_state_.configured || vector_state_.vl == 0) {
      uint32_t vl_val = (prng_() % 32) + 1;
      int vl_written =
          absl::SNPrintF(fixed_assembly_buffer_.data() + assembly_length_,
                         kAssemblyBufferSize - assembly_length_,
                         "addi t3, zero, %u\n", vl_val);
      CHECK_GE(vl_written, 0);
      CHECK_LT(assembly_length_ + vl_written, kAssemblyBufferSize);
      assembly_length_ += vl_written;
      EmitVsetvli("t2", "t3", vector_state_.sew, vector_state_.lmul);
      vector_state_.vl = vl_val;
      EmitSafePointerInit({"t2", "t3"});
    }
  }
  int inst_written =
      absl::SNPrintF(fixed_assembly_buffer_.data() + assembly_length_,
                     kAssemblyBufferSize - assembly_length_, "%.*s\n",
                     static_cast<int>(inst.length()), inst.data());
  CHECK_GE(inst_written, 0);
  CHECK_LT(assembly_length_ + inst_written, kAssemblyBufferSize);
  assembly_length_ += inst_written;
  return *this;
}

IsgEngine& IsgEngine::EmitMpause() {
  int written = absl::SNPrintF(fixed_assembly_buffer_.data() + assembly_length_,
                               kAssemblyBufferSize - assembly_length_,
                               ".word 0x08000073\n");
  CHECK_GE(written, 0);
  CHECK_LT(assembly_length_ + written, kAssemblyBufferSize);
  assembly_length_ += written;
  return *this;
}

uint32_t IsgEngine::CurrentPc() const {
  uint32_t pc = 0x0;
  absl::string_view text(fixed_assembly_buffer_.data(), assembly_length_);
  for (absl::string_view subline :
       absl::StrSplit(text, '\n', absl::SkipEmpty())) {
    absl::string_view trimmed = absl::StripAsciiWhitespace(subline);
    if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';') continue;
    if (trimmed.back() == ':') continue;
    if (trimmed[0] == '.') {
      if (absl::StartsWith(trimmed, ".word")) {
        pc += 4;
      }
      continue;
    }
    pc += 4;
  }
  return pc;
}

TestSequence IsgEngine::Build() const {
  TestSequence seq;
  seq.set_prng_seed(seed_);
  if (expected_trap_.has_value()) {
    *seq.mutable_trap_event() = *expected_trap_;
  }
  std::string final_text(fixed_assembly_buffer_.data(), assembly_length_);
  absl::StrAppend(&final_text,
                  "_mutable_end:\n; END MUTABLE\n; BEGIN PROTECTED\n");
  uint32_t pc = CurrentPc();
  CHECK(pc <= 8188) << "Instruction sequence exceeds 8kB ITCM limit: " << pc
                    << " bytes";
  while (pc < 8188) {
    absl::StrAppend(&final_text, "addi x0, x0, 0\n");
    pc += 4;
  }
  absl::StrAppend(&final_text, ".word 0x08000073\n; END PROTECTED\n");
  seq.set_assembly_text(final_text);
  seq.set_expected_terminal_state("mpause");
  seq.set_assertion_count(assertion_count_);
  for (const auto& tag : hazard_tags_) {
    seq.add_hazard_tags(tag);
  }
  return seq;
}

IsgEngine& IsgEngine::EmitPreamble() {
  auto append = [&](absl::string_view str) {
    int written =
        absl::SNPrintF(fixed_assembly_buffer_.data() + assembly_length_,
                       kAssemblyBufferSize - assembly_length_, "%.*s",
                       static_cast<int>(str.length()), str.data());
    CHECK_GE(written, 0);
    assembly_length_ += written;
  };
  append("; BEGIN PROTECTED\n.text\n.global _start\n_start:\n");
  EmitInstruction("jal zero, 0x70");
  EmitInstruction("csrrw t0, mscratch, t0");
  EmitInstruction("csrrw t0, mtval, t0");
  EmitInstruction("lui t0, 0x10");
  EmitInstruction("sw t1, 4(t0)");
  EmitInstruction("sw t2, 8(t0)");
  EmitInstruction("csrrs t1, mscratch, zero");
  EmitInstruction("sw t1, 0(t0)");
  EmitInstruction("csrrs t1, mtval, zero");
  EmitInstruction("sw t1, 20(t0)");
  EmitInstruction("csrrs t1, mcause, zero");
  EmitInstruction("sw t1, 12(t0)");
  EmitInstruction("csrrs t1, mepc, zero");
  EmitInstruction("sw t1, 16(t0)");
  EmitInstruction("lw t1, 12(t0)");
  EmitInstruction("addi t2, zero, 3");
  uint32_t ebreak_addr = CurrentPc() + 20;
  EmitInstructionFormat("beq t1, t2, %#x", ebreak_addr);
  EmitInstruction("lui t1, 0x2");
  EmitInstruction("addi t1, t1, -4");
  EmitInstruction("csrw mepc, t1");
  uint32_t restore_addr = CurrentPc() + 8;
  EmitInstructionFormat("jal zero, %#x", restore_addr);
  EmitInstruction("ebreak");
  EmitInstruction("lw t1, 20(t0)");
  EmitInstruction("csrw mscratch, t1");
  EmitInstruction("lw t1, 4(t0)");
  EmitInstruction("lw t2, 8(t0)");
  EmitInstruction("lw t0, 0(t0)");
  EmitInstruction("mret");
  SetupTrapHandler(0x4);
  EmitLi(this, "t0", ::coralnpu::sim::kDefaultRwRegionStart);
  {
    uint32_t sv = prng_();
    if (sv == 0) sv = 1;
    EmitLi(this, "t1", sv);
  }
  EmitLi(this, "t2", 8192);
  uint32_t loop_start_pc = CurrentPc();
  append("1:\n");
  EmitInstruction("sw t1, 0(t0)");
  EmitInstruction("slli t3, t1, 0xd");
  EmitInstruction("xor t1, t1, t3");
  EmitInstruction("srli t3, t1, 0x11");
  EmitInstruction("xor t1, t1, t3");
  EmitInstruction("slli t3, t1, 0x5");
  EmitInstruction("xor t1, t1, t3");
  EmitInstruction("addi t0, t0, 4");
  EmitInstruction("addi t2, t2, -1");
  EmitInstructionFormat("bne t2, zero, 0x%x", loop_start_pc);
  EmitLi(this, "t0", 0x6600);
  EmitInstruction("csrw mstatus, t0");
  for (int i = 1; i < 32; ++i) {
    char reg_buf[16];
    absl::SNPrintF(reg_buf, sizeof(reg_buf), "x%d", i);
    EmitLi(this, reg_buf, static_cast<uint32_t>(prng_()));
  }
  uint32_t fcsr_val = ((prng_() % 5) << 5) | (prng_() % 32);
  EmitInstructionFormat("addi t0, zero, %u", fcsr_val);
  EmitInstruction("csrw fcsr, t0");
  EmitLi(this, "t0", prng_());
  EmitInstruction("csrw mscratch, t0");
  EmitLi(this, "t0", prng_());
  EmitInstruction("csrw mepc, t0");
  EmitLi(this, "t0", prng_());
  EmitInstruction("csrw mcause, t0");
  EmitInstructionFormat("addi t0, zero, %u", prng_() % 4);
  EmitInstruction("csrw vxrm, t0");
  EmitInstructionFormat("addi t0, zero, %u", prng_() % 2);
  EmitInstruction("csrw vxsat, t0");
  EmitInstruction("csrw vstart, zero");
  for (int i = 0; i < 32; ++i) {
    EmitLi(this, "t0", static_cast<uint32_t>(prng_()));
    EmitInstructionFormat("mv.w.x f%d, t0", i);
  }
  vector_state_.sew = VectorSew::e8;
  vector_state_.lmul = VectorLmul::m8;
  vector_state_.configured = false;
  vector_state_.vl = 0;
  EmitLi(this, "t0",
         ::coralnpu::sim::kDefaultRwRegionStart + kReservedDtcmBytes);
  for (int i = 0; i < 32; i += 8) {
    EmitInstructionFormat("vle8.v v%d,(t0)", i);
    EmitInstruction("addi t0, t0, 1024");
  }
  EmitLi(this, "t0",
         ::coralnpu::sim::kDefaultRwRegionStart + kReservedDtcmBytes +
             (prng_() % (0x1000 - kReservedDtcmBytes)));
  EmitLi(this, "t1",
         ::coralnpu::sim::kDefaultRwRegionStart + kReservedDtcmBytes +
             (prng_() % (0x1000 - kReservedDtcmBytes)));
  EmitLi(this, "t2",
         ::coralnpu::sim::kDefaultRwRegionStart + kReservedDtcmBytes +
             (prng_() % (0x1000 - kReservedDtcmBytes)));
  EmitLi(this, "t3",
         ::coralnpu::sim::kDefaultRwRegionStart + kReservedDtcmBytes +
             (prng_() % (0x1000 - kReservedDtcmBytes)));
  EmitLi(this, "sp",
         ::coralnpu::sim::kDefaultRwRegionStart +
             ::coralnpu::sim::kDefaultRwRegionLength);
  append("; END PROTECTED\n; BEGIN MUTABLE\n_mutable_start:\n");
  return *this;
}
IsgEngine& IsgEngine::EmitCoverpoint(absl::string_view name) {
  int written = absl::SNPrintF(fixed_assembly_buffer_.data() + assembly_length_,
                               kAssemblyBufferSize - assembly_length_,
                               "# COVERPOINT: %.*s\n",
                               static_cast<int>(name.length()), name.data());
  CHECK_GE(written, 0);
  CHECK_LT(assembly_length_ + written, kAssemblyBufferSize);
  assembly_length_ += written;
  return *this;
}
IsgEngine& IsgEngine::AddHazardTag(absl::string_view tag) {
  hazard_tags_.emplace_back(tag);
  return *this;
}
IsgEngine& IsgEngine::EmitEbreak() {
  EmitInstruction("ebreak");
  return *this;
}
IsgEngine& IsgEngine::EmitIllegalInstruction() {
  int written = absl::SNPrintF(fixed_assembly_buffer_.data() + assembly_length_,
                               kAssemblyBufferSize - assembly_length_,
                               ".word 0x00000000\n");
  CHECK_GE(written, 0);
  CHECK_LT(assembly_length_ + written, kAssemblyBufferSize);
  assembly_length_ += written;
  return *this;
}
IsgEngine& IsgEngine::SetupTrapHandler(uint32_t trap_addr) {
  EmitInstructionFormat("addi t0, zero, %u", trap_addr);
  EmitInstruction("csrw mtvec, t0");
  return *this;
}
IsgEngine& IsgEngine::SetExpectedTrap(uint64_t mcause, uint64_t mepc,
                                      uint64_t mtval) {
  expected_trap_.emplace();
  expected_trap_->set_mcause(mcause);
  expected_trap_->set_mepc(mepc);
  expected_trap_->set_mtval(mtval);
  return *this;
}
VectorBlockBuilder IsgEngine::BeginVectorBlock() {
  return VectorBlockBuilder(this);
}
MemoryBlockBuilder IsgEngine::BeginMemoryBlock() {
  return MemoryBlockBuilder(this);
}
IsgEngine& IsgEngine::EmitDataHazard(
    ::coralnpu::sim::isa32_m3::OpcodeEnum op1,
    ::coralnpu::sim::isa32_m3::OpcodeEnum op2) {
  static constexpr int kNumOpcodes =
      static_cast<int>(::coralnpu::sim::isa32_m3::OpcodeEnum::kPastMaxValue);
  static constexpr int kMaxMnemonicLen = 32;
  struct CanonicalizedMnemonics {
    char buffer[kNumOpcodes][kMaxMnemonicLen];
    std::array<absl::string_view, kNumOpcodes> mnemonics;
    CanonicalizedMnemonics() {
      for (int i = 0; i < kNumOpcodes; ++i) {
        std::string canonical =
            CanonicalizeMnemonic(::coralnpu::sim::isa32_m3::kOpcodeNames[i]);
        int len = absl::SNPrintF(buffer[i], kMaxMnemonicLen, "%s", canonical);
        if (len >= kMaxMnemonicLen) {
          len = kMaxMnemonicLen - 1;
        }
        mnemonics[i] = absl::string_view(buffer[i], len);
      }
    }
  };
  static const absl::NoDestructor<CanonicalizedMnemonics>
      kCanonicalizedMnemonics;
  absl::string_view mnemonic1 =
      kCanonicalizedMnemonics->mnemonics[static_cast<int>(op1)];
  absl::string_view mnemonic2 =
      kCanonicalizedMnemonics->mnemonics[static_cast<int>(op2)];
  EmitInstructionFormat("%s t0, t1, t2", mnemonic1);
  EmitInstructionFormat("%s t3, t0, t4", mnemonic2);
  return *this;
}
IsgEngine& IsgEngine::EmitFpPoisonPills() {
  int written = absl::SNPrintF(fixed_assembly_buffer_.data() + assembly_length_,
                               kAssemblyBufferSize - assembly_length_,
                               "; COVERPOINT: FP Poison Pills\n");
  CHECK_GE(written, 0);
  CHECK_LT(assembly_length_ + written, kAssemblyBufferSize);
  assembly_length_ += written;
  EmitLi(this, "t0", 0xFFFFFFFF);
  EmitInstruction("mv.w.x f1, t0");
  EmitLi(this, "t0", 0x7F800001);
  EmitInstruction("mv.w.x f2, t0");
  EmitLi(this, "t0", 0x00000001);
  EmitInstruction("mv.w.x f3, t0");
  EmitInstruction("mv.x.w t1, f1");
  EmitInstruction("mv.w.x f4, t1");
  EmitInstruction("mv.x.w t2, f2");
  EmitInstruction("mv.w.x f5, t2");
  EmitInstruction("mv.x.w t3, f3");
  EmitInstruction("mv.w.x f6, t3");
  EmitInstruction("fadd.s f7, f1, f2");
  EmitInstruction("fsub.s f8, f2, f3");
  EmitInstruction("fmul.s f9, f1, f3");
  EmitInstruction("fdiv.s f10, f1, f1");
  return *this;
}
IsgEngine& IsgEngine::EmitVtype(int sew, int lmul, int vl) {
  VectorSew v_sew = VectorSew::e8;
  switch (sew) {
    case 8:
      v_sew = VectorSew::e8;
      break;
    case 16:
      v_sew = VectorSew::e16;
      break;
    case 32:
      v_sew = VectorSew::e32;
      break;
    case 64:
      v_sew = VectorSew::e64;
      break;
    default:
      LOG(FATAL) << "Invalid SEW for EmitVtype: " << sew;
  }
  VectorLmul v_lmul = VectorLmul::m1;
  switch (lmul) {
    case 1:
      v_lmul = VectorLmul::m1;
      break;
    case 2:
      v_lmul = VectorLmul::m2;
      break;
    case 4:
      v_lmul = VectorLmul::m4;
      break;
    case 8:
      v_lmul = VectorLmul::m8;
      break;
    default:
      LOG(FATAL) << "Unsupported LMUL for EmitVtype prototype: " << lmul;
  }
  if (vl != -1) {
    EmitLi(this, "t3", static_cast<uint32_t>(vl));
    EmitVsetvli("zero", "t3", v_sew, v_lmul);
    vector_state_.vl = vl;
    return *this;
  }
  return EmitVsetvli("zero", "zero", v_sew, v_lmul);
}
void IsgEngine::Finalize(::coralnpu::sim::proto::Database* db,
                         std::string* output) {
  *db->add_test_sequences() = Build();
  if (output != nullptr) {
    ::google::protobuf::io::StringOutputStream raw_out(output);
    ::google::protobuf::io::CodedOutputStream coded_out(&raw_out);
    coded_out.SetSerializationDeterministic(true);
    db->SerializeToCodedStream(&coded_out);
  }
}
void IsgEngine::Reset() {
  assembly_length_ = 0;
  vector_state_ = VectorStateTracker();
  hazard_tags_.clear();
  assertion_count_ = 0;
  prng_.seed(seed_);
  watchdog_.Reset();
}
static float GetLmulRatio(VectorLmul lmul) {
  switch (lmul) {
    case VectorLmul::mf8:
      return 0.125f;
    case VectorLmul::mf4:
      return 0.25f;
    case VectorLmul::mf2:
      return 0.5f;
    case VectorLmul::m1:
      return 1.0f;
    case VectorLmul::m2:
      return 2.0f;
    case VectorLmul::m4:
      return 4.0f;
    case VectorLmul::m8:
      return 8.0f;
  }
  return 1.0f;
}
static float GetSewBits(VectorSew sew) {
  switch (sew) {
    case VectorSew::e8:
      return 8.0f;
    case VectorSew::e16:
      return 16.0f;
    case VectorSew::e32:
      return 32.0f;
    case VectorSew::e64:
      return 64.0f;
  }
  return 8.0f;
}
static VectorLmul CalculateEmul(VectorSew sew, VectorLmul lmul, uint32_t eew,
                                IsgEngine* engine) {
  float emul_ratio =
      (static_cast<float>(eew) / GetSewBits(sew)) * GetLmulRatio(lmul);
  if (engine && (engine->prng()() % 16 == 0)) {
    return VectorLmul::m1;
  }
  if (emul_ratio > 8.0f) {
    engine->EmitVsetvli("zero", "zero", VectorSew::e32, VectorLmul::m8);
    return VectorLmul::m8;
  }
  if (emul_ratio >= 8.0f) return VectorLmul::m8;
  if (emul_ratio >= 4.0f) return VectorLmul::m4;
  if (emul_ratio >= 2.0f) return VectorLmul::m2;
  if (emul_ratio >= 1.0f) return VectorLmul::m1;
  if (emul_ratio >= 0.5f) return VectorLmul::mf2;
  if (emul_ratio >= 0.25f) return VectorLmul::mf4;
  return VectorLmul::mf8;
}
static constexpr absl::string_view kVectorRegNames[] = {
    "v0",  "v1",  "v2",  "v3",  "v4",  "v5",  "v6",  "v7",  "v8",  "v9",  "v10",
    "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21",
    "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31"};
static absl::string_view AlignVectorReg(absl::string_view reg_str,
                                        VectorLmul lmul,
                                        IsgEngine* engine = nullptr) {
  if (reg_str.empty() || reg_str[0] != 'v') return reg_str;
  uint32_t reg_num;
  if (!absl::SimpleAtoi(reg_str.substr(1), &reg_num)) return reg_str;
  if (engine && (engine->prng()() % 16 == 0)) {
    if (engine->prng()() % 2 == 0) {
      reg_num = 0;
    }
  }
  uint32_t align = 1;
  if (lmul == VectorLmul::m2)
    align = 2;
  else if (lmul == VectorLmul::m4)
    align = 4;
  else if (lmul == VectorLmul::m8)
    align = 8;
  reg_num &= ~(align - 1);
  reg_num %= 32;
  return kVectorRegNames[reg_num];
}
static void EmitDynamicMask(IsgEngine* engine, uint32_t offset_size) {
  if (!engine->strict_sandboxing()) {
    return;
  }
  uint32_t r1_mask =
      ::coralnpu::sim::kDefaultRwRegionLength > offset_size
          ? ::coralnpu::sim::kDefaultRwRegionLength - offset_size + 1
          : 1;
  uint32_t r2_mask =
      ::coralnpu::sim::kDefaultRwRegion2Length > offset_size
          ? ::coralnpu::sim::kDefaultRwRegion2Length - offset_size + 1
          : 1;
  uint32_t diff = r2_mask - r1_mask;
  engine->EmitInstruction("addi sp, sp, -16");
  engine->EmitInstruction("sw t1, 0(sp)");
  engine->EmitInstruction("sw t2, 4(sp)");
  engine->EmitInstruction("sw t3, 8(sp)");
  engine->EmitInstruction("sw t5, 12(sp)");
  engine->EmitInstruction("srli t5, t6, 0x1d");
  engine->EmitInstruction("slli t5, t5, 0x1d");
  engine->EmitInstruction("lui t1, 0x20000");
  engine->EmitInstruction("and t5, t5, t1");
  engine->EmitInstruction("sltiu t1, t5, 1");
  engine->EmitInstruction("sub t3, zero, t1");
  uint32_t diff_upper = (diff + 0x800) >> 12;
  int32_t diff_lower = static_cast<int32_t>(diff) - (diff_upper << 12);
  engine->EmitInstructionFormat("lui t2, 0x%x", diff_upper);
  if (diff_lower != 0) {
    engine->EmitInstructionFormat("addi t2, t2, %d", diff_lower);
  }
  engine->EmitInstruction("and t2, t2, t3");
  uint32_t r2_upper = (r2_mask + 0x800) >> 12;
  int32_t r2_lower = static_cast<int32_t>(r2_mask) - (r2_upper << 12);
  engine->EmitInstructionFormat("lui t1, 0x%x", r2_upper);
  if (r2_lower != 0) {
    engine->EmitInstructionFormat("addi t1, t1, %d", r2_lower);
  }
  engine->EmitInstruction("sub t2, t1, t2");
  engine->EmitInstruction("lui t1, 0x10");
  engine->EmitInstruction("and t1, t1, t3");
  engine->EmitInstruction("or t5, t5, t1");
  engine->EmitInstruction("sub t6, t6, t5");
  // Ensure t2 is not zero to prevent remu hang.
  engine->EmitInstruction("sltiu t1, t2, 1");  // t1 = (t2 == 0)
  engine->EmitInstruction("add t2, t2, t1");   // t2 = t2 + (t2 == 0)
  engine->EmitInstruction("remu t6, t6, t2");
  if (r1_mask > IsgEngine::kReservedDtcmBytes) {
    engine->EmitInstructionFormat("sltiu t1, t6, %u",
                                  IsgEngine::kReservedDtcmBytes);
    engine->EmitInstruction("sub t1, zero, t1");
    engine->EmitInstruction("and t1, t1, t3");
    engine->EmitInstructionFormat("addi t2, zero, %u",
                                  IsgEngine::kReservedDtcmBytes);
    engine->EmitInstruction("sub t2, t2, t6");
    engine->EmitInstruction("and t2, t2, t1");
    engine->EmitInstruction("add t6, t6, t2");
  }
  engine->EmitInstruction("add t6, t6, t5");
  engine->EmitInstruction("lw t1, 0(sp)");
  engine->EmitInstruction("lw t2, 4(sp)");
  engine->EmitInstruction("lw t3, 8(sp)");
  engine->EmitInstruction("lw t5, 12(sp)");
  engine->EmitInstruction("addi sp, sp, 16");
}
VectorBlockBuilder& VectorBlockBuilder::EmitVadd(absl::string_view vd,
                                                 absl::string_view vs2,
                                                 absl::string_view vs1,
                                                 bool masked) {
  absl::string_view aligned_vd =
      AlignVectorReg(vd, engine_->vector_state().lmul, engine_);
  absl::string_view aligned_vs2 =
      AlignVectorReg(vs2, engine_->vector_state().lmul, engine_);
  absl::string_view aligned_vs1 =
      AlignVectorReg(vs1, engine_->vector_state().lmul, engine_);
  engine_->EmitInstructionFormat("vadd.vv %s, %s, %s%s", aligned_vd,
                                 aligned_vs2, aligned_vs1,
                                 masked ? ", v0.t" : "");
  return *this;
}
VectorBlockBuilder& VectorBlockBuilder::EmitVmul(absl::string_view vd,
                                                 absl::string_view vs2,
                                                 absl::string_view vs1,
                                                 bool masked) {
  absl::string_view aligned_vd =
      AlignVectorReg(vd, engine_->vector_state().lmul, engine_);
  absl::string_view aligned_vs2 =
      AlignVectorReg(vs2, engine_->vector_state().lmul, engine_);
  absl::string_view aligned_vs1 =
      AlignVectorReg(vs1, engine_->vector_state().lmul, engine_);
  engine_->EmitInstructionFormat("vmul.vv %s, %s, %s%s", aligned_vd,
                                 aligned_vs2, aligned_vs1,
                                 masked ? ", v0.t" : "");
  return *this;
}
VectorBlockBuilder& VectorBlockBuilder::EmitVsub(absl::string_view vd,
                                                 absl::string_view vs2,
                                                 absl::string_view vs1,
                                                 bool masked) {
  absl::string_view aligned_vd =
      AlignVectorReg(vd, engine_->vector_state().lmul, engine_);
  absl::string_view aligned_vs2 =
      AlignVectorReg(vs2, engine_->vector_state().lmul, engine_);
  absl::string_view aligned_vs1 =
      AlignVectorReg(vs1, engine_->vector_state().lmul, engine_);
  engine_->EmitInstructionFormat("vsub.vv %s, %s, %s%s", aligned_vd,
                                 aligned_vs2, aligned_vs1,
                                 masked ? ", v0.t" : "");
  return *this;
}
VectorBlockBuilder& VectorBlockBuilder::EmitVdiv(absl::string_view vd,
                                                 absl::string_view vs2,
                                                 absl::string_view vs1,
                                                 bool masked) {
  absl::string_view aligned_vd =
      AlignVectorReg(vd, engine_->vector_state().lmul, engine_);
  absl::string_view aligned_vs2 =
      AlignVectorReg(vs2, engine_->vector_state().lmul, engine_);
  absl::string_view aligned_vs1 =
      AlignVectorReg(vs1, engine_->vector_state().lmul, engine_);
  engine_->EmitInstructionFormat("vdiv.vv %s, %s, %s%s", aligned_vd,
                                 aligned_vs2, aligned_vs1,
                                 masked ? ", v0.t" : "");
  return *this;
}
MemoryBlockBuilder& MemoryBlockBuilder::EmitLoad(absl::string_view rd,
                                                 absl::string_view rs1,
                                                 int32_t imm) {
  uint32_t uimm = static_cast<uint32_t>(imm);
  uint32_t imm_upper = (uimm + 0x800) >> 12;
  int32_t imm_lower = static_cast<int32_t>(uimm) - (imm_upper << 12);
  absl::string_view scratch = (rs1 == "t6") ? "t1" : "t6";
  engine_->EmitInstructionFormat("lui %s, 0x%x", scratch, imm_upper);
  if (imm_lower != 0) {
    engine_->EmitInstructionFormat("addi %s, %s, %d", scratch, scratch,
                                   imm_lower);
  }
  engine_->EmitInstructionFormat("add t6, %s, %s", rs1, scratch);
  EmitDynamicMask(engine_, 4);
  engine_->EmitInstructionFormat("lw %s, 0(t6)", rd);
  return *this;
}
MemoryBlockBuilder& MemoryBlockBuilder::EmitStore(absl::string_view rs2,
                                                  absl::string_view rs1,
                                                  int32_t imm) {
  uint32_t uimm = static_cast<uint32_t>(imm);
  uint32_t imm_upper = (uimm + 0x800) >> 12;
  int32_t imm_lower = static_cast<int32_t>(uimm) - (imm_upper << 12);
  absl::string_view scratch = "t1";
  if (rs1 == "t1" || rs2 == "t1") {
    scratch = (rs1 == "t2" || rs2 == "t2") ? "t3" : "t2";
  }
  engine_->EmitInstructionFormat("lui %s, 0x%x", scratch, imm_upper);
  engine_->EmitInstructionFormat("addi %s, %s, %d", scratch, scratch,
                                 imm_lower);
  engine_->EmitInstructionFormat("add t6, %s, %s", rs1, scratch);
  engine_->EmitInstructionFormat("addi t4, %s, 0", rs2);
  EmitDynamicMask(engine_, 4);
  engine_->EmitInstructionFormat("sw t4, 0(t6)");
  return *this;
}
MemoryBlockBuilder& MemoryBlockBuilder::EmitVectorLoadStrided(
    absl::string_view vd, absl::string_view rs1, absl::string_view rs2) {
  engine_->EmitInstructionFormat("andi t4, %s, 0x7", rs2);
  engine_->EmitInstructionFormat("addi t6, %s, 0", rs1);
  EmitDynamicMask(engine_, 2048);
  VectorLmul emul = CalculateEmul(engine_->vector_state().sew,
                                  engine_->vector_state().lmul, 32, engine_);
  absl::string_view aligned_vd = AlignVectorReg(vd, emul, engine_);
  engine_->EmitInstructionFormat("vlse32.v %s, (t6), t4", aligned_vd);
  return *this;
}
MemoryBlockBuilder& MemoryBlockBuilder::EmitVectorLoadIndexed(
    absl::string_view vd, absl::string_view rs1, absl::string_view vs2) {
  VectorLmul emul = CalculateEmul(engine_->vector_state().sew,
                                  engine_->vector_state().lmul, 32, engine_);
  absl::string_view aligned_vd =
      AlignVectorReg(vd, engine_->vector_state().lmul, engine_);
  absl::string_view aligned_vs2 = AlignVectorReg(vs2, emul, engine_);
  absl::string_view temp_vs2 = AlignVectorReg("v31", emul, engine_);
  if (temp_vs2 == aligned_vd || temp_vs2 == aligned_vs2) {
    temp_vs2 = AlignVectorReg("v15", emul, engine_);
    if (temp_vs2 == aligned_vd || temp_vs2 == aligned_vs2) {
      temp_vs2 = AlignVectorReg("v23", emul, engine_);
    }
  }
  absl::string_view scratch = (rs1 == "t1") ? "t4" : "t1";
  engine_->EmitInstructionFormat("addi %s, zero, 0xFF", scratch);
  engine_->EmitInstructionFormat("vand.vx %s, %s, %s", temp_vs2, aligned_vs2,
                                 scratch);
  engine_->EmitInstructionFormat("addi t6, %s, 0", rs1);
  EmitDynamicMask(engine_, 0x400);
  engine_->EmitInstructionFormat("vloxei32.v %s, (t6), %s", aligned_vd,
                                 temp_vs2);
  return *this;
}

}  // namespace fuzzer
}  // namespace coralnpu