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

#include "sim/coralnpu_m3_zfbfmin_overrides.h"

#include <cstdint>
#include <memory>

#include "sim/coralnpu_v2_state.h"
#include "googletest/include/gtest/gtest.h"
#include "riscv/riscv_fp_info.h"
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

class CoralNPUM3ZfbfminOverridesTest : public ::testing::Test {
 protected:
  void SetUp() override {
    memory_ = std::make_unique<FlatDemandMemory>();
    CoralNPUV2StateConfig config;
    state_ = CreateCoralNPUV2State("test_state", RiscVXlen::RV32, memory_.get(),
                                   nullptr, &config);
    rv_fp_state_ = std::make_unique<::mpact::sim::riscv::RiscVFPState>(
        state_->csr_set(), state_.get());
    state_->set_rv_fp(rv_fp_state_.get());
    state_->mstatus()->set_fs(1);  // Initial/Enabled
    state_->mstatus()->Submit();
    frs1_reg_ = new RVFpRegister(state_.get(), "f1");
    state_->AddRegister("f1", frs1_reg_);
    frd_reg_ = new RVFpRegister(state_.get(), "f0");
    state_->AddRegister("f0", frd_reg_);
    fflags_reg_ =
        new mpact::sim::generic::Register<uint32_t>(state_.get(), "fflags");
    state_->AddRegister("fflags", fflags_reg_);

    state_->set_on_trap([this](bool is_interrupt, uint64_t trap_value,
                               uint64_t exception_code, uint64_t epc,
                               const Instruction* inst) -> bool {
      was_trap_handler_called_ = true;
      exception_code_ = exception_code;
      return true;
    });
  }

  std::unique_ptr<FlatDemandMemory> memory_;
  std::unique_ptr<CoralNPUV2State> state_;
  std::unique_ptr<::mpact::sim::riscv::RiscVFPState> rv_fp_state_;
  RVFpRegister* frs1_reg_;
  RVFpRegister* frd_reg_;
  mpact::sim::generic::Register<uint32_t>* fflags_reg_;
  bool was_trap_handler_called_ = false;
  uint64_t exception_code_ = 0;
};

TEST_F(CoralNPUM3ZfbfminOverridesTest, FcvtBf16STrapsIfFsZero) {
  state_->mstatus()->set_fs(0);  // Disabled
  state_->mstatus()->Submit();

  auto inst = std::make_unique<Instruction>(/*address=*/0x1000, state_.get());
  inst->set_semantic_function(CoralNPUM3ZfbfminFcvtBf16S);

  inst->Execute();

  EXPECT_TRUE(was_trap_handler_called_);
  EXPECT_EQ(exception_code_,
            static_cast<uint64_t>(
                mpact::sim::riscv::ExceptionCode::kIllegalInstruction));
}

TEST_F(CoralNPUM3ZfbfminOverridesTest, FcvtBf16SBasic) {
  auto inst = std::make_unique<Instruction>(/*address=*/0, state_.get());
  inst->set_semantic_function(CoralNPUM3ZfbfminFcvtBf16S);

  // FP32: 1.0 (0x3f800000) -> BF16: 1.0 (0x3f80)
  frs1_reg_->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFF3f800000ULL);
  inst->AppendSource(frs1_reg_->CreateSourceOperand());
  // Rounding mode RNE (0)
  inst->AppendSource(new ImmediateOperand<uint32_t>(0));

  inst->AppendDestination(frd_reg_->CreateDestinationOperand(/*latency=*/0));
  inst->AppendDestination(fflags_reg_->CreateDestinationOperand(/*latency=*/0));

  inst->Execute();
  EXPECT_EQ(frd_reg_->data_buffer()->Get<uint64_t>(0), 0xffffffffffff3f80ULL);
  EXPECT_EQ(state_->mstatus()->fs(), 3);
  EXPECT_EQ(fflags_reg_->data_buffer()->Get<uint32_t>(0), 0);
}

