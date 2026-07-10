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

#include "sim/coralnpu_m3_zfhmin_instructions.h"

#include <cstdint>
#include <type_traits>

#include "sim/coralnpu_state.h"
#include "sim/coralnpu_v2_state.h"
#include "absl/base/nullability.h"
#include "riscv/riscv_register.h"
#include "riscv/riscv_state.h"
#include "mpact/sim/generic/instruction.h"

namespace coralnpu::sim {

using ::mpact::sim::generic::Instruction;

namespace {

constexpr uint32_t kFp16Mask = 0xFFFF;

template <typename Register, typename ValueType>
bool AccessCheck(const Instruction* /*absl_nonnull*/ instruction,
                 mpact::sim::riscv::ExceptionCode fault_exception_code) {
  using RegVal = typename Register::ValueType;
  using URegVal = typename std::make_unsigned<RegVal>::type;
  URegVal base = mpact::sim::generic::GetInstructionSource<URegVal>(
      instruction, /*index=*/0);
  RegVal offset = mpact::sim::generic::GetInstructionSource<RegVal>(
      instruction, /*index=*/1);
  URegVal address = base + offset;

  auto* state =
      static_cast<coralnpu::sim::CoralNPUV2State*>(instruction->state());

  address &= kMemMask;

  coralnpu::sim::MemoryPermission permission =
      (fault_exception_code ==
       mpact::sim::riscv::ExceptionCode::kLoadAccessFault)
          ? coralnpu::sim::MemoryPermission::kRead
          : coralnpu::sim::MemoryPermission::kWrite;

  if (!state->HasPermission(address, sizeof(ValueType), permission)) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/address,
                static_cast<uint64_t>(fault_exception_code),
                /*epc=*/instruction->address(), instruction);
    return false;
  }
  return true;
}

}  // namespace

void CoralNPUM3ZfhminFsh(const Instruction* /*absl_nonnull*/ instruction) {
  auto* state =
      static_cast<coralnpu::sim::CoralNPUV2State*>(instruction->state());
  if (state->rv_fp() == nullptr || state->mstatus()->fs() == 0) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                static_cast<uint64_t>(
                    mpact::sim::riscv::ExceptionCode::kIllegalInstruction),
                instruction->address(), instruction);
    return;
  }

  if (!AccessCheck<mpact::sim::riscv::RV32Register, uint16_t>(
          instruction, mpact::sim::riscv::ExceptionCode::kStoreAccessFault)) {
    return;
  }

  uint32_t base =
      mpact::sim::generic::GetInstructionSource<uint32_t>(instruction, 0);
  int32_t offset =
      mpact::sim::generic::GetInstructionSource<int32_t>(instruction, 1);
  uint64_t frs2 =
      mpact::sim::generic::GetInstructionSource<uint64_t>(instruction, 2);
  uint32_t address = base + offset;

  address &= kMemMask;

  auto* db = state->db_factory()->Allocate<uint16_t>(1);
  db->Set<uint16_t>(0, static_cast<uint16_t>(frs2 & kFp16Mask));

  state->StoreMemory(instruction, address, db);
  db->DecRef();
}

void CoralNPUM3ZfhminFlh(const Instruction* /*absl_nonnull*/ instruction) {
  auto* state =
      static_cast<coralnpu::sim::CoralNPUV2State*>(instruction->state());
  if (state->rv_fp() == nullptr || state->mstatus()->fs() == 0) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                static_cast<uint64_t>(
                    mpact::sim::riscv::ExceptionCode::kIllegalInstruction),
                instruction->address(), instruction);
    return;
  }

  if (!AccessCheck<mpact::sim::riscv::RV32Register, uint16_t>(
          instruction, mpact::sim::riscv::ExceptionCode::kLoadAccessFault)) {
    return;
  }

  uint32_t base =
      mpact::sim::generic::GetInstructionSource<uint32_t>(instruction, 0);
  int32_t offset =
      mpact::sim::generic::GetInstructionSource<int32_t>(instruction, 1);
  uint32_t address = base + offset;

  address &= kMemMask;

  auto* value_db =
      instruction->state()->db_factory()->Allocate(sizeof(uint16_t));
  value_db->set_latency(0);
  auto* context = new mpact::sim::riscv::LoadContext(value_db);
  state->LoadMemory(instruction, address, value_db, instruction->child(),
                    context);
  context->DecRef();
}

void CoralNPUM3ZfhminFMvxh(const Instruction* /*absl_nonnull*/ instruction) {
  auto* state = static_cast<CoralNPUV2State*>(instruction->state());
  if (state->rv_fp() == nullptr || state->mstatus()->fs() == 0) {
    state->Trap(/*is_interrupt=*/false, /*trap_value=*/0,
                static_cast<uint64_t>(
                    mpact::sim::riscv::ExceptionCode::kIllegalInstruction),
                instruction->address(), instruction);
    return;
  }

  uint64_t frs1 =
      mpact::sim::generic::GetInstructionSource<uint64_t>(instruction, 0);
  uint16_t val = static_cast<uint16_t>(frs1 & kFp16Mask);
  uint32_t res = static_cast<uint32_t>(static_cast<int16_t>(val));

  auto* db = instruction->Destination(0)->AllocateDataBuffer();
  db->Set<uint32_t>(0, res);
  db->Submit();
}

}  // namespace coralnpu::sim
