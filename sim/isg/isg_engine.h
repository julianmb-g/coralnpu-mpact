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

#ifndef KELVIN_SIM_ISG_ISG_ENGINE_H_
#define KELVIN_SIM_ISG_ISG_ENGINE_H_

#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include "sim/coralnpu_m3_enums.h"
#include "sim/isg/isg_watchdog.h"
#include "sim/proto/isg_database.pb.h"
#include "absl/log/check.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"

namespace mpact {
namespace sim {
namespace util {
class FlatDemandMemory;
}  // namespace util
}  // namespace sim
}  // namespace mpact

namespace coralnpu {
namespace sim {
class CoralNPUV2State;
class CoralNPUM3UserDecoder;
}  // namespace sim
}  // namespace coralnpu

namespace coralnpu {
namespace fuzzer {

using ::coralnpu::sim::proto::TestSequence;

std::string CanonicalizeMnemonic(absl::string_view name);

// Tracks the current RISC-V Vector state constraints.
enum class VectorSew { e8, e16, e32, e64 };
enum class VectorLmul { mf8, mf4, mf2, m1, m2, m4, m8 };

struct VectorStateTracker {
  uint32_t vl = 0;
  VectorSew sew = VectorSew::e8;
  VectorLmul lmul = VectorLmul::m1;
  bool configured = false;
  bool tail_agnostic = true;
  bool mask_agnostic = true;
};

class IsgEngine;

class BlockBuilder {
 public:
  virtual ~BlockBuilder() = default;
  IsgEngine& EndBlock() { return *engine_; }

 protected:
  explicit BlockBuilder(IsgEngine* engine) : engine_(engine) {}
  IsgEngine* engine_;
};

class VectorBlockBuilder : public BlockBuilder {
 public:
  explicit VectorBlockBuilder(IsgEngine* engine) : BlockBuilder(engine) {}
  VectorBlockBuilder& EmitVadd(absl::string_view vd, absl::string_view vs2,
                               absl::string_view vs1, bool masked = false);
  VectorBlockBuilder& EmitVmul(absl::string_view vd, absl::string_view vs2,
                               absl::string_view vs1, bool masked = false);
  VectorBlockBuilder& EmitVsub(absl::string_view vd, absl::string_view vs2,
                               absl::string_view vs1, bool masked = false);
  VectorBlockBuilder& EmitVdiv(absl::string_view vd, absl::string_view vs2,
                               absl::string_view vs1, bool masked = false);
};

class MemoryBlockBuilder : public BlockBuilder {
 public:
  explicit MemoryBlockBuilder(IsgEngine* engine) : BlockBuilder(engine) {}
  MemoryBlockBuilder& EmitLoad(absl::string_view rd, absl::string_view rs1,
                               int32_t imm);
  MemoryBlockBuilder& EmitStore(absl::string_view rs2, absl::string_view rs1,
                                int32_t imm);
  MemoryBlockBuilder& EmitVectorLoadStrided(absl::string_view vd,
                                            absl::string_view rs1,
                                            absl::string_view rs2);
  MemoryBlockBuilder& EmitVectorLoadIndexed(absl::string_view vd,
                                            absl::string_view rs1,
                                            absl::string_view vs2);
};

// Instruction Stream Generator (ISG) Engine
class IsgEngine {
 public:
  explicit IsgEngine(uint64_t seed);
  ~IsgEngine();

  // Initialize the assembly sequence with a preamble that sets up registers and
  // traps.
  IsgEngine& EmitPreamble();

  // Fluent Builder API for generating instructions
  IsgEngine& EmitInstruction(absl::string_view inst);

  template <typename... Args>
  IsgEngine& EmitInstructionFormat(const absl::FormatSpec<Args...>& format,
                                   const Args&... args) {
    CHECK_OK(watchdog_.Tick());
    size_t fixed_start_pos = assembly_length_;

    int written =
        absl::SNPrintF(fixed_assembly_buffer_.data() + assembly_length_,
                       kAssemblyBufferSize - assembly_length_, format, args...);
    CHECK_GE(written, 0);
    CHECK_LT(assembly_length_ + written, kAssemblyBufferSize);

    absl::string_view inst(fixed_assembly_buffer_.data() + fixed_start_pos,
                           written);
    absl::string_view trimmed = absl::StripAsciiWhitespace(inst);

    if (trimmed.length() >= 4 && trimmed.substr(0, 4) == "vset") {
      if (absl::StartsWith(trimmed, "vsetvli zero, zero") ||
          absl::StartsWith(trimmed, "vsetvli zero,zero")) {
        vector_state_.configured = true;
        vector_state_.vl = 0xffffffff;
      } else {
        CHECK(false)
            << "vset is disallowed in EmitInstruction. Use EmitVsetvli.";
      }
      assembly_length_ += written;
    } else if (!trimmed.empty() && trimmed[0] == 'v') {
      if (!vector_state_.configured || vector_state_.vl == 0) {
        // fixed_assembly_buffer_ length is not updated yet, so it reverts to
        // fixed_start_pos

        uint32_t vl_val = (prng_() % 32) + 1;

        int vl_written =
            absl::SNPrintF(fixed_assembly_buffer_.data() + assembly_length_,
                           kAssemblyBufferSize - assembly_length_,
                           "addi t3, zero, %u\n", vl_val);
        CHECK_GE(vl_written, 0);
        assembly_length_ += vl_written;

        EmitVsetvli("t2", "t3", vector_state_.sew, vector_state_.lmul);

        vector_state_.vl = vl_val;
        EmitSafePointerInit({"t2", "t3"});

        int final_written = absl::SNPrintF(
            fixed_assembly_buffer_.data() + assembly_length_,
            kAssemblyBufferSize - assembly_length_, format, args...);
        CHECK_GE(final_written, 0);
        assembly_length_ += final_written;
      } else {
        assembly_length_ += written;
      }
    } else {
      assembly_length_ += written;
    }

    int nl_written =
        absl::SNPrintF(fixed_assembly_buffer_.data() + assembly_length_,
                       kAssemblyBufferSize - assembly_length_, "\n");
    CHECK_GE(nl_written, 0);
    assembly_length_ += nl_written;

    return *this;
  }

