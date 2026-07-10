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

#include <cstdint>
#include <memory>

#include "sim/coralnpu_m3_zfbfmin_overrides.h"
#include "sim/coralnpu_v2_state.h"
#include "googletest/include/gtest/gtest.h"
#include "riscv/riscv_fp_state.h"
#include "riscv/riscv_register.h"
#include "riscv/riscv_state.h"
#include "mpact/sim/generic/immediate_operand.h"
#include "mpact/sim/generic/instruction.h"
#include "mpact/sim/generic/register.h"
#include "mpact/sim/util/memory/flat_demand_memory.h"

namespace coralnpu::sim {
namespace {

using ::mpact::sim::generic::ImmediateOperand;
using ::mpact::sim::generic::Instruction;
using ::mpact::sim::riscv::RiscVXlen;
using ::mpact::sim::riscv::RVFpRegister;
using ::mpact::sim::util::FlatDemandMemory;

class CoralNPUM3ZfbfminRoundingTest : public ::testing::Test {
 protected:
  void SetUp() override {
    memory_ = std::make_unique<FlatDemandMemory>();
    CoralNPUV2StateConfig config;
    state_ = CreateCoralNPUV2State("test_state", RiscVXlen::RV32, memory_.get(),
                                   nullptr, &config);
    rv_fp_state_ = std::make_unique<::mpact::sim::riscv::RiscVFPState>(
        state_->csr_set(), state_.get());
    state_->set_rv_fp(rv_fp_state_.get());
    state_->mstatus()->set_fs(1);  // Enabled
    state_->mstatus()->Submit();
    frs1_reg_ = new RVFpRegister(state_.get(), "f1");
    state_->AddRegister("f1", frs1_reg_);
    frd_reg_ = new RVFpRegister(state_.get(), "f0");
    state_->AddRegister("f0", frd_reg_);
    fflags_reg_ =
        new mpact::sim::generic::Register<uint32_t>(state_.get(), "fflags");
    state_->AddRegister("fflags", fflags_reg_);
  }

  std::unique_ptr<FlatDemandMemory> memory_;
  std::unique_ptr<CoralNPUV2State> state_;
  std::unique_ptr<::mpact::sim::riscv::RiscVFPState> rv_fp_state_;
  RVFpRegister* frs1_reg_;
  RVFpRegister* frd_reg_;
  mpact::sim::generic::Register<uint32_t>* fflags_reg_;
};

TEST_F(CoralNPUM3ZfbfminRoundingTest, FcvtBf16SRNE_ExactConversion) {
  auto inst = std::make_unique<Instruction>(/*address=*/0, state_.get());
  inst->set_semantic_function(CoralNPUM3ZfbfminFcvtBf16S);

  // FP32: 1.0 (0x3f800000). BF16: 1.0 (0x3f80).
  frs1_reg_->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFF3f800000ULL);
  inst->AppendSource(frs1_reg_->CreateSourceOperand());
  inst->AppendSource(new ImmediateOperand<uint32_t>(0));  // RNE

  inst->AppendDestination(frd_reg_->CreateDestinationOperand(/*latency=*/0));
  inst->AppendDestination(fflags_reg_->CreateDestinationOperand(/*latency=*/0));

  inst->Execute();

  EXPECT_EQ(frd_reg_->data_buffer()->Get<uint64_t>(0), 0xffffffffffff3f80ULL);
  EXPECT_EQ(fflags_reg_->data_buffer()->Get<uint32_t>(0), 0);
}

TEST_F(CoralNPUM3ZfbfminRoundingTest, FcvtBf16SRNE_TieToEven) {
  auto inst = std::make_unique<Instruction>(/*address=*/0, state_.get());
  inst->set_semantic_function(CoralNPUM3ZfbfminFcvtBf16S);

  // FP32: Tie case L=0, R=1, S=0. Should round down (L=0).
  // 1.0 + 0.5 LSB = 0x3f808000.
  frs1_reg_->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFF3f808000ULL);
  inst->AppendSource(frs1_reg_->CreateSourceOperand());
  inst->AppendSource(new ImmediateOperand<uint32_t>(0));  // RNE

