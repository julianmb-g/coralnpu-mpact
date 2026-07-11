// Copyright 2023 Google LLC
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

#include "sim/coralnpu_instructions.h"

#include <cstdint>
#include <string>

#include "sim/coralnpu_state.h"
#include "riscv/riscv_fp_info.h"
#include "riscv/riscv_fp_state.h"
#include "riscv/riscv_state.h"
#include "mpact/sim/generic/instruction.h"
#include "mpact/sim/generic/type_helpers.h"

namespace coralnpu::sim {

// Rationale: Helper function to perform BFloat16 rounding as per RISC-V
// Zvfbfmin. This implements the same rounding logic as scalar fcvt.bf16.s.
uint16_t RoundBFloat16(uint32_t bits, uint32_t inst_rm,
                       mpact::sim::riscv::RiscVFPState* fp_state,
                       uint32_t* fflags) {
  using ::mpact::sim::riscv::FPExceptions;
  using ::mpact::sim::riscv::FPRoundingMode;
  bool sign = (bits >> 31) & 1;
  bool l = (bits >> 16) & 1;      // LSB
  bool g = (bits >> 15) & 1;      // Guard
  bool s = (bits & 0x7FFF) != 0;  // Sticky

  bool increment = false;
  if (g || s) {
    *fflags |= static_cast<uint32_t>(FPExceptions::kInexact);
    auto rm = (inst_rm == 7)
                  ? (fp_state != nullptr ? fp_state->GetRoundingMode()
                                         : FPRoundingMode::kRoundToNearest)
                  : static_cast<FPRoundingMode>(inst_rm);
    switch (rm) {
      case FPRoundingMode::kRoundToNearest:
        increment = g && (l || s);
        break;
      case FPRoundingMode::kRoundTowardsZero:
        increment = false;
        break;
      case FPRoundingMode::kRoundUp:
        increment = !sign && (g || s);
        break;
      case FPRoundingMode::kRoundDown:
        increment = sign && (g || s);
        break;
      case FPRoundingMode::kRoundToNearestTiesToMax:
        increment = g;
        break;
      default:
        break;
    }
  }
  uint32_t exp = (bits >> 23) & 0xFF;
  uint32_t res_bits = (bits >> 16);
  if (increment) {
    res_bits += 1;
    if ((res_bits & 0x7F80) == 0x7F80 && exp != 0xFF) {
      *fflags |= static_cast<uint32_t>(FPExceptions::kOverflow) |
                 static_cast<uint32_t>(FPExceptions::kInexact);
    }
  }
  return static_cast<uint16_t>(res_bits);
}

void Fcvtsbf16(mpact::sim::generic::Instruction* inst) {
  auto* state = static_cast<CoralNPUState*>(inst->state());
  mpact::sim::riscv::RiscVFPState* fp_state = state->rv_fp();

  if (fp_state == nullptr) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                static_cast<uint64_t>(
                    mpact::sim::riscv::ExceptionCode::kIllegalInstruction),
                inst->address(), inst);
    return;
  }

  uint32_t inst_rm =
      mpact::sim::generic::GetInstructionSource<uint32_t>(inst, 1);

  // RISC-V Zfbfmin specification reserves rounding modes 5 and 6.
  // We must trap with an illegal instruction exception.
  if (inst_rm == 5 || inst_rm == 6) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                static_cast<uint64_t>(
                    mpact::sim::riscv::ExceptionCode::kIllegalInstruction),
                inst->address(), inst);
    return;
  }

  if (inst_rm == 7 && !fp_state->rounding_mode_valid()) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                static_cast<uint64_t>(
                    mpact::sim::riscv::ExceptionCode::kIllegalInstruction),
                inst->address(), inst);
    return;
  }

  uint64_t rs1 = mpact::sim::generic::GetInstructionSource<uint64_t>(inst, 0);

  uint16_t src_bits = rs1 & 0xFFFF;
  uint32_t bits = 0;
  if ((rs1 >> 16) != 0xFFFFFFFFFFFFULL) {
    bits = 0x7FC00000;  // Canonical float32 NaN
  } else {
    // Check for NaN (Exponent 0xFF, Fraction != 0)
    uint32_t exp = (src_bits >> 7) & 0xFF;
    uint32_t frac = src_bits & 0x7F;
    if (exp == 0xFF && frac != 0) {
      // If SNaN (Fraction MSB is 0), set InvalidOp flag
      if ((frac & 0x40) == 0) {
        if (fp_state != nullptr && fp_state->fflags() != nullptr) {
          fp_state->fflags()->Set(
              fp_state->fflags()->GetUint32() |
              static_cast<uint32_t>(
                  mpact::sim::riscv::FPExceptions::kInvalidOp));
        }
      }
      bits = 0x7FC00000;  // Canonical float32 NaN
    } else {
      bits = static_cast<uint32_t>(src_bits) << 16;
    }
  }

  auto* dest = inst->Destination(0);
  auto* db = dest->AllocateDataBuffer();
  if (db != nullptr) {
    db->Set<uint64_t>(0, 0xFFFFFFFF00000000ULL | bits);
    db->Submit();
  }
}