  // High-level generators
  IsgEngine& EmitVsetvli(absl::string_view rd, absl::string_view rs1,
                         VectorSew sew, VectorLmul lmul,
                         bool tail_agnostic = true, bool mask_agnostic = true);
  IsgEngine& EmitVsetivli(absl::string_view rd, uint32_t uimm, VectorSew sew,
                          VectorLmul lmul, bool tail_agnostic = true,
                          bool mask_agnostic = true);
  IsgEngine& EmitEbreak();
  IsgEngine& EmitMpause();
  IsgEngine& EmitIllegalInstruction();

  IsgEngine& EmitDataHazard(::coralnpu::sim::isa32_m3::OpcodeEnum op1,
                            ::coralnpu::sim::isa32_m3::OpcodeEnum op2);
  IsgEngine& EmitFpPoisonPills();
  IsgEngine& EmitVtype(int sew, int lmul, int vl = -1);
  IsgEngine& SetExpectedTrap(uint64_t mcause, uint64_t mepc, uint64_t mtval);
  void Finalize(::coralnpu::sim::proto::Database* db,
                std::string* output = nullptr);
  void Reset();
  void SetSeed(uint64_t seed) {
    seed_ = seed;
    prng_.seed(seed);
  }

  // Programmatic coverpoints
  IsgEngine& EmitCoverpoint(absl::string_view name);
  IsgEngine& EmitSafePointerInit(absl::Span<const absl::string_view> regs);
  IsgEngine& AddHazardTag(absl::string_view tag);

  // Set the trap handler address
  IsgEngine& SetupTrapHandler(uint32_t trap_addr);

  // Extract the generated Protobuf TestSequence
  TestSequence Build() const;

  // Calculate the current Program Counter (PC) of the generated assembly.
  uint32_t CurrentPc() const;

  VectorBlockBuilder BeginVectorBlock();
  MemoryBlockBuilder BeginMemoryBlock();

  // PRNG access for custom generation logic
  std::mt19937_64& prng() { return prng_; }

  static constexpr uint32_t kReservedDtcmBytes = 512;
  static constexpr size_t kAssemblyBufferSize = 131072;

  const VectorStateTracker& vector_state() const { return vector_state_; }

  bool strict_sandboxing() const { return strict_sandboxing_; }
  void set_strict_sandboxing(bool strict) { strict_sandboxing_ = strict; }

  ::mpact::sim::util::FlatDemandMemory* shared_memory() const {
    return memory_.get();
  }
  ::coralnpu::sim::CoralNPUV2State* shared_state() const {
    return state_.get();
  }
  ::coralnpu::sim::CoralNPUM3UserDecoder* shared_decoder() const {
    return decoder_.get();
  }

 private:
  friend class VectorBlockBuilder;
  friend class MemoryBlockBuilder;
  uint64_t seed_;
  std::mt19937_64 prng_;
  std::array<char, kAssemblyBufferSize> fixed_assembly_buffer_;
  size_t assembly_length_ = 0;
  VectorStateTracker vector_state_;
  bool strict_sandboxing_ = true;
  IsgWatchdog watchdog_;
  uint32_t assertion_count_ = 0;
  std::vector<std::string> hazard_tags_;
  std::optional<::coralnpu::sim::proto::TrapEvent> expected_trap_;

  std::unique_ptr<::mpact::sim::util::FlatDemandMemory> memory_;
  std::unique_ptr<::coralnpu::sim::CoralNPUV2State> state_;
  std::unique_ptr<::coralnpu::sim::CoralNPUM3UserDecoder> decoder_;
};

}  // namespace fuzzer
}  // namespace coralnpu

#endif  // KELVIN_SIM_ISG_ISG_ENGINE_H_
