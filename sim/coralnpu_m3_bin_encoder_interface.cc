// Copyright 2026 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "sim/coralnpu_m3_bin_encoder_interface.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

#include "sim/coralnpu_m3_bin_encoder.h"
#include "sim/coralnpu_m3_encoder.h"
#include "sim/coralnpu_m3_enums.h"
#include "absl/base/no_destructor.h"
#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "mpact/sim/util/asm/opcode_assembler_interface.h"
#include "mpact/sim/util/asm/resolver_interface.h"

namespace coralnpu {
namespace sim {
namespace isa32_m3 {

using ::mpact::sim::util::assembler::RelocationInfo;
using ::mpact::sim::util::assembler::ResolverInterface;

CoralNPUM3BinEncoderInterface::CoralNPUM3BinEncoderInterface() = default;

absl::StatusOr<std::tuple<uint64_t, int>>
CoralNPUM3BinEncoderInterface::GetOpcodeEncoding(SlotEnum slot, int entry,
                                                 OpcodeEnum opcode,
                                                 ResolverInterface* resolver) {
  auto it = encoding_m3::kOpcodeEncodings->find(opcode);
  if (it == encoding_m3::kOpcodeEncodings->end()) {
    return absl::NotFoundError(
        absl::StrCat("Opcode not found: ", static_cast<int>(opcode)));
  }
  return it->second;
}

absl::StatusOr<uint64_t> CoralNPUM3BinEncoderInterface::ParseReg(
    absl::string_view text_in, SourceOpEnum src_op) const {
  absl::string_view text = absl::StripAsciiWhitespace(text_in);
  if (absl::StartsWith(text, ",")) {
    text = absl::StripAsciiWhitespace(text.substr(1));
  }
  if (text.empty()) {
    if (src_op == SourceOpEnum::kVmask) return 1;  // unmasked
    if (src_op == SourceOpEnum::kRm) return 7;     // dyn
    return 0;
  }
  if (text == "iorw") return 15;
  if (text == "ior") return 14;
  if (text == "iow") return 13;
  if (text == "io") return 12;
  if (text == "irw") return 11;
  if (text == "ir") return 10;
  if (text == "iw") return 9;
  if (text == "i") return 8;
  if (text == "orw") return 7;
  if (text == "or") return 6;
  if (text == "ow") return 5;
  if (text == "o") return 4;
  if (text == "rw") return 3;
  if (text == "r") return 2;
  if (text == "w") return 1;

  absl::string_view s = text;
  if (absl::EndsWith(s, ".t")) {
    if (s != "v0.t") {
      return absl::InvalidArgumentError(absl::StrCat(
          "Invalid mask register: ", s, ". Only v0.t is allowed."));
    }
    return 0;
  }
  if (s == "rne") return 0;
  if (s == "rtz") return 1;
  if (s == "rdn") return 2;
  if (s == "rup") return 3;
  if (s == "rmm") return 4;
  if (s == "dyn") return 7;

  if (s == "e8") return 0;
  if (s == "e16") return 1;
  if (s == "e32") return 2;

  if (s == "m1") return 0;
  if (s == "m2") return 1;
  if (s == "m4") return 2;
  if (s == "m8") return 3;
  if (s == "res4") return 4;
  if (s == "mf8") return 5;
  if (s == "mf4") return 6;
  if (s == "mf2") return 7;

  if (s == "ta") return 1;
  if (s == "tu") return 0;
  if (s == "ma") return 1;
  if (s == "mu") return 0;

  if (s == "fflags") return 1;
  if (s == "frm") return 2;
  if (s == "fcsr") return 3;
  if (s == "vstart") return 8;
  if (s == "vxsat") return 9;
  if (s == "vxrm") return 10;
  if (s == "vcsr") return 15;
  if (s == "vl") return 0xc20;
  if (s == "vtype") return 0xc21;
  if (s == "vlenb") return 0xc22;
  if (s == "cycle") return 0xc00;
  if (s == "time") return 0xc01;
  if (s == "instret") return 0xc02;
  if (s == "fcsr") return 0x003;
  if (s == "vstart") return 0x008;
  if (s == "satp") return 0x180;
  if (s == "mstatus") return 0x300;
  if (s == "mscratch") return 0x340;
  if (s == "mie") return 0x304;
  if (s == "mtvec") return 0x305;
  if (s == "mepc") return 0x341;
  if (s == "mcause") return 0x342;
  if (s == "mtval") return 0x343;
  if (s == "mip") return 0x344;

  // TODO(julianmb): The scalar registers (x0-x31, f0-f31, mstatus) defined here
  // duplicate logic already present in
  // third_party/mpact_riscv/riscv_bin_setters.h. We currently maintain this
  // unified dictionary because mpact_riscv is missing vector CSRs (vstart, vl,
  // vtype, etc.) and custom NPU operands (m4, e32, dyn). In the future, we
  // should enhance mpact_riscv to support these vector extensions and refactor
  // this assembler to reuse that upstream logic.
  static const absl::NoDestructor<absl::flat_hash_map<std::string, uint64_t>>
      kRegs({{"x0", 0},
             {"x1", 1},
             {"x2", 2},
             {"x3", 3},
             {"x4", 4},
             {"x5", 5},
             {"x6", 6},
             {"x7", 7},
             {"x8", 8},
             {"x9", 9},
             {"x10", 10},
             {"x11", 11},
             {"x12", 12},
             {"x13", 13},
             {"x14", 14},
             {"x15", 15},
             {"x16", 16},
             {"x17", 17},
             {"x18", 18},
             {"x19", 19},
             {"x20", 20},
             {"x21", 21},
             {"x22", 22},
             {"x23", 23},
             {"x24", 24},
             {"x25", 25},
             {"x26", 26},
             {"x27", 27},
             {"x28", 28},
             {"x29", 29},
             {"x30", 30},
             {"x31", 31},
             {"zero", 0},
             {"ra", 1},
             {"sp", 2},
             {"gp", 3},
             {"tp", 4},
             {"t0", 5},
             {"t1", 6},
             {"t2", 7},
             {"s0", 8},
             {"s1", 9},
             {"a0", 10},
             {"a1", 11},
             {"a2", 12},
             {"a3", 13},
             {"a4", 14},
             {"a5", 15},
             {"a6", 16},
             {"a7", 17},
             {"s2", 18},
             {"s3", 19},
             {"s4", 20},
             {"s5", 21},
             {"s6", 22},
             {"s7", 23},
             {"s8", 24},
             {"s9", 25},
             {"s10", 26},
             {"s11", 27},
             {"t3", 28},
             {"t4", 29},
             {"t5", 30},
             {"t6", 31},
             {"f0", 0},
             {"f1", 1},
             {"f2", 2},
             {"f3", 3},
             {"f4", 4},
             {"f5", 5},
             {"f6", 6},
             {"f7", 7},
             {"f8", 8},
             {"f9", 9},
             {"f10", 10},
             {"f11", 11},
             {"f12", 12},
             {"f13", 13},
             {"f14", 14},
             {"f15", 15},
             {"f16", 16},
             {"f17", 17},
             {"f18", 18},
             {"f19", 19},
             {"f20", 20},
             {"f21", 21},
             {"f22", 22},
             {"f23", 23},
             {"f24", 24},
             {"f25", 25},
             {"f26", 26},
             {"f27", 27},
             {"f28", 28},
             {"f29", 29},
             {"f30", 30},
             {"f31", 31},
             {"ft0", 0},
             {"ft1", 1},
             {"ft2", 2},
             {"ft3", 3},
             {"ft4", 4},
             {"ft5", 5},
             {"ft6", 6},
             {"ft7", 7},
             {"fs0", 8},
             {"fs1", 9},
             {"fa0", 10},
             {"fa1", 11},
             {"fa2", 12},
             {"fa3", 13},
             {"fa4", 14},
             {"fa5", 15},
             {"fa6", 16},
             {"fa7", 17},
             {"fs2", 18},
             {"fs3", 19},
             {"fs4", 20},
             {"fs5", 21},
             {"fs6", 22},
             {"fs7", 23},
             {"fs8", 24},
             {"fs9", 25},
             {"fs10", 26},
             {"fs11", 27},
             {"ft8", 28},
             {"ft9", 29},
             {"ft10", 30},
             {"ft11", 31},
             {"v0", 0},
             {"v1", 1},
             {"v2", 2},
             {"v3", 3},
             {"v4", 4},
             {"v5", 5},
             {"v6", 6},
             {"v7", 7},
             {"v8", 8},
             {"v9", 9},
             {"v10", 10},
             {"v11", 11},
             {"v12", 12},
             {"v13", 13},
             {"v14", 14},
             {"v15", 15},
             {"v16", 16},
             {"v17", 17},
             {"v18", 18},
             {"v19", 19},
             {"v20", 20},
             {"v21", 21},
             {"v22", 22},
             {"v23", 23},
             {"v24", 24},
             {"v25", 25},
             {"v26", 26},
             {"v27", 27},
             {"v28", 28},
             {"v29", 29},
             {"v30", 30},
             {"v31", 31},
             // CSR aliases supported by the decoder
             {"fflags", 0x001},
             {"frm", 0x002},
             {"fcsr", 0x003},
             {"vl", 0xC20},
             {"vtype", 0xC21},
             {"vlenb", 0xC22},
             {"vstart", 0x008},
             {"vxsat", 0x009},
             {"vxrm", 0x00A},
             {"vcsr", 0x00F},
             {"cycle", 0xC00},
             {"time", 0xC01},
             {"instret", 0xC02},
             {"mstatus", 0x300},
             {"mie", 0x304},
             {"mepc", 0x341},
             {"mtvec", 0x305},
             {"satp", 0x180},
             {"mscratch", 0x340},
             {"mcause", 0x342},
             {"mtval", 0x343},
             {"mip", 0x344},
             {"hpmcounter3", 0xC03},
             {"hpmcounter4", 0xC04},
             {"hpmcounter5", 0xC05},
             {"hpmcounter6", 0xC06},
             {"hpmcounter7", 0xC07},
             {"hpmcounter8", 0xC08},
             {"hpmcounter9", 0xC09},
             {"hpmcounter10", 0xC0A},
             {"hpmcounter11", 0xC0B},
             {"hpmcounter12", 0xC0C},
             {"hpmcounter13", 0xC0D},
             {"hpmcounter14", 0xC0E},
             {"hpmcounter15", 0xC0F},
             {"hpmcounter16", 0xC10},
             {"hpmcounter17", 0xC11},
             {"hpmcounter18", 0xC12},
             {"hpmcounter19", 0xC13},
             {"hpmcounter20", 0xC14},
             {"hpmcounter21", 0xC15},
             {"hpmcounter22", 0xC16},
             {"hpmcounter23", 0xC17},
             {"hpmcounter24", 0xC18},
             {"hpmcounter25", 0xC19},
             {"hpmcounter26", 0xC1A},
             {"hpmcounter27", 0xC1B},
             {"hpmcounter28", 0xC1C},
             {"hpmcounter29", 0xC1D},
             {"hpmcounter30", 0xC1E},
             {"hpmcounter31", 0xC1F}});

  size_t paren_start = s.find('(');
  size_t paren_end = s.find(')');
  if (paren_start != std::string::npos && paren_end != std::string::npos &&
      paren_end > paren_start) {
    if (src_op == SourceOpEnum::kIImm12 || src_op == SourceOpEnum::kSImm12 ||
        src_op == SourceOpEnum::kBImm12 || src_op == SourceOpEnum::kJImm12 ||
        src_op == SourceOpEnum::kSimm5 || src_op == SourceOpEnum::kUimm5) {
      s = s.substr(0, paren_start);
    } else {
      s = s.substr(paren_start + 1, paren_end - paren_start - 1);
    }
  }

  if (auto it = kRegs->find(s); it != kRegs->end()) return it->second;

  // Parse dynamic CSRs (hpmcounter3-31, mhpmcounter3-31, mhpmevent3-31, and
  // their RV32 high counterparts).
  if (absl::StartsWith(s, "hpmcounter") || absl::StartsWith(s, "mhpmcounter") ||
      absl::StartsWith(s, "mhpmevent")) {
    bool is_m = absl::StartsWith(s, "m");
    absl::string_view prefix =
        is_m
            ? (absl::StartsWith(s, "mhpmcounter") ? "mhpmcounter" : "mhpmevent")
            : "hpmcounter";
    absl::string_view suffix = absl::string_view(s).substr(prefix.size());
    bool is_h = absl::EndsWith(suffix, "h");
    if (is_h) {
      suffix = suffix.substr(0, suffix.size() - 1);
    }
    int n = 0;
    if (absl::SimpleAtoi(suffix, &n) && n >= 3 && n <= 31) {
      if (prefix == "hpmcounter") {
        return (is_h ? 0xC80 : 0xC00) + n;
      } else if (prefix == "mhpmcounter") {
        return (is_h ? 0xB80 : 0xB00) + n;
      } else if (prefix == "mhpmevent") {
        return (is_h ? 0x720 : 0x320) + n;
      }
    }
  }

  size_t comment_pos = s.find('<');
  std::string hex_str = (comment_pos != std::string::npos)
                            ? std::string(s.substr(0, comment_pos))
                            : std::string(s);
  hex_str = absl::StripAsciiWhitespace(hex_str);

  if (absl::StartsWith(hex_str, "0x") || absl::StartsWith(hex_str, "0X")) {
    uint64_t v = 0;
    if (absl::SimpleHexAtoi(hex_str.substr(2), &v)) return v;
    return 0;
  }

  // If the source operand is one of the hex-prefixed immediates in the ISA,
  // we must parse it as hex first even if it doesn't start with "0x" (because
  // the regex stripped the literal "0x" prefix).
  if (src_op == SourceOpEnum::kUImm20 || src_op == SourceOpEnum::kBImm12 ||
      src_op == SourceOpEnum::kJImm20) {
    uint64_t hex_val = 0;
    if (absl::SimpleHexAtoi(hex_str, &hex_val)) return hex_val;
  }

  int64_t val = 0;
  if (absl::SimpleAtoi(hex_str, &val)) return val;

  uint64_t hex_val = 0;
  if (absl::SimpleHexAtoi(hex_str, &hex_val)) return hex_val;

  return absl::InvalidArgumentError(
      absl::StrCat("Invalid register or immediate: ", text_in));
}

absl::StatusOr<uint64_t> CoralNPUM3BinEncoderInterface::GetSrcOpEncoding(
    uint64_t address, absl::string_view text, SlotEnum slot, int entry,
    OpcodeEnum opcode, SourceOpEnum source_op, int source_num,
    ResolverInterface* resolver) {
  auto val_or = ParseReg(text, source_op);
  if (!val_or.ok()) return val_or.status();
  uint64_t val = *val_or;
  switch (source_op) {
    case SourceOpEnum::kRs1:
    case SourceOpEnum::kFrs1:
    case SourceOpEnum::kVs1:
      if (opcode == OpcodeEnum::kVsetvliXn && val == 0) {
        return absl::InvalidArgumentError("vsetvli_xn requires rs1 != 0");
      }
      if (opcode == OpcodeEnum::kVsetvliNz && val != 0) {
        return absl::InvalidArgumentError("vsetvli_nz requires rs1 == 0");
      }
      if (opcode == OpcodeEnum::kVsetvliZz && val != 0) {
        return absl::InvalidArgumentError("vsetvli_zz requires rs1 == 0");
      }
      return encoding_m3::Encoder::Inst32Format::InsertVs1(val, 0ULL);
    case SourceOpEnum::kRs2:
    case SourceOpEnum::kFrs2:
    case SourceOpEnum::kVs2:
      return encoding_m3::Encoder::RType::InsertRs2(val, 0ULL);
    case SourceOpEnum::kFrs3:
      return encoding_m3::Encoder::R4Type::InsertRs3(val, 0ULL);
    case SourceOpEnum::kVs3:
    case SourceOpEnum::kVd:
      return encoding_m3::Encoder::Inst32Format::InsertVd(val, 0ULL);
    case SourceOpEnum::kRm:
      return encoding_m3::Encoder::Inst32Format::InsertFunc3(val, 0ULL);
    case SourceOpEnum::kVmask:
      return encoding_m3::Encoder::Inst32Format::InsertVm(val, 0ULL);
    case SourceOpEnum::kIImm12:
    case SourceOpEnum::kCsr:
    case SourceOpEnum::kJImm12:
      return encoding_m3::Encoder::Inst32Format::InsertImm12(val, 0ULL);
    case SourceOpEnum::kSImm12:
      return encoding_m3::Encoder::Inst32Format::InsertSImm(val, 0ULL);
    case SourceOpEnum::kBImm12: {
      int64_t signed_val = static_cast<int64_t>(val);
      int64_t offset = signed_val;
      // If the parsed value is already a relative offset (e.g. negative or
      // small), do not subtract address
      if (signed_val < 0) {
        offset = signed_val;
      } else {
        offset = signed_val - static_cast<int64_t>(address);
      }
      return encoding_m3::Encoder::Inst32Format::InsertBImm(offset, 0ULL);
    }
    case SourceOpEnum::kUImm20:
      return encoding_m3::Encoder::Inst32Format::InsertImm20(val, 0ULL);
    case SourceOpEnum::kJImm20:
      return encoding_m3::Encoder::Inst32Format::InsertJImm(
          static_cast<int64_t>(val) - static_cast<int64_t>(address), 0ULL);
    case SourceOpEnum::kCSRUimm5:
    case SourceOpEnum::kSimm5:
    case SourceOpEnum::kUimm5:
      return encoding_m3::Encoder::Inst32Format::InsertUimm5(val, 0ULL);
    case SourceOpEnum::kRUimm5:
      return encoding_m3::Encoder::Inst32Format::InsertRUimm5(val, 0ULL);
    case SourceOpEnum::kSucc:
      return encoding_m3::Encoder::Inst32Format::InsertSucc(val, 0ULL);
    case SourceOpEnum::kPred:
      return encoding_m3::Encoder::Inst32Format::InsertPred(val, 0ULL);
    case SourceOpEnum::kNf:
      return encoding_m3::Encoder::Inst32Format::InsertNf(val - 1, 0ULL);
    case SourceOpEnum::kZimm11:
      return encoding_m3::Encoder::VConfig::InsertZimm11(val, 0ULL);
    case SourceOpEnum::kZimm10:
      return encoding_m3::Encoder::VConfig::InsertZimm10(val, 0ULL);

    // The following operands are implicit semantic literals or markers used
    // by the execution getters (e.g., ImmediateOperand<uint32_t>(4) or
    // RV32VectorTrueOperand). They do not correspond to any bits in the
    // physical instruction encoding, so we simply return 0.
    case SourceOpEnum::kConst1:
    case SourceOpEnum::kConst2:
    case SourceOpEnum::kConst4:
    case SourceOpEnum::kVmaskTrue:
    case SourceOpEnum::kNone:
      return 0ULL;

    default:
      break;
  }
  return absl::InvalidArgumentError(
      absl::StrCat("Invalid source op: ", static_cast<int>(source_op)));
}

absl::Status CoralNPUM3BinEncoderInterface::AppendSrcOpRelocation(
    uint64_t address, absl::string_view text, SlotEnum slot, int entry,
    OpcodeEnum opcode, SourceOpEnum source_op, int source_num,
    ResolverInterface* resolver, std::vector<RelocationInfo>& relocations) {
  return absl::OkStatus();
}

absl::StatusOr<uint64_t> CoralNPUM3BinEncoderInterface::GetDestOpEncoding(
    uint64_t address, absl::string_view text, SlotEnum slot, int entry,
    OpcodeEnum opcode, DestOpEnum dest_op, int dest_num,
    ResolverInterface* resolver) {
  absl::StatusOr<uint64_t> val_or = ParseReg(text);
  if (!val_or.ok()) return val_or.status();
  uint64_t val = *val_or;
  switch (dest_op) {
    case DestOpEnum::kRd:
    case DestOpEnum::kFrd:
      if (opcode == OpcodeEnum::kVsetvliNz && val == 0) {
        return absl::InvalidArgumentError("vsetvli_nz requires rd != 0");
      }
      if (opcode == OpcodeEnum::kVsetvliZz && val != 0) {
        return absl::InvalidArgumentError("vsetvli_zz requires rd == 0");
      }
      return encoding_m3::Encoder::RType::InsertRd(val, 0ULL);
    case DestOpEnum::kVd:
      return encoding_m3::Encoder::Inst32Format::InsertVd(val, 0ULL);
    case DestOpEnum::kCsr:
      return encoding_m3::Encoder::Inst32Format::InsertImm12(val, 0ULL);
    case DestOpEnum::kFflags:
    case DestOpEnum::kNextPc:
    case DestOpEnum::kNone:
      return 0ULL;
    default:
      break;
  }
  return absl::InvalidArgumentError("Invalid dest op");
}

absl::Status CoralNPUM3BinEncoderInterface::AppendDestOpRelocation(
    uint64_t address, absl::string_view text, SlotEnum slot, int entry,
    OpcodeEnum opcode, DestOpEnum dest_op, int dest_num,
    ResolverInterface* resolver, std::vector<RelocationInfo>& relocations) {
  return absl::OkStatus();
}

absl::StatusOr<uint64_t> CoralNPUM3BinEncoderInterface::GetListSrcOpEncoding(
    uint64_t /*address*/, absl::string_view /*text*/, SlotEnum /*slot*/,
    int /*entry*/, OpcodeEnum /*opcode*/, ListSourceOpEnum /*source_op*/,
    int /*source_num*/, ResolverInterface* /*resolver*/) {
  return absl::InvalidArgumentError("Not implemented");
}

absl::StatusOr<uint64_t> CoralNPUM3BinEncoderInterface::GetListDestOpEncoding(
    uint64_t /*address*/, absl::string_view /*text*/, SlotEnum /*slot*/,
    int /*entry*/, OpcodeEnum /*opcode*/, ListDestOpEnum /*dest_op*/,
    int /*dest_num*/, ResolverInterface* /*resolver*/) {
  return absl::InvalidArgumentError("Not implemented");
}

absl::StatusOr<uint64_t> CoralNPUM3BinEncoderInterface::GetPredOpEncoding(
    uint64_t /*address*/, absl::string_view text_in, SlotEnum /*slot*/,
    int /*entry*/, OpcodeEnum /*opcode*/, PredOpEnum /*pred_op*/,
    ResolverInterface* /*resolver*/) {
  absl::string_view text = absl::StripAsciiWhitespace(text_in);
  if (absl::StartsWith(text, ",")) {
    text = absl::StripAsciiWhitespace(text.substr(1));
  }
  if (text == "iorw") return 15;
  if (text == "ior") return 14;
  if (text == "iow") return 13;
  if (text == "io") return 12;
  if (text == "irw") return 11;
  if (text == "ir") return 10;
  if (text == "iw") return 9;
  if (text == "i") return 8;
  if (text == "orw") return 7;
  if (text == "or") return 6;
  if (text == "ow") return 5;
  if (text == "o") return 4;
  if (text == "rw") return 3;
  if (text == "r") return 2;
  if (text == "w") return 1;
  return absl::InvalidArgumentError("Not implemented");
}

}  // namespace isa32_m3
}  // namespace sim
}  // namespace coralnpu
