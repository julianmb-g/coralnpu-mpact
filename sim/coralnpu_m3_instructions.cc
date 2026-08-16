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

// Implementation of instruction semantic functions for the CoralNPU M3 ISA.
// Instruction semantic functions specific to the CoralNPU M3 architecture are
// added here.

#include "sim/coralnpu_m3_instructions.h"

#include <cstdint>
#include <functional>

#include "sim/coralnpu_state.h"
#include "absl/base/casts.h"
#include "riscv/riscv_fp_info.h"
#include "riscv/riscv_fp_state.h"
#include "riscv/riscv_instruction_helpers.h"
#include "riscv/riscv_register.h"
#include "riscv/riscv_state.h"
#include "riscv/riscv_vector_state.h"
#include "mpact/sim/generic/data_buffer.h"
#include "mpact/sim/generic/register.h"
#include "mpact/sim/generic/type_helpers.h"

namespace coralnpu::sim {
using ::mpact::sim::generic::FPTypeInfo;
using ::mpact::sim::generic::operator*;  // NOLINT: clang-tidy false positive.
using ::mpact::sim::generic::GetInstructionSource;
using ::mpact::sim::generic::RegisterDestinationOperand;
using ::mpact::sim::riscv::FPExceptions;
using ::mpact::sim::riscv::FPRoundingMode;
using ::mpact::sim::riscv::GetNaNBoxedSource;
using ::mpact::sim::riscv::RiscVFPState;
using ::mpact::sim::riscv::RiscVState;
using ::mpact::sim::riscv::RV32VectorDestinationOperand;
using ::mpact::sim::riscv::RV32VectorSourceOperand;
namespace {

// Internal layout parameters, bitmasks, and predicate functions for bfloat16
// format.
struct BF16Info {
  static constexpr int kBitSize = 16;
  static constexpr int kExpSize = FPTypeInfo<float>::kExpSize;
  static constexpr int kSigSize = kBitSize - kExpSize - 1;
  static constexpr int kF32ToBF16Shift = FPTypeInfo<float>::kSigSize - kSigSize;

  static constexpr uint16_t kCanonicalNaN = 0x7FC0;
  static constexpr uint16_t kExponentMask = ((1U << kExpSize) - 1) << kSigSize;
  static constexpr uint16_t kFractionMask = (1U << kSigSize) - 1;
  static constexpr uint16_t kQuietBit = 1U << (kSigSize - 1);