TEST_F(CoralNPUM3ZfbfminOverridesTest, FcvtBf16SRoundToNearestEven) {
  auto inst = std::make_unique<Instruction>(/*address=*/0, state_.get());
  inst->set_semantic_function(CoralNPUM3ZfbfminFcvtBf16S);

  // FP32: 1.0 + epsilon (0x3f808000), L=0, R=1, S=0 -> Round down (0x3f80)
  frs1_reg_->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFF3f808000ULL);
  inst->AppendSource(frs1_reg_->CreateSourceOperand());
  // Rounding mode RNE (0)
  inst->AppendSource(new ImmediateOperand<uint32_t>(0));

  inst->AppendDestination(frd_reg_->CreateDestinationOperand(/*latency=*/0));
  inst->AppendDestination(fflags_reg_->CreateDestinationOperand(/*latency=*/0));

  inst->Execute();
  EXPECT_EQ(frd_reg_->data_buffer()->Get<uint64_t>(0), 0xffffffffffff3f80ULL);
  EXPECT_EQ(fflags_reg_->data_buffer()->Get<uint32_t>(0), 1);  // Inexact

  // Reset and test round up case
  // FP32: 1.0 + eps (0x3f808001), L=0, R=1, S=1 -> Round up (0x3f81)
  frs1_reg_->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFF3f808001ULL);
  inst->Execute();
  EXPECT_EQ(frd_reg_->data_buffer()->Get<uint64_t>(0), 0xffffffffffff3f81ULL);
  EXPECT_EQ(fflags_reg_->data_buffer()->Get<uint32_t>(0), 1);  // Inexact
}

TEST_F(CoralNPUM3ZfbfminOverridesTest, FcvtBf16S_RTZ) {
  auto inst = std::make_unique<Instruction>(/*address=*/0, state_.get());
  inst->set_semantic_function(CoralNPUM3ZfbfminFcvtBf16S);

  // FP32: 1.0 + epsilon (0x3f800001)
  frs1_reg_->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFF3f800001ULL);
  inst->AppendSource(frs1_reg_->CreateSourceOperand());
  inst->AppendSource(new ImmediateOperand<uint32_t>(1));  // RTZ

  inst->AppendDestination(frd_reg_->CreateDestinationOperand(/*latency=*/0));
  inst->AppendDestination(fflags_reg_->CreateDestinationOperand(/*latency=*/0));

  inst->Execute();
  // Should round towards zero to 1.0 (0x3f80)
  EXPECT_EQ(frd_reg_->data_buffer()->Get<uint64_t>(0), 0xffffffffffff3f80ULL);
  EXPECT_EQ(fflags_reg_->data_buffer()->Get<uint32_t>(0), 1);
}

TEST_F(CoralNPUM3ZfbfminOverridesTest, FcvtBf16S_DynamicRm) {
  auto inst = std::make_unique<Instruction>(/*address=*/0, state_.get());
  inst->set_semantic_function(CoralNPUM3ZfbfminFcvtBf16S);

  // Set dynamic rounding mode to RTZ (1)
  rv_fp_state_->SetRoundingMode(
      mpact::sim::riscv::FPRoundingMode::kRoundTowardsZero);

  // FP32: 1.0 + epsilon (0x3f800001)
  frs1_reg_->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFF3f800001ULL);
  inst->AppendSource(frs1_reg_->CreateSourceOperand());
  inst->AppendSource(new ImmediateOperand<uint32_t>(7));  // Dynamic

  inst->AppendDestination(frd_reg_->CreateDestinationOperand(/*latency=*/0));
  inst->AppendDestination(fflags_reg_->CreateDestinationOperand(/*latency=*/0));

  inst->Execute();
  EXPECT_EQ(frd_reg_->data_buffer()->Get<uint64_t>(0), 0xffffffffffff3f80ULL);
  EXPECT_EQ(fflags_reg_->data_buffer()->Get<uint32_t>(0), 1);
}

