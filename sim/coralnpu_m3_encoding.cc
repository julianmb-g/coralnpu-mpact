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

#include "sim/coralnpu_m3_encoding.h"

#include <cstdint>

#include "sim/coralnpu_m3_bin_decoder.h"
#include "sim/coralnpu_m3_enums.h"
#include "sim/coralnpu_v2_encoding_template.h"
#include "sim/coralnpu_v2_state.h"
#include "absl/types/span.h"
#include "riscv/riscv_register.h"
#include "riscv/riscv_vector_state.h"
#include "mpact/sim/generic/operand_interface.h"
#include "mpact/sim/generic/register.h"

namespace coralnpu::sim {

CoralNPUM3Encoding::CoralNPUM3Encoding(CoralNPUV2State* state) : Base(state) {
  static constexpr const char* const kVectorRegNames[32] = {
      "v0",  "v1",  "v2",  "v3",  "v4",  "v5",  "v6",  "v7",
      "v8",  "v9",  "v10", "v11", "v12", "v13", "v14", "v15",
      "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
      "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31"};
  for (int i = 0; i < 32; ++i) {
    vregs_[i] = state_
                    ->GetRegister<mpact::sim::riscv::RVVectorRegister>(
                        kVectorRegNames[i])
                    .first;
  }
}

void CoralNPUM3Encoding::ParseInstruction(uint32_t inst_word) {
  CoralNPUV2EncodingTemplate::ParseInstruction(
      inst_word, ::coralnpu::sim::encoding_m3::DecodeCoralNPUM3Inst32);
}

::mpact::sim::generic::RegisterBase* CoralNPUM3Encoding::GetVReg(int index) {
  return vregs_[index];
}

::mpact::sim::generic::SourceOperandInterface* CoralNPUM3Encoding::GetSource(
    ::coralnpu::sim::isa32_m3::SlotEnum slot, int entry,
    ::coralnpu::sim::isa32_m3::OpcodeEnum opcode,
    ::coralnpu::sim::isa32_m3::SourceOpEnum op, int source_no) {
  // We must check kVs2 here because custom logic below might return early
  // and bypass the systemic fix in the base template. Other operands
  // fall through to the base template which safely rejects reserved VLMUL.
  if (op == ::coralnpu::sim::isa32_m3::SourceOpEnum::kVs2 &&
      state_ != nullptr && state_->rv_vector() != nullptr &&
      (state_->rv_vector()->vtype() & 0x7) == 4) {
    return nullptr;
  }

  int widen_factor = 1;
  if (op == ::coralnpu::sim::isa32_m3::SourceOpEnum::kVs2) {
    switch (opcode) {
      case ::coralnpu::sim::isa32_m3::OpcodeEnum::kVfncvtbf16FFW:
      case ::coralnpu::sim::isa32_m3::OpcodeEnum::kVfncvtXuFW:
      case ::coralnpu::sim::isa32_m3::OpcodeEnum::kVfncvtXFW:
      case ::coralnpu::sim::isa32_m3::OpcodeEnum::kVfncvtFXuW:
      case ::coralnpu::sim::isa32_m3::OpcodeEnum::kVfncvtFXW:
      case ::coralnpu::sim::isa32_m3::OpcodeEnum::kVfncvtFFW:
      case ::coralnpu::sim::isa32_m3::OpcodeEnum::kVfncvtRodFFW:
      case ::coralnpu::sim::isa32_m3::OpcodeEnum::kVfncvtRtzXuFW:
      case ::coralnpu::sim::isa32_m3::OpcodeEnum::kVfncvtRtzXFW:
        widen_factor = 2;
        break;
      default:
        break;
    }
  }

  if (widen_factor > 1) {
    auto reg_num = ::coralnpu::sim::encoding_m3::Extractors::VArith::ExtractVs2(
        inst_word_);
    bool strip_mine = (inst_word_ >> 25) & 0x1;
    int regs_count = (strip_mine ? 4 : 1) * widen_factor;
    if (widen_factor == 2) {
      uint32_t vlmul = (state_ != nullptr && state_->rv_vector() != nullptr)
                           ? (state_->rv_vector()->vtype() & 0x7)
                           : 0;
      if (vlmul > 4) {
        int denom = 1 << (8 - vlmul);
        regs_count = (widen_factor > denom) ? (widen_factor / denom) : 1;
      } else {
        regs_count = (1 << vlmul) * widen_factor;
      }
    }

    if (reg_num % regs_count != 0) {
      return nullptr;
    }
    return new mpact::sim::riscv::RV32VectorSourceOperand(
        absl::Span<mpact::sim::generic::RegisterBase*>(&vregs_[reg_num],
                                                       regs_count),
        vregs_[reg_num]->name());
  }

  if (opcode == ::coralnpu::sim::isa32_m3::OpcodeEnum::kVfwcvtbf16FFV) {
    if (op == ::coralnpu::sim::isa32_m3::SourceOpEnum::kVs2) {
      uint32_t vlmul = (state_ != nullptr && state_->rv_vector() != nullptr)
                           ? (state_->rv_vector()->vtype() & 0x7)
                           : 0;
      int regs_count = (vlmul < 4) ? (1 << vlmul) : 1;
      auto reg_num =
          ::coralnpu::sim::encoding_m3::Extractors::VArith::ExtractVs2(
              inst_word_);
      if (reg_num % regs_count != 0) return nullptr;
      return new mpact::sim::riscv::RV32VectorSourceOperand(
          absl::Span<mpact::sim::generic::RegisterBase*>(&vregs_[reg_num],
                                                         regs_count),
          vregs_[reg_num]->name());
    }
  }

  return Base::GetSource(slot, entry, opcode, op, source_no);
}

::mpact::sim::generic::DestinationOperandInterface*
CoralNPUM3Encoding::GetDestination(::coralnpu::sim::isa32_m3::SlotEnum slot,
                                   int entry,
                                   ::coralnpu::sim::isa32_m3::OpcodeEnum opcode,
                                   ::coralnpu::sim::isa32_m3::DestOpEnum op,
                                   int dest_no, int latency) {
  // We must check kVd here because custom logic below might return early
  // and bypass the systemic fix in the base template.
  if (op == ::coralnpu::sim::isa32_m3::DestOpEnum::kVd && state_ != nullptr &&
      state_->rv_vector() != nullptr &&
      (state_->rv_vector()->vtype() & 0x7) == 4) {
    return nullptr;
  }

  int widen_factor = 1;
  if (op == ::coralnpu::sim::isa32_m3::DestOpEnum::kVd) {
    switch (opcode) {
      case ::coralnpu::sim::isa32_m3::OpcodeEnum::kVfwcvtbf16FFV:
      case ::coralnpu::sim::isa32_m3::OpcodeEnum::kVfwcvtFFV:
      case ::coralnpu::sim::isa32_m3::OpcodeEnum::kVfwcvtXuFV:
      case ::coralnpu::sim::isa32_m3::OpcodeEnum::kVfwcvtXFV:
      case ::coralnpu::sim::isa32_m3::OpcodeEnum::kVfwcvtFXuV:
      case ::coralnpu::sim::isa32_m3::OpcodeEnum::kVfwcvtFXV:
      case ::coralnpu::sim::isa32_m3::OpcodeEnum::kVfwcvtRtzXuFV:
      case ::coralnpu::sim::isa32_m3::OpcodeEnum::kVfwcvtRtzXFV:
        widen_factor = 2;
        break;
      default:
        break;
    }
  }

  if (widen_factor > 1) {
    auto reg_num =
        ::coralnpu::sim::encoding_m3::Extractors::VArith::ExtractVd(inst_word_);
    bool strip_mine = (inst_word_ >> 25) & 0x1;
    int regs_count = (strip_mine ? 4 : 1) * widen_factor;
    if (widen_factor == 2) {
      uint32_t vlmul = (state_ != nullptr && state_->rv_vector() != nullptr)
                           ? (state_->rv_vector()->vtype() & 0x7)
                           : 0;
      if (vlmul > 4) {
        int denom = 1 << (8 - vlmul);
        regs_count = (widen_factor > denom) ? (widen_factor / denom) : 1;
      } else {
        regs_count = (1 << vlmul) * widen_factor;
      }
    }

    if (reg_num % regs_count != 0) {
      return nullptr;
    }
    return new mpact::sim::riscv::RV32VectorDestinationOperand(
        absl::Span<mpact::sim::generic::RegisterBase*>(&vregs_[reg_num],
                                                       regs_count),
        latency, vregs_[reg_num]->name());
  }

  if (opcode == ::coralnpu::sim::isa32_m3::OpcodeEnum::kVfncvtbf16FFW) {
    if (op == ::coralnpu::sim::isa32_m3::DestOpEnum::kVd) {
      uint32_t vlmul = (state_ != nullptr && state_->rv_vector() != nullptr)
                           ? (state_->rv_vector()->vtype() & 0x7)
                           : 0;
      int regs_count = (vlmul < 4) ? (1 << vlmul) : 1;
      auto reg_num =
          ::coralnpu::sim::encoding_m3::Extractors::VArith::ExtractVd(
              inst_word_);
      if (reg_num % regs_count != 0) return nullptr;
      return new mpact::sim::riscv::RV32VectorDestinationOperand(
          absl::Span<mpact::sim::generic::RegisterBase*>(&vregs_[reg_num],
                                                         regs_count),
          latency, vregs_[reg_num]->name());
    }
  }

  return Base::GetDestination(slot, entry, opcode, op, dest_no, latency);
}

}  // namespace coralnpu::sim