void Fcvtbf16s(mpact::sim::generic::Instruction* inst) {
  using ::mpact::sim::riscv::ExceptionCode;

  uint64_t vs2_64 =
      mpact::sim::generic::GetInstructionSource<uint64_t>(inst, 0);
  uint32_t inst_rm =
      mpact::sim::generic::GetInstructionSource<uint32_t>(inst, 1);
  auto* state = static_cast<CoralNPUState*>(inst->state());
  mpact::sim::riscv::RiscVFPState* fp_state = state->rv_fp();

  if (fp_state == nullptr) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                static_cast<uint64_t>(ExceptionCode::kIllegalInstruction),
                inst->address(), inst);
    return;
  }

  if (inst_rm == 5 || inst_rm == 6) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                static_cast<uint64_t>(ExceptionCode::kIllegalInstruction),
                inst->address(), inst);
    return;
  }

  if (inst_rm == 7 && !fp_state->rounding_mode_valid()) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                static_cast<uint64_t>(ExceptionCode::kIllegalInstruction),
                inst->address(), inst);
    return;
  }

  uint32_t bits = 0;
  uint32_t fflags = 0;
  if ((vs2_64 >> 32) != 0xFFFFFFFFULL) {
    bits = 0x7FC00000;
  } else {
    bits = static_cast<uint32_t>(vs2_64 & 0xFFFFFFFF);
    uint32_t exp = (bits >> 23) & 0xFF;
    uint32_t frac = bits & 0x7FFFFF;

    if (exp == 0xFF && frac != 0) {
      if (!(frac & 0x400000)) {
        fflags |=
            static_cast<uint32_t>(mpact::sim::riscv::FPExceptions::kInvalidOp);
      }
      bits = 0x7FC00000;
    }
  }

  uint16_t result = RoundBFloat16(bits, inst_rm, fp_state, &fflags);

  if (fflags != 0 && fp_state != nullptr && fp_state->fflags() != nullptr) {
    fp_state->fflags()->Set(fp_state->fflags()->GetUint32() | fflags);
  }

  auto* dest = inst->Destination(0);
  auto* db = dest->AllocateDataBuffer();
  if (db != nullptr) {
    db->Set<uint64_t>(0, 0xFFFFFFFFFFFF0000ULL | result);
    db->Submit();
  }
}

void CoralNPUIllegalInstruction(mpact::sim::generic::Instruction* inst) {
  auto* state = static_cast<CoralNPUState*>(inst->state());
  state->Trap(/*is_interrupt*/ false, /*trap_value*/ 0,
              static_cast<uint64_t>(
                  mpact::sim::riscv::ExceptionCode::kIllegalInstruction),
              /*epc*/ inst->address(), inst);
}

void CoralNPUNopInstruction(mpact::sim::generic::Instruction* inst) {}

void CoralNPUIMpause(const mpact::sim::generic::Instruction* inst) {
  auto* state = static_cast<CoralNPUState*>(inst->state());
  state->MPause(inst);
}