  static bool IsNaN(uint16_t val) {
    return (val & kExponentMask) == kExponentMask && (val & kFractionMask) != 0;
  }
  static bool IsSNaN(uint16_t val) {
    return IsNaN(val) && (val & kQuietBit) == 0;
  }
};

// Mask used to NaN-box a 16-bit bfloat16 value in a 64-bit floating-point
// register.
static constexpr uint64_t kBoxedBf16Mask = ~uint64_t{0} << 16;

// Rounds a 32-bit float bit pattern down to 16-bit bfloat16 precision based on
// specified rounding mode and updates exception flags (fflags).
uint16_t RoundBFloat16(uint32_t bits, uint32_t inst_rm, RiscVFPState* fp_state,
                       uint32_t* fflags) {
  [[maybe_unused]] bool sign =
      FPTypeInfo<float>::SignBit(absl::bit_cast<float>(bits));

  // Extract rounding bits relative to bfloat16 mantissa boundary.
  bool lsb = (bits >> BF16Info::kF32ToBF16Shift) & 1;
  bool guard_bit = (bits >> (BF16Info::kF32ToBF16Shift - 1)) & 1;
  constexpr uint32_t kStickyMask = (1U << (BF16Info::kF32ToBF16Shift - 1)) - 1;
  bool sticky_bit = (bits & kStickyMask) != 0;

  bool increment = false;
  if (guard_bit || sticky_bit) {
    *fflags |= *FPExceptions::kInexact;
    // Determine effective rounding mode (resolving dynamic mode from FCSR if
    // requested).
    auto rm = (inst_rm == *FPRoundingMode::kDynamic)
                  ? (fp_state != nullptr ? fp_state->GetRoundingMode()
                                         : FPRoundingMode::kRoundToNearest)
                  : static_cast<FPRoundingMode>(inst_rm);
    switch (rm) {
      case FPRoundingMode::kRoundToNearest:
        increment = guard_bit && (lsb || sticky_bit);
        break;
      case FPRoundingMode::kRoundUp:
        increment = !sign && (guard_bit || sticky_bit);
        break;
      case FPRoundingMode::kRoundDown:
        increment = sign && (guard_bit || sticky_bit);
        break;
      case FPRoundingMode::kRoundToNearestTiesToMax:
        increment = guard_bit;
        break;
      default:
        break;
    }
  }

  uint32_t exp =
      (bits & FPTypeInfo<float>::kExpMask) >> FPTypeInfo<float>::kSigSize;
  constexpr uint32_t kMaxExp = (1U << FPTypeInfo<float>::kExpSize) - 1;
  uint32_t res_bits = (bits >> BF16Info::kF32ToBF16Shift);
  if (increment) {
    res_bits += 1;
    // Check for rounding overflow beyond representable normal range.
    if ((res_bits & BF16Info::kExponentMask) == BF16Info::kExponentMask &&
        exp != kMaxExp) {
      *fflags |= *FPExceptions::kOverflow | *FPExceptions::kInexact;
    }
  }

  // Check for underflow signal when result is subnormal and inexact.
  if ((guard_bit || sticky_bit) && (res_bits & BF16Info::kExponentMask) == 0) {
    *fflags |= *FPExceptions::kUnderflow;
  }
  return static_cast<uint16_t>(res_bits);
}

// Converts single-precision float bit pattern to bfloat16, handling NaN
// payloads.
uint16_t ConvertF32ToBF16(uint32_t bits, uint32_t rm_val,
                          ::mpact::sim::riscv::RiscVState* state,
                          uint32_t* fflags) {
  float f32_f = absl::bit_cast<float>(bits);
  if (FPTypeInfo<float>::IsNaN(f32_f)) {
    if (FPTypeInfo<float>::IsSNaN(f32_f)) {
      *fflags |= *FPExceptions::kInvalidOp;
    }
    return BF16Info::kCanonicalNaN;
  }
  return RoundBFloat16(bits, rm_val, state->rv_fp(), fflags);
}

float ConvertBF16ToF32(uint16_t bf16) {
  uint32_t f32_bits = static_cast<uint32_t>(bf16) << 16;
  return absl::bit_cast<float>(f32_bits);
}

struct ConversionResultF32 {
  uint32_t result = 0;
  uint32_t fflags = 0;
};

ConversionResultF32 ConvertBF16ToF32Precise(uint16_t bf16_bits) {
  constexpr uint32_t kBF16ExpShift = 7;
  constexpr uint32_t kBF16ExpMask = 0xFF;
  constexpr uint32_t kBF16MantissaMask = 0x7F;
  constexpr uint32_t kBF16SNaNBit = 0x40;
  constexpr uint32_t kFloat32MantissaShift = 16;
  constexpr uint32_t kFloat32CanonicalNan = 0x7FC00000;

  uint32_t src_bits = bf16_bits;
  uint32_t exp = (src_bits >> kBF16ExpShift) & kBF16ExpMask;
  uint32_t frac = src_bits & kBF16MantissaMask;

  if (exp == kBF16ExpMask && frac != 0) {
    // NaN input.
    uint32_t fflags = 0;
    if ((frac & kBF16SNaNBit) == 0) {
      // SNaN
      fflags = *FPExceptions::kInvalidOp;
    }
    return {kFloat32CanonicalNan, fflags};
  }

  return {src_bits << kFloat32MantissaShift, 0};
}

}  // namespace

void CoralNPUFcvtBf16S(Instruction* inst) {
  auto state = static_cast<::mpact::sim::riscv::RiscVState*>(inst->state());

  // Check floating-point unit enable bit in mstatus.
  if (state->mstatus()->fs() == 0) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                *mpact::sim::riscv::ExceptionCode::kIllegalInstruction,
                /*epc=*/inst->address(), inst);
    return;
  }
  // Validate rounding mode specified in instruction or FCSR.
  uint32_t rm_val = inst->Source(1)->AsUint32(0);
  if (rm_val > *FPRoundingMode::kRoundToNearestTiesToMax &&
      rm_val != *FPRoundingMode::kDynamic) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                *mpact::sim::riscv::ExceptionCode::kIllegalInstruction,
                /*epc=*/inst->address(), inst);
    return;
  }
  if (rm_val == *FPRoundingMode::kDynamic &&
      !state->rv_fp()->rounding_mode_valid()) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                *mpact::sim::riscv::ExceptionCode::kIllegalInstruction,
                /*epc=*/inst->address(), inst);
    return;
  }
  uint32_t rm = (rm_val == *FPRoundingMode::kDynamic)
                    ? *state->rv_fp()->GetRoundingMode()
                    : rm_val;

  // Fetch NaN-boxed float input from fs1.
  float f32_val = GetNaNBoxedSource<uint64_t, float>(inst, 0);
  uint32_t f32 = absl::bit_cast<uint32_t>(f32_val);

  uint32_t fflags = 0;
  uint16_t bf16 = ConvertF32ToBF16(f32, rm, state, &fflags);

  // Write NaN-boxed bfloat16 output to destination register fd.
  auto* rd_op =
      static_cast<RegisterDestinationOperand<uint64_t>*>(inst->Destination(0));
  auto* rd_reg = rd_op->GetRegister();
  rd_reg->data_buffer()->Set<uint64_t>(
      0, static_cast<uint64_t>(bf16) | kBoxedBf16Mask);

  // Accumulate exception flags into fflags/fcsr destination operand.
  auto* fflags_db = inst->Destination(1)->AllocateDataBuffer();
  fflags_db->Set<uint32_t>(0, fflags);
  fflags_db->Submit();
}