TEST_F(CoralNPUM3ZfbfminOverridesTest, FcvtSBf16) {
  auto inst = std::make_unique<Instruction>(/*address=*/0, state_.get());
  inst->set_semantic_function(CoralNPUM3ZfbfminFcvtSBf16);

  // BF16: 1.0 (0x3f80) -> FP32: 1.0 (0x3f800000)
  frs1_reg_->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFFFFFF3f80ULL);
  inst->AppendSource(frs1_reg_->CreateSourceOperand());
  // Rounding mode RNE (0)
  inst->AppendSource(new ImmediateOperand<uint32_t>(0));

  inst->AppendDestination(frd_reg_->CreateDestinationOperand(/*latency=*/0));
  inst->AppendDestination(fflags_reg_->CreateDestinationOperand(/*latency=*/0));

  inst->Execute();
  EXPECT_EQ(frd_reg_->data_buffer()->Get<uint64_t>(0), 0xFFFFFFFF3f800000ULL);
  EXPECT_EQ(state_->mstatus()->fs(), 3);
  EXPECT_EQ(fflags_reg_->data_buffer()->Get<uint32_t>(0), 0);
}

TEST_F(CoralNPUM3ZfbfminOverridesTest, FcvtSBf16_SNaN) {
  auto inst = std::make_unique<Instruction>(/*address=*/0, state_.get());
  inst->set_semantic_function(CoralNPUM3ZfbfminFcvtSBf16);

  // BF16: SNaN (0x7f81) -> FP32: Canonical NaN (0x7fc00000)
  frs1_reg_->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFFFFFF7F81ULL);
  inst->AppendSource(frs1_reg_->CreateSourceOperand());
  // Rounding mode RNE (0)
  inst->AppendSource(new ImmediateOperand<uint32_t>(0));

  inst->AppendDestination(frd_reg_->CreateDestinationOperand(/*latency=*/0));
  inst->AppendDestination(fflags_reg_->CreateDestinationOperand(/*latency=*/0));

  inst->Execute();
  EXPECT_EQ(frd_reg_->data_buffer()->Get<uint64_t>(0), 0xFFFFFFFF7FC00000ULL);
  // Expect InvalidOp (16)
  EXPECT_EQ(fflags_reg_->data_buffer()->Get<uint32_t>(0), 16);
}

TEST_F(CoralNPUM3ZfbfminOverridesTest, FcvtSBf16_QNaN) {
  auto inst = std::make_unique<Instruction>(/*address=*/0, state_.get());
  inst->set_semantic_function(CoralNPUM3ZfbfminFcvtSBf16);

  // BF16: QNaN (0x7fc1) -> FP32: Canonical NaN (0x7fc00000)
  frs1_reg_->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFFFFFF7FC1ULL);
  inst->AppendSource(frs1_reg_->CreateSourceOperand());
  // Rounding mode RNE (0)
  inst->AppendSource(new ImmediateOperand<uint32_t>(0));

  inst->AppendDestination(frd_reg_->CreateDestinationOperand(/*latency=*/0));
  inst->AppendDestination(fflags_reg_->CreateDestinationOperand(/*latency=*/0));

  inst->Execute();
  EXPECT_EQ(frd_reg_->data_buffer()->Get<uint64_t>(0), 0xFFFFFFFF7FC00000ULL);
  // Expect no exception
  EXPECT_EQ(fflags_reg_->data_buffer()->Get<uint32_t>(0), 0);
}

TEST_F(CoralNPUM3ZfbfminOverridesTest, FcvtSBf16_Unboxed) {
  auto inst = std::make_unique<Instruction>(/*address=*/0, state_.get());
  inst->set_semantic_function(CoralNPUM3ZfbfminFcvtSBf16);

  // Unboxed SNaN
  frs1_reg_->data_buffer()->Set<uint64_t>(0, 0x0000000000007F81ULL);
  inst->AppendSource(frs1_reg_->CreateSourceOperand());
  inst->AppendSource(new ImmediateOperand<uint32_t>(0));

  inst->AppendDestination(frd_reg_->CreateDestinationOperand(/*latency=*/0));
  inst->AppendDestination(fflags_reg_->CreateDestinationOperand(/*latency=*/0));

  inst->Execute();
  EXPECT_EQ(frd_reg_->data_buffer()->Get<uint64_t>(0), 0xFFFFFFFF7FC00000ULL);
  // Expect no exception because it was unboxed (treated as Quiet NaN input)
  EXPECT_EQ(fflags_reg_->data_buffer()->Get<uint32_t>(0), 0);
}

