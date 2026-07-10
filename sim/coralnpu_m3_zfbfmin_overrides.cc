// Copyright 2026 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "sim/coralnpu_m3_zfbfmin_overrides.h"

#include <cstdint>

#include "sim/coralnpu_v2_state.h"
#include "absl/base/casts.h"
#include "riscv/riscv_fp_info.h"
#include "riscv/riscv_instruction_helpers.h"
#include "riscv/riscv_state.h"
#include "mpact/sim/generic/instruction.h"

namespace coralnpu::sim {

using ::mpact::sim::riscv::ExceptionCode;

void CoralNPUM3ZfbfminFcvtBf16S(
    const mpact::sim::generic::Instruction* instruction) {
  auto* state = static_cast<CoralNPUV2State*>(instruction->state());
  if (state->rv_fp() == nullptr || state->mstatus()->fs() == 0) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                static_cast<uint64_t>(ExceptionCode::kIllegalInstruction),
                instruction->address(), instruction);
    return;
  }

  uint32_t rm =
      ::mpact::sim::generic::GetInstructionSource<uint32_t>(instruction, 1);
  if (rm == 7) {
    auto* rv_fp = state->rv_fp();
    if (!rv_fp->rounding_mode_valid()) {
      state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                  static_cast<uint64_t>(ExceptionCode::kIllegalInstruction),
                  instruction->address(), instruction);
      return;
    }
    rm = static_cast<uint32_t>(rv_fp->GetRoundingMode());
  }

  if (rm == 5 || rm == 6 || rm > 7) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                static_cast<uint64_t>(ExceptionCode::kIllegalInstruction),
                instruction->address(), instruction);
    return;
  }

  uint32_t f_u = absl::bit_cast<uint32_t>(
      mpact::sim::riscv::GetNaNBoxedSource<uint64_t, float>(instruction, 0));

  bool is_nan = ((f_u & 0x7F800000) == 0x7F800000) && ((f_u & 0x007FFFFF) != 0);

  uint16_t bf16_val;
  uint32_t fflags = 0;

  if (is_nan) {
    bf16_val = 0x7FC0;  // Canonical NaN
    if ((f_u & 0x00400000) == 0) {
      fflags |=
          static_cast<uint32_t>(mpact::sim::riscv::FPExceptions::kInvalidOp);
    }
  } else {
    bool sign = (f_u >> 31) != 0;
    bool r_bit = (f_u >> 15) & 1;
    bool s_bit = (f_u & 0x7FFF) != 0;
    bool l_bit = (f_u >> 16) & 1;

    if (r_bit || s_bit) {
      fflags |=
          static_cast<uint32_t>(mpact::sim::riscv::FPExceptions::kInexact);
    }

    bool increment = false;
    switch (rm) {
      case 0:  // RNE
        increment = r_bit && (l_bit || s_bit);
        break;
      case 1:  // RTZ
        increment = false;
        break;
      case 2:  // RDN
        increment = sign && (r_bit || s_bit);
        break;
      case 3:  // RUP
        increment = !sign && (r_bit || s_bit);
        break;
      case 4:  // RMM
        increment = r_bit;
        break;
    }

    uint32_t upper = f_u >> 16;
    if (increment) {
      upper += 1;
    }

    uint32_t exp_before = (f_u >> 23) & 0xFF;
    uint32_t exp_after = (upper >> 7) & 0xFF;

    if (exp_before < 0xFF && exp_after == 0xFF) {
      fflags |=
          static_cast<uint32_t>(mpact::sim::riscv::FPExceptions::kOverflow) |
          static_cast<uint32_t>(mpact::sim::riscv::FPExceptions::kInexact);
    } else if (exp_after == 0 &&
               (fflags & static_cast<uint32_t>(
                             mpact::sim::riscv::FPExceptions::kInexact))) {
      fflags |=
          static_cast<uint32_t>(mpact::sim::riscv::FPExceptions::kUnderflow);
    }

    bf16_val = static_cast<uint16_t>(upper);
  }

  if (instruction->DestinationsSize() > 1) {
    auto* fflags_db = instruction->Destination(1)->AllocateDataBuffer();
    fflags_db->Set<uint32_t>(0, fflags);
    fflags_db->Submit();
  }

  auto* db = instruction->Destination(0)->AllocateDataBuffer();
  db->Set<uint64_t>(0, 0xFFFFFFFFFFFF0000ULL | bf16_val);
  db->Submit();

  state->mstatus()->set_fs(3);  // Dirty
  state->mstatus()->Submit();
}

void CoralNPUM3ZfbfminFcvtSBf16(
    const mpact::sim::generic::Instruction* instruction) {
  auto* state = static_cast<CoralNPUV2State*>(instruction->state());
  if (state->rv_fp() == nullptr || state->mstatus()->fs() == 0) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                static_cast<uint64_t>(ExceptionCode::kIllegalInstruction),
                instruction->address(), instruction);
    return;
  }

  uint32_t rm =
      ::mpact::sim::generic::GetInstructionSource<uint32_t>(instruction, 1);
  if (rm != 0) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                static_cast<uint64_t>(ExceptionCode::kIllegalInstruction),
                instruction->address(), instruction);
    return;
  }

  uint64_t frs1 =
      ::mpact::sim::generic::GetInstructionSource<uint64_t>(instruction, 0);

  bool is_nan_boxed = (frs1 & 0xFFFFFFFFFFFF0000ULL) == 0xFFFFFFFFFFFF0000ULL;
  uint16_t bf16_val = static_cast<uint16_t>(frs1 & 0xFFFF);

  // NaN check (ADR-6)
  bool is_nan = (bf16_val & 0x7F80) == 0x7F80 && (bf16_val & 0x7F) != 0;

  uint32_t fp32_val;
  uint32_t fflags = 0;

  if (!is_nan_boxed || is_nan) {
    fp32_val = 0x7FC00000;  // Canonical NaN
    if (is_nan_boxed && is_nan && (bf16_val & 0x40) == 0) {
      fflags |=
          static_cast<uint32_t>(mpact::sim::riscv::FPExceptions::kInvalidOp);
    }
  } else {
    fp32_val = static_cast<uint32_t>(bf16_val) << 16;
  }

  if (instruction->DestinationsSize() > 1) {
    auto* fflags_db = instruction->Destination(1)->AllocateDataBuffer();
    fflags_db->Set<uint32_t>(0, fflags);
    fflags_db->Submit();
  }

  auto* db = instruction->Destination(0)->AllocateDataBuffer();
  db->Set<uint64_t>(0, 0xFFFFFFFF00000000ULL | fp32_val);
  db->Submit();

  state->mstatus()->set_fs(3);
  state->mstatus()->Submit();
}

}  // namespace coralnpu::sim