void FcvtBf16S(Instruction* inst) { CoralNPUFcvtBf16S(inst); }

void FcvtSBf16(Instruction* inst) {
  auto* state = static_cast<::mpact::sim::riscv::RiscVState*>(inst->state());
  if (state == nullptr || state->rv_fp() == nullptr) {
    return;
  }

  // Check floating-point unit enable bit in mstatus.
  if (state->mstatus()->fs() == 0) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                *mpact::sim::riscv::ExceptionCode::kIllegalInstruction,
                /*epc=*/inst->address(), inst);
    return;
  }

  uint32_t rm_val = inst->Source(1)->AsUint32(0);
  if (rm_val != 0) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                *mpact::sim::riscv::ExceptionCode::kIllegalInstruction,
                /*epc=*/inst->address(), inst);
    return;
  }

  uint32_t src_bits = GetInstructionSource<uint32_t>(inst, 0);

  // Validate 16-bit NaN-boxing (upper 16 bits of the 32-bit register must be
  // all 1s).
  constexpr uint32_t kFP32NaNBoxedMask = 0xFFFF0000;
  bool is_boxed = (src_bits & kFP32NaNBoxedMask) == kFP32NaNBoxedMask;

  ConversionResultF32 conv;
  if (!is_boxed) {
    constexpr uint32_t kFloat32CanonicalNan = 0x7FC00000;
    conv.result = kFloat32CanonicalNan;
    conv.fflags = 0;
  } else {
    uint16_t bf16_bits = static_cast<uint16_t>(src_bits & 0xFFFF);
    conv = ConvertBF16ToF32Precise(bf16_bits);
  }

  auto* dest_db = inst->Destination(0)->AllocateDataBuffer();
  dest_db->Set<uint64_t>(0, 0xFFFFFFFF00000000ULL | conv.result);
  dest_db->Submit();

  if (inst->DestinationsSize() >= 2 && inst->Destination(1) != nullptr) {
    auto* fflags_db = inst->Destination(1)->AllocateDataBuffer();
    fflags_db->Set<uint32_t>(0, conv.fflags);
    fflags_db->Submit();
  }
}