TEST_F(CoralNPUM3ZfbfminOverridesTest, FcvtSBf16_ImproperNaNBoxedInput) {
  auto inst = std::make_unique<Instruction>(/*address=*/0, state_.get());
  inst->set_semantic_function(CoralNPUM3ZfbfminFcvtSBf16);

  // Improperly NaN-boxed SNaN (garbage in upper bits)
  frs1_reg_->data_buffer()->Set<uint64_t>(0, 0xDEADBEEFBAAD7F81ULL);
  inst->AppendSource(frs1_reg_->CreateSourceOperand());
  inst->AppendSource(new ImmediateOperand<uint32_t>(0));

  inst->AppendDestination(frd_reg_->CreateDestinationOperand(/*latency=*/0));
  inst->AppendDestination(fflags_reg_->CreateDestinationOperand(/*latency=*/0));

  inst->Execute();
  EXPECT_EQ(frd_reg_->data_buffer()->Get<uint64_t>(0), 0xFFFFFFFF7FC00000ULL);
  // Expect no exception because it was unboxed (treated as Quiet NaN input)
  EXPECT_EQ(fflags_reg_->data_buffer()->Get<uint32_t>(0), 0);
}

TEST_F(CoralNPUM3ZfbfminOverridesTest, FcvtBf16S_IllegalRoundingModes) {
  for (uint32_t rm : {5, 6, 8, 9}) {
    auto inst = std::make_unique<Instruction>(/*address=*/0, state_.get());
    inst->set_semantic_function(CoralNPUM3ZfbfminFcvtBf16S);

    frs1_reg_->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFF3f800000ULL);
    inst->AppendSource(frs1_reg_->CreateSourceOperand());
    inst->AppendSource(new ImmediateOperand<uint32_t>(rm));

    inst->AppendDestination(frd_reg_->CreateDestinationOperand(/*latency=*/0));
    inst->AppendDestination(
        fflags_reg_->CreateDestinationOperand(/*latency=*/0));

    bool trap_called = false;
    uint64_t exception_code = 0;
    state_->set_on_trap([&](bool, uint64_t, uint64_t code, uint64_t,
                            const Instruction*) -> bool {
      trap_called = true;
      exception_code = code;
      return true;
    });

    inst->Execute();
    EXPECT_TRUE(trap_called);
    EXPECT_EQ(exception_code, 2);  // kIllegalInstruction
  }
}

TEST_F(CoralNPUM3ZfbfminOverridesTest, FcvtBf16S_Invalid) {
  auto inst = std::make_unique<Instruction>(/*address=*/0, state_.get());
  inst->set_semantic_function(CoralNPUM3ZfbfminFcvtBf16S);

  // FP32: SNaN (0x7fa00000)
  frs1_reg_->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFF7fa00000ULL);
  inst->AppendSource(frs1_reg_->CreateSourceOperand());
  inst->AppendSource(new ImmediateOperand<uint32_t>(0));

  inst->AppendDestination(frd_reg_->CreateDestinationOperand(/*latency=*/0));
  inst->AppendDestination(fflags_reg_->CreateDestinationOperand(/*latency=*/0));

  inst->Execute();
  // Should convert to Canonical NaN (0x7fc0)
  EXPECT_EQ(frd_reg_->data_buffer()->Get<uint64_t>(0), 0xffffffffffff7fc0ULL);
  // Expect InvalidOp (16)
  EXPECT_EQ(fflags_reg_->data_buffer()->Get<uint32_t>(0), 16);
}