// A helper function to determine if there is a \0 in a char[4] stored in
// uint32_t
bool WordHasZero(uint32_t data) {
  return (((data >> 24) & 0xff) == 0) || (((data >> 16) & 0xff) == 0) ||
         (((data >> 8) & 0xff) == 0) || ((data & 0xff) == 0);
}

// A helper function to load a string from the memory address by detecting the
// '\0' terminator
void CoralNPUStringLoadHelper(const mpact::sim::generic::Instruction* inst,
                              std::string* out_string) {
  auto* state = static_cast<CoralNPUState*>(inst->state());
  auto addr = mpact::sim::generic::GetInstructionSource<uint32_t>(inst, 0, 0);
  uint32_t data;
  auto* db = state->db_factory()->Allocate<uint32_t>(1);
  do {
    state->LoadMemory(inst, addr, db, nullptr, nullptr);
    data = db->Get<uint32_t>(0);
    *out_string +=
        std::string(reinterpret_cast<char*>(&data), sizeof(uint32_t));
    addr += 4;
  } while (!WordHasZero(data) && addr < state->max_physical_address());
  // Trim the string properly.
  out_string->resize(out_string->find('\0'));
  db->DecRef();
}

// Handle FLOG, SLOG, CLOG, and KLOG instructions
void CoralNPULogInstruction(int log_mode,
                            mpact::sim::generic::Instruction* inst) {
  auto* state = static_cast<CoralNPUState*>(inst->state());
  switch (log_mode) {
    case 0: {  // Format log op to set the format of the printout and print it.
      std::string format_string;
      CoralNPUStringLoadHelper(inst, &format_string);
      state->PrintLog(format_string);
      break;
    }
    case 1: {  // Scalar log op to load an integer argument.
      // The value is stored as an unsigned integer. The actual format will be
      // determined with the format specifier "d" or "u".
      auto data =
          mpact::sim::generic::GetInstructionSource<uint32_t>(inst, 0, 0);
      state->SetLogArgs(data);
      break;
    }
    case 2: {  // Character log op to load a group of char[4] as an argument.
      auto data =
          mpact::sim::generic::GetInstructionSource<uint32_t>(inst, 0, 0);
      auto* clog_string = state->clog_string();
      // CLOG can break a long character array as multiple CLOG calls, and they
      // need to be combined as a single string argument.
      *clog_string +=
          std::string(reinterpret_cast<char*>(&data), sizeof(uint32_t));
      if (WordHasZero(data)) {
        // Trim the string properly.
        clog_string->resize(clog_string->find('\0'));
        state->SetLogArgs(*clog_string);
        clog_string->clear();
      }
      break;
    }
    case 3: {  // String log to op load a string argument.
      std::string str_arg;
      CoralNPUStringLoadHelper(inst, &str_arg);
      state->SetLogArgs(str_arg);
      break;
    }
    default:
      break;
  }
}

// Handle Store instructions for mmap_uncached addresses
template <typename T>
void CoralNPUIStore(Instruction* inst) {
  uint32_t base = mpact::sim::generic::GetInstructionSource<uint32_t>(inst, 0);
  int32_t offset = mpact::sim::generic::GetInstructionSource<int32_t>(inst, 1);
  uint32_t address = base + offset;
  T value = mpact::sim::generic::GetInstructionSource<T>(inst, 2);
  auto* state = static_cast<CoralNPUState*>(inst->state());
  // Check and exclude the cache invalidation bit. However, the semihost tests
  // use the memory space greater than the coralnpu HW configuration and do not
  // comply to the magic bit setting. Exclude the check and mask for those
  // tests.
  if (state->max_physical_address() <=
      kCoralnpuMaxMemoryAddress) {  // exclude semihost tests
    address &= kMemMask;
  }
  auto* db = state->db_factory()->Allocate(sizeof(T));
  db->Set<T>(0, value);
  state->StoreMemory(inst, address, db);
  db->DecRef();
}

template void CoralNPUIStore<uint32_t>(mpact::sim::generic::Instruction* inst);
template void CoralNPUIStore<uint16_t>(mpact::sim::generic::Instruction* inst);
template void CoralNPUIStore<uint8_t>(mpact::sim::generic::Instruction* inst);

}  // namespace coralnpu::sim