void Vfwcvtbf16ffv(Instruction* inst) {
  auto* state = static_cast<CoralNPUState*>(inst->state());
  if (state == nullptr || state->rv_fp() == nullptr ||
      state->rv_vector() == nullptr) {
    return;
  }

  // Check floating-point unit and vector unit enable bits in mstatus.
  bool fs_enabled = state->mstatus()->fs() != 0;
  bool vs_enabled = ((state->mstatus()->GetUint64() >> 9) & 0b11) != 0;
  if (!fs_enabled || !vs_enabled) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                *mpact::sim::riscv::ExceptionCode::kIllegalInstruction,
                /*epc=*/inst->address(), inst);
    return;
  }

  if (inst->DestinationsSize() == 0 || inst->SourcesSize() == 0 ||
      inst->Destination(0) == nullptr || inst->Source(0) == nullptr) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                *mpact::sim::riscv::ExceptionCode::kIllegalInstruction,
                /*epc=*/inst->address(), inst);
    return;
  }

  auto* vd = static_cast<RV32VectorDestinationOperand*>(inst->Destination(0));
  auto* vs2 = static_cast<RV32VectorSourceOperand*>(inst->Source(0));

  // Check register group overlap (widening requires vd to not overlap vs2).
  bool overlaps = false;
  for (int dest_reg_idx = 0; dest_reg_idx < vd->size(); ++dest_reg_idx) {
    auto* dest_reg = std::any_cast<mpact::sim::generic::RegisterBase*>(
        vd->GetObject(dest_reg_idx));
    for (int src_reg_idx = 0; src_reg_idx < vs2->size(); ++src_reg_idx) {
      auto* src_reg = vs2->GetRegister(src_reg_idx);
      if (dest_reg == src_reg) {
        overlaps = true;
        break;
      }
    }
    if (overlaps) break;
  }

  if (overlaps) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                *mpact::sim::riscv::ExceptionCode::kIllegalInstruction,
                /*epc=*/inst->address(), inst);
    return;
  }

  auto* vstate = state->rv_vector();
  int vl = vstate->vector_length();
  int vstart = vstate->vstart();
  uint32_t fflags = 0;

  bool is_masked = false;
  RV32VectorSourceOperand* mask_op = nullptr;
  if (inst->SourcesSize() >= 2) {
    auto* vm_op = inst->Source(1);
    if (vm_op != nullptr && !vm_op->AsString().empty() &&
        vm_op->AsString() != "__VectorTrue__") {
      is_masked = true;
      mask_op = static_cast<RV32VectorSourceOperand*>(vm_op);
    }
  }

  absl::Span<const uint8_t> mask_span;
  if (is_masked) {
    mask_span = mask_op->GetRegister(0)->data_buffer()->Get<uint8_t>();
  }

  int elements_per_vector =
      vstate->vector_register_byte_length() / sizeof(float);

  int last_reg = -1;
  ::mpact::sim::generic::DataBuffer* dest_db = nullptr;
  absl::Span<float> dest_span;

  for (int vector_index = vstart; vector_index < vl; ++vector_index) {
    int reg = vector_index / elements_per_vector;
    int i = vector_index % elements_per_vector;
    if (reg != last_reg) {
      if (dest_db != nullptr) {
        dest_db->Submit();
      }
      dest_db = vd->CopyDataBuffer(reg);
      dest_span = dest_db->Get<float>();
      last_reg = reg;
    }

    bool mask_value = true;
    if (is_masked) {
      int mask_index = vector_index >> 3;
      int mask_offset = vector_index & 0b111;
      mask_value = ((mask_span[mask_index] >> mask_offset) & 0b1) != 0;
    }

    if (mask_value) {
      uint16_t bf16 = GetInstructionSource<uint16_t>(inst, 0, vector_index);
      auto conv = ConvertBF16ToF32Precise(bf16);
      dest_span[i] = absl::bit_cast<float>(conv.result);
      fflags |= conv.fflags;
    }
  }

  if (dest_db != nullptr) {
    dest_db->Submit();
  }
  // Clear vstart.
  vstate->clear_vstart();

  // Update fflags.
  if (fflags != 0) {
    uint32_t current_fflags = state->rv_fp()->fflags()->GetUint32();
    state->rv_fp()->fflags()->Set(current_fflags | fflags);
  }
}