TEST_F(CoralNPUM3ZfbfminOverridesTest, FcvtBf16S_Overflow) {
  auto inst = std::make_unique<Instruction>(/*address=*/0, state_.get());
  inst->set_semantic_function(CoralNPUM3ZfbfminFcvtBf16S);

  // FP32: Max finite (0x7f7fffff)
  frs1_reg_->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFF7f7fffffULL);
  inst->AppendSource(frs1_reg_->CreateSourceOperand());
  inst->AppendSource(new ImmediateOperand<uint32_t>(0));  // RNE

  inst->AppendDestination(frd_reg_->CreateDestinationOperand(/*latency=*/0));
  inst->AppendDestination(fflags_reg_->CreateDestinationOperand(/*latency=*/0));

  inst->Execute();
  // Should round to Infinity (0x7f80)
  EXPECT_EQ(frd_reg_->data_buffer()->Get<uint64_t>(0), 0xffffffffffff7f80ULL);
  // Expect Overflow (4) | Inexact (1) = 5
  EXPECT_EQ(fflags_reg_->data_buffer()->Get<uint32_t>(0), 5);
}

TEST_F(CoralNPUM3ZfbfminOverridesTest, FcvtBf16S_DynamicRoundingMode) {
  auto inst = std::make_unique<Instruction>(/*address=*/0, state_.get());
  inst->set_semantic_function(CoralNPUM3ZfbfminFcvtBf16S);

  frs1_reg_->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFF3f800000ULL);
  inst->AppendSource(frs1_reg_->CreateSourceOperand());
  // Rounding mode Dynamic (7)
  inst->AppendSource(new ImmediateOperand<uint32_t>(7));

  inst->AppendDestination(frd_reg_->CreateDestinationOperand(/*latency=*/0));
  inst->AppendDestination(fflags_reg_->CreateDestinationOperand(/*latency=*/0));

  bool trap_called = false;
  state_->set_on_trap(
      [&](bool, uint64_t, uint64_t, uint64_t, const Instruction*) -> bool {
        trap_called = true;
        return true;
      });

  // Set rounding mode in FCSR to RNE (0)
  rv_fp_state_->SetRoundingMode(
      mpact::sim::riscv::FPRoundingMode::kRoundToNearest);
  inst->Execute();

  EXPECT_FALSE(trap_called);
  EXPECT_EQ(frd_reg_->data_buffer()->Get<uint64_t>(0), 0xffffffffffff3f80ULL);
}

TEST_F(CoralNPUM3ZfbfminOverridesTest, FcvtBf16S_DynamicRoundingModeInvalid) {
  auto inst = std::make_unique<Instruction>(/*address=*/0, state_.get());
  inst->set_semantic_function(CoralNPUM3ZfbfminFcvtBf16S);

  frs1_reg_->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFF3f800000ULL);
  inst->AppendSource(frs1_reg_->CreateSourceOperand());
  // Rounding mode Dynamic (7)
  inst->AppendSource(new ImmediateOperand<uint32_t>(7));

  inst->AppendDestination(frd_reg_->CreateDestinationOperand(/*latency=*/0));
  inst->AppendDestination(fflags_reg_->CreateDestinationOperand(/*latency=*/0));

  bool trap_called = false;
  uint64_t exception_code = 0;
  state_->set_on_trap(
      [&](bool, uint64_t, uint64_t code, uint64_t, const Instruction*) -> bool {
        trap_called = true;
        exception_code = code;
        return true;
      });

  // Set rounding mode in FCSR to an invalid value (e.g., 5)
  rv_fp_state_->SetRoundingMode(
      static_cast<mpact::sim::riscv::FPRoundingMode>(5));
  inst->Execute();

  EXPECT_TRUE(trap_called);
  EXPECT_EQ(exception_code, 2);  // kIllegalInstruction
}

}  // namespace
}  // namespace coralnpu::sim