  inst->AppendDestination(frd_reg_->CreateDestinationOperand(/*latency=*/0));
  inst->AppendDestination(fflags_reg_->CreateDestinationOperand(/*latency=*/0));

  inst->Execute();
  EXPECT_EQ(frd_reg_->data_buffer()->Get<uint64_t>(0), 0xffffffffffff3f80ULL);
  EXPECT_EQ(fflags_reg_->data_buffer()->Get<uint32_t>(0), 1);

  // FP32: Tie case L=1, R=1, S=0. Should round up (L becomes 0).
  // 1.0 + 1 LSB + 0.5 LSB = 0x3f818000.
  frs1_reg_->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFF3f818000ULL);
  inst->Execute();
  EXPECT_EQ(frd_reg_->data_buffer()->Get<uint64_t>(0), 0xffffffffffff3f82ULL);
  EXPECT_EQ(fflags_reg_->data_buffer()->Get<uint32_t>(0), 1);
}

TEST_F(CoralNPUM3ZfbfminRoundingTest, FcvtBf16SRDN_R0_S1) {
  auto inst = std::make_unique<Instruction>(/*address=*/0, state_.get());
  inst->set_semantic_function(CoralNPUM3ZfbfminFcvtBf16S);

  // FP32: -1.0 - epsilon. sign=1, R=0, S!=0.
  // 0xBF804000.
  // R is bit 15 (0). S is bits 14-0 (0x4000, non-zero).
  frs1_reg_->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFFbf804000ULL);
  inst->AppendSource(frs1_reg_->CreateSourceOperand());
  inst->AppendSource(new ImmediateOperand<uint32_t>(2));  // RDN

  inst->AppendDestination(frd_reg_->CreateDestinationOperand(/*latency=*/0));
  inst->AppendDestination(fflags_reg_->CreateDestinationOperand(/*latency=*/0));

  inst->Execute();

  // Under RDN (Round Down), a negative inexact number should increment
  // magnitude. So 0xbf80 becomes 0xbf81.
  EXPECT_EQ(frd_reg_->data_buffer()->Get<uint64_t>(0), 0xffffffffffffbf81ULL);
  EXPECT_EQ(fflags_reg_->data_buffer()->Get<uint32_t>(0), 1);
}

TEST_F(CoralNPUM3ZfbfminRoundingTest, FcvtBf16STrapsOnInvalidRoundingMode) {
  bool trap_called = false;
  state_->set_on_trap([&trap_called](bool is_interrupt, uint64_t trap_value,
                                     uint64_t exception_code, uint64_t epc,
                                     const Instruction* inst) -> bool {
    trap_called = true;
    EXPECT_FALSE(is_interrupt);
    EXPECT_EQ(exception_code,
              static_cast<uint64_t>(
                  mpact::sim::riscv::ExceptionCode::kIllegalInstruction));
    return true;
  });

  auto inst = std::make_unique<Instruction>(/*address=*/0, state_.get());
  inst->set_semantic_function(CoralNPUM3ZfbfminFcvtBf16S);

  frs1_reg_->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFF3f800000ULL);
  inst->AppendSource(frs1_reg_->CreateSourceOperand());

  // Test rounding mode 5 (Invalid)
  inst->AppendSource(new ImmediateOperand<uint32_t>(5));
  inst->AppendDestination(frd_reg_->CreateDestinationOperand(/*latency=*/0));
  inst->AppendDestination(fflags_reg_->CreateDestinationOperand(/*latency=*/0));

  inst->Execute();
  EXPECT_TRUE(trap_called);

  // Test rounding mode 6 (Invalid)
  trap_called = false;
  inst = std::make_unique<Instruction>(/*address=*/0, state_.get());
  inst->set_semantic_function(CoralNPUM3ZfbfminFcvtBf16S);
  inst->AppendSource(frs1_reg_->CreateSourceOperand());
  inst->AppendSource(new ImmediateOperand<uint32_t>(6));
  inst->AppendDestination(frd_reg_->CreateDestinationOperand(/*latency=*/0));
  inst->AppendDestination(fflags_reg_->CreateDestinationOperand(/*latency=*/0));

  inst->Execute();
  EXPECT_TRUE(trap_called);
}

}  // namespace
}  // namespace coralnpu::sim