void Vfncvtbf16ffw(Instruction* inst) {
  auto* state = static_cast<CoralNPUState*>(inst->state());
  if (state == nullptr || state->rv_fp() == nullptr ||
      state->rv_vector() == nullptr) {
    return;
  }

  // Check floating-point unit and vector unit enable bits in mstatus.
  bool fs_enabled = state->mstatus()->fs() != 0;
  bool vs_enabled = ((state->mstatus()->GetUint64() >> 9) & 0b11) != 0;
  if (!fs_enabled || !vs_enabled) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                *mpact::sim::riscv::ExceptionCode::kIllegalInstruction,
                /*epc=*/inst->address(), inst);
    return;
  }

  if (inst->DestinationsSize() == 0 || inst->SourcesSize() == 0 ||
      inst->Destination(0) == nullptr || inst->Source(0) == nullptr) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                *mpact::sim::riscv::ExceptionCode::kIllegalInstruction,
                /*epc=*/inst->address(), inst);
    return;
  }

  if (!state->rv_fp()->rounding_mode_valid()) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                *mpact::sim::riscv::ExceptionCode::kIllegalInstruction,
                /*epc=*/inst->address(), inst);
    return;
  }

  auto* vd = static_cast<RV32VectorDestinationOperand*>(inst->Destination(0));
  auto* vs2 = static_cast<RV32VectorSourceOperand*>(inst->Source(0));

  // Check register group overlap (narrowing requires vd to not overlap vs2).
  bool overlaps = false;
  for (int dest_reg_idx = 0; dest_reg_idx < vd->size(); ++dest_reg_idx) {
    auto* dest_reg = std::any_cast<mpact::sim::generic::RegisterBase*>(
        vd->GetObject(dest_reg_idx));
    for (int src_reg_idx = 0; src_reg_idx < vs2->size(); ++src_reg_idx) {
      auto* src_reg = vs2->GetRegister(src_reg_idx);
      if (dest_reg == src_reg) {
        overlaps = true;
        break;
      }
    }
    if (overlaps) break;
  }

  if (overlaps) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                *mpact::sim::riscv::ExceptionCode::kIllegalInstruction,
                /*epc=*/inst->address(), inst);
    return;
  }

  auto* vstate = state->rv_vector();
  int vl = vstate->vector_length();
  int vstart = vstate->vstart();
  uint32_t rm = *state->rv_fp()->GetRoundingMode();
  uint32_t fflags = 0;

  bool is_masked = false;
  RV32VectorSourceOperand* mask_op = nullptr;
  if (inst->SourcesSize() >= 2) {
    auto* vm_op = inst->Source(1);
    if (vm_op != nullptr && !vm_op->AsString().empty() &&
        vm_op->AsString() != "__VectorTrue__") {
      is_masked = true;
      mask_op = static_cast<RV32VectorSourceOperand*>(vm_op);
    }
  }

  absl::Span<const uint8_t> mask_span;
  if (is_masked) {
    mask_span = mask_op->GetRegister(0)->data_buffer()->Get<uint8_t>();
  }

  int elements_per_vector =
      vstate->vector_register_byte_length() / sizeof(uint16_t);

  int last_reg = -1;
  ::mpact::sim::generic::DataBuffer* dest_db = nullptr;
  absl::Span<uint16_t> dest_span;

  for (int vector_index = vstart; vector_index < vl; ++vector_index) {
    int reg = vector_index / elements_per_vector;
    int i = vector_index % elements_per_vector;
    if (reg != last_reg) {
      if (dest_db != nullptr) {
        dest_db->Submit();
      }
      dest_db = vd->CopyDataBuffer(reg);
      dest_span = dest_db->Get<uint16_t>();
      last_reg = reg;
    }

    bool mask_value = true;
    if (is_masked) {
      int mask_index = vector_index >> 3;
      int mask_offset = vector_index & 0b111;
      mask_value = ((mask_span[mask_index] >> mask_offset) & 0b1) != 0;
    }

    if (mask_value) {
      float f32 = GetInstructionSource<float>(inst, 0, vector_index);
      uint32_t f32_bits = absl::bit_cast<uint32_t>(f32);
      uint16_t bf16 = ConvertF32ToBF16(f32_bits, rm, state, &fflags);
      dest_span[i] = bf16;
    }
  }

  if (dest_db != nullptr) {
    dest_db->Submit();
  }
  // Clear vstart.
  vstate->clear_vstart();

  // Update fflags.
  uint32_t current_fflags = state->rv_fp()->fflags()->GetUint32();
  state->rv_fp()->fflags()->Set(current_fflags | fflags);
}

}  // namespace coralnpu::sim
