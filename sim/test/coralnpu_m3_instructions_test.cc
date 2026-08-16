#include "mpact/sim/generic/register.h"
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
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "sim/coralnpu_architecture.h"
#include "sim/coralnpu_m3_instructions.h"
#include "sim/coralnpu_simulator.h"
#include "sim/coralnpu_v2_state.h"
#include "googletest/include/gtest/gtest.h"
#include "absl/base/casts.h"
#include "absl/strings/str_cat.h"
#include "absl/types/span.h"
#include "riscv/riscv_fp_info.h"
#include "riscv/riscv_fp_state.h"
#include "riscv/riscv_register.h"
#include "riscv/riscv_state.h"
#include "riscv/riscv_vector_state.h"
#include "mpact/sim/generic/data_buffer.h"
#include "mpact/sim/generic/immediate_operand.h"
#include "mpact/sim/generic/instruction.h"
#include "mpact/sim/generic/operand_interface.h"
#include "mpact/sim/generic/register.h"

namespace {

using ::coralnpu::sim::CoralNPUV2State;
using ::mpact::sim::generic::ImmediateOperand;
using ::mpact::sim::generic::Instruction;
using ::mpact::sim::generic::RegisterBase;
using ::mpact::sim::generic::SourceOperandInterface;
using ::mpact::sim::riscv::ExceptionCode;
using ::mpact::sim::riscv::FPExceptions;
using ::mpact::sim::riscv::FPRoundingMode;
using ::mpact::sim::riscv::RiscVFPState;
using ::mpact::sim::riscv::RiscVVectorState;
using ::mpact::sim::riscv::RV32Register;
using ::mpact::sim::riscv::RV32VectorDestinationOperand;
using ::mpact::sim::riscv::RV32VectorSourceOperand;
using ::mpact::sim::riscv::RV32VectorTrueOperand;
using ::mpact::sim::riscv::RVFpRegister;
using ::mpact::sim::riscv::RVVectorRegister;

class CoralNPUM3InstructionsTest : public ::testing::Test {
 protected:
  void SetUp() override {
    coralnpu::sim::CoralNPUSimulatorOptions options;
    options.architecture = coralnpu::sim::Architecture::kM3;
    simulator_ = std::make_unique<coralnpu::sim::CoralNPUSimulator>(options);
    state_ = simulator_->state();

    rv_vector_ = state_->rv_vector();
    rv_fp_ = state_->rv_fp();

    state_->set_on_trap([this](bool, uint64_t trap_value,
                               uint64_t exception_code, uint64_t epc,
                               const Instruction*) -> bool {
      was_trap_handler_called_ = true;
      exception_code_ = static_cast<ExceptionCode>(exception_code);
      trap_value_ = trap_value;
      epc_ = epc;
      return false;
    });

    for (int i = 0; i < 32; i++) {
      auto [reg, success] =
          state_->GetRegister<RVVectorRegister>(absl::StrCat("v", i));
      v_regs_.push_back(reg);
    }

    // Enable FP and Vector extensions.
    state_->mstatus()->set_fs(1);
    state_->mstatus()->Submit();
    state_->mstatus()->Set(
        static_cast<uint64_t>(state_->mstatus()->GetUint64() | 0x600ULL));
  }

  // Set vector state: sew is determined by vtype, vl is vector length.
  void SetupVectorState(uint32_t vtype, int vl) {
    rv_vector_->SetVectorType(vtype);
    rv_vector_->set_vector_length(vl);
  }

  // Create Vfwcvtbf16ffv instruction object.
  std::unique_ptr<Instruction> CreateVfwcvtbf16ffvInstruction(
      int dest_reg_idx, int num_regs_dst, int src_reg_idx, int num_regs_src,
      SourceOperandInterface* mask_operand = nullptr) {
    auto inst = std::make_unique<Instruction>(0, state_);
    inst->set_semantic_function(&coralnpu::sim::Vfwcvtbf16ffv);

    std::vector<RegisterBase*> dest_regs;
    for (int element_idx = 0; element_idx < num_regs_dst; ++element_idx) {
      dest_regs.push_back(v_regs_[dest_reg_idx + element_idx]);
    }
    inst->AppendDestination(new RV32VectorDestinationOperand(
        absl::MakeSpan(dest_regs), /*latency=*/0,
        absl::StrCat("v", dest_reg_idx)));
    auto* fflags_reg = state_->rv_fp()->fflags();
    if (fflags_reg != nullptr) {
      inst->AppendDestination(
          fflags_reg->CreateSetDestinationOperand(/*latency=*/0, "fflags"));
    }

    std::vector<RegisterBase*> src_regs;
    for (int element_idx = 0; element_idx < num_regs_src; ++element_idx) {
      src_regs.push_back(v_regs_[src_reg_idx + element_idx]);
    }
    inst->AppendSource(new RV32VectorSourceOperand(
        absl::MakeSpan(src_regs), absl::StrCat("v", src_reg_idx)));

    if (mask_operand == nullptr) {
      mask_operand = new RV32VectorTrueOperand(state_);
    }
    inst->AppendSource(mask_operand);

    return inst;
  }

  // Create Vfncvtbf16ffw instruction object.
  std::unique_ptr<Instruction> CreateVfncvtbf16ffwInstruction(
      int dest_reg_idx, int num_regs_dst, int src_reg_idx, int num_regs_src,
      SourceOperandInterface* mask_operand = nullptr) {
    auto inst = std::make_unique<Instruction>(0, state_);
    inst->set_semantic_function(&coralnpu::sim::Vfncvtbf16ffw);

    std::vector<RegisterBase*> dest_regs;
    for (int element_idx = 0; element_idx < num_regs_dst; ++element_idx) {
      dest_regs.push_back(v_regs_[dest_reg_idx + element_idx]);
    }
    inst->AppendDestination(new RV32VectorDestinationOperand(
        absl::MakeSpan(dest_regs), /*latency=*/0,
        absl::StrCat("v", dest_reg_idx)));
    auto* fflags_reg = state_->rv_fp()->fflags();
    if (fflags_reg != nullptr) {
      inst->AppendDestination(
          fflags_reg->CreateSetDestinationOperand(/*latency=*/0, "fflags"));
    }

    std::vector<RegisterBase*> src_regs;
    for (int element_idx = 0; element_idx < num_regs_src; ++element_idx) {
      src_regs.push_back(v_regs_[src_reg_idx + element_idx]);
    }
    inst->AppendSource(new RV32VectorSourceOperand(
        absl::MakeSpan(src_regs), absl::StrCat("v", src_reg_idx)));

    if (mask_operand == nullptr) {
      mask_operand = new RV32VectorTrueOperand(state_);
    }
    inst->AppendSource(mask_operand);

    return inst;
  }

  // Create FcvtBf16S instruction object.
  std::unique_ptr<Instruction> CreateFcvtBf16SInstruction(
      const std::string& frd_name, const std::string& frs1_name, int rm_val) {
    auto inst = std::make_unique<Instruction>(0, state_);
    inst->set_semantic_function(&coralnpu::sim::FcvtBf16S);

    auto [frd_reg, success_dst] = state_->GetRegister<RVFpRegister>(frd_name);
    inst->AppendDestination(frd_reg->CreateDestinationOperand(/*latency=*/0));

    auto* fflags_reg = state_->rv_fp()->fflags();
    if (fflags_reg != nullptr) {
      inst->AppendDestination(
          fflags_reg->CreateSetDestinationOperand(/*latency=*/0, "fflags"));
    }

    auto [frs1_reg, success_src] = state_->GetRegister<RVFpRegister>(frs1_name);
    inst->AppendSource(frs1_reg->CreateSourceOperand());
    inst->AppendSource(new ImmediateOperand<int32_t>(rm_val));

    return inst;
  }

  // Create FcvtSBf16 instruction object.
  std::unique_ptr<Instruction> CreateFcvtSBf16Instruction(
      const std::string& frd_name, const std::string& frs1_name,
      int rm_val = 0) {
    auto inst = std::make_unique<Instruction>(0, state_);
    inst->set_semantic_function(&coralnpu::sim::FcvtSBf16);

    auto [frd_reg, success_dst] = state_->GetRegister<RVFpRegister>(frd_name);
    inst->AppendDestination(frd_reg->CreateDestinationOperand(/*latency=*/0));

    auto* fflags_reg = state_->rv_fp()->fflags();
    if (fflags_reg != nullptr) {
      inst->AppendDestination(
          fflags_reg->CreateSetDestinationOperand(/*latency=*/0, "fflags"));
    }

    auto [frs1_reg, success_src] = state_->GetRegister<RVFpRegister>(frs1_name);
    inst->AppendSource(frs1_reg->CreateSourceOperand());
    inst->AppendSource(new ImmediateOperand<int32_t>(rm_val));

    return inst;
  }

  std::unique_ptr<coralnpu::sim::CoralNPUSimulator> simulator_;
  CoralNPUV2State* state_;
  RiscVVectorState* rv_vector_;
  RiscVFPState* rv_fp_;
  std::vector<RVVectorRegister*> v_regs_;

  bool was_trap_handler_called_ = false;
  ExceptionCode exception_code_ = ExceptionCode::kBreakpoint;
  uint64_t trap_value_ = 0;
  uint64_t epc_ = 0;
};

// LMUL=2 test for Vfwcvtbf16ffv.
TEST_F(CoralNPUM3InstructionsTest, Vfwcvtbf16ffv_LMUL2) {
  // vtype = 9 (SEW=16, LMUL=2)
  SetupVectorState(/*vtype=*/9, /*vl=*/8);

  // Initialize input register group (v2, v3) with BF16 values representing
  // numbers 1.0f (0x3f80) to 8.0f.
  uint16_t bf16_inputs[8] = {0x3f80, 0x4000, 0x4040, 0x4080,
                             0x40a0, 0x40c0, 0x40e0, 0x4100};
  for (int element_idx = 0; element_idx < 8; ++element_idx) {
    int reg_idx = 2 + (element_idx / 8);
    int offset = element_idx % 8;
    v_regs_[reg_idx]->data_buffer()->Set<uint16_t>(offset,
                                                   bf16_inputs[element_idx]);
  }

  // Widening instruction vfwcvtbf16ffv v4, v2
  // Destination group: v4, v5, v6, v7 (num_regs_dst = 4)
  // Source group: v2, v3 (num_regs_src = 2)
  auto inst = CreateVfwcvtbf16ffvInstruction(/*dest_reg_idx=*/4,
                                             /*num_regs_dst=*/4,
                                             /*src_reg_idx=*/2,
                                             /*num_regs_src=*/2);
  inst->Execute();

  // Verify elements in destination registers.
  float expected_outputs[8] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
  for (int element_idx = 0; element_idx < 8; ++element_idx) {
    int reg_idx = 4 + (element_idx / 4);
    int offset = element_idx % 4;
    float result = v_regs_[reg_idx]->data_buffer()->Get<float>(offset);
    EXPECT_EQ(result, expected_outputs[element_idx]);
  }
}

// LMUL=4 test for Vfwcvtbf16ffv.
TEST_F(CoralNPUM3InstructionsTest, Vfwcvtbf16ffv_LMUL4) {
  // vtype = 10 (SEW=16, LMUL=4)
  SetupVectorState(/*vtype=*/10, /*vl=*/16);

  // Initialize input register group (v4, v5, v6, v7) with BF16 values.
  for (int element_idx = 0; element_idx < 16; ++element_idx) {
    int reg_idx = 4 + (element_idx / 8);
    int offset = element_idx % 8;
    // Store 1.0f (0x3f80) in each element.
    v_regs_[reg_idx]->data_buffer()->Set<uint16_t>(offset, 0x3f80);
  }

  // Widening instruction vfwcvtbf16ffv v8, v4
  // Destination group: v8 to v15 (num_regs_dst = 8)
  // Source group: v4 to v7 (num_regs_src = 4)
  auto inst = CreateVfwcvtbf16ffvInstruction(/*dest_reg_idx=*/8,
                                             /*num_regs_dst=*/8,
                                             /*src_reg_idx=*/4,
                                             /*num_regs_src=*/4);
  inst->Execute();

  // Verify elements in destination registers are exactly 1.0f.
  for (int element_idx = 0; element_idx < 16; ++element_idx) {
    int reg_idx = 8 + (element_idx / 4);
    int offset = element_idx % 4;
    float result = v_regs_[reg_idx]->data_buffer()->Get<float>(offset);
    EXPECT_EQ(result, 1.0f);
  }
}

// LMUL=2 test for Vfncvtbf16ffw.
TEST_F(CoralNPUM3InstructionsTest, Vfncvtbf16ffw_LMUL2) {
  // vtype = 9 (SEW=16, LMUL=2 for destination)
  SetupVectorState(/*vtype=*/9, /*vl=*/8);

  // Initialize input source register group (v4, v5, v6, v7) (num_regs_src = 4)
  // with float32 values 1.0f to 8.0f.
  float f32_inputs[8] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
  for (int element_idx = 0; element_idx < 8; ++element_idx) {
    int reg_idx = 4 + (element_idx / 4);
    int offset = element_idx % 4;
    v_regs_[reg_idx]->data_buffer()->Set<float>(offset,
                                                f32_inputs[element_idx]);
  }

  // Narrowing instruction vfncvtbf16ffw v2, v4
  // Destination group: v2, v3 (num_regs_dst = 2)
  // Source group: v4 to v7 (num_regs_src = 4)
  auto inst = CreateVfncvtbf16ffwInstruction(/*dest_reg_idx=*/2,
                                             /*num_regs_dst=*/2,
                                             /*src_reg_idx=*/4,
                                             /*num_regs_src=*/4);
  inst->Execute();

  // Verify elements in destination registers.
  uint16_t expected_outputs[8] = {0x3f80, 0x4000, 0x4040, 0x4080,
                                  0x40a0, 0x40c0, 0x40e0, 0x4100};
  for (int element_idx = 0; element_idx < 8; ++element_idx) {
    int reg_idx = 2 + (element_idx / 8);
    int offset = element_idx % 8;
    uint16_t result = v_regs_[reg_idx]->data_buffer()->Get<uint16_t>(offset);
    EXPECT_EQ(result, expected_outputs[element_idx]);
  }
}

// LMUL=4 test for Vfncvtbf16ffw.
TEST_F(CoralNPUM3InstructionsTest, Vfncvtbf16ffw_LMUL4) {
  // vtype = 10 (SEW=16, LMUL=4 for destination)
  SetupVectorState(/*vtype=*/10, /*vl=*/16);

  // Initialize input source register group (v8 to v15) (num_regs_src = 8)
  // with float32 2.0f.
  for (int element_idx = 0; element_idx < 16; ++element_idx) {
    int reg_idx = 8 + (element_idx / 4);
    int offset = element_idx % 4;
    v_regs_[reg_idx]->data_buffer()->Set<float>(offset, 2.0f);
  }

  // Narrowing instruction vfncvtbf16ffw v4, v8
  // Destination group: v4 to v7 (num_regs_dst = 4)
  // Source group: v8 to v15 (num_regs_src = 8)
  auto inst = CreateVfncvtbf16ffwInstruction(/*dest_reg_idx=*/4,
                                             /*num_regs_dst=*/4,
                                             /*src_reg_idx=*/8,
                                             /*num_regs_src=*/8);
  inst->Execute();

  // Verify elements in destination registers are exactly 2.0f in BF16 (0x4000).
  for (int element_idx = 0; element_idx < 16; ++element_idx) {
    int reg_idx = 4 + (element_idx / 8);
    int offset = element_idx % 8;
    uint16_t result = v_regs_[reg_idx]->data_buffer()->Get<uint16_t>(offset);
    EXPECT_EQ(result, 0x4000);
  }
}

// Overlap check for Vfncvtbf16ffw with LMUL=2.
TEST_F(CoralNPUM3InstructionsTest, Vfncvtbf16ffw_OverlapLMUL2) {
  // vtype = 9 (SEW=16, LMUL=2 for destination)
  SetupVectorState(/*vtype=*/9, /*vl=*/8);

  // Overlapping registers: dest starts at v2, source starts at v2.
  auto inst = CreateVfncvtbf16ffwInstruction(/*dest_reg_idx=*/2,
                                             /*num_regs_dst=*/2,
                                             /*src_reg_idx=*/2,
                                             /*num_regs_src=*/4);
  inst->Execute();

  // Should trap as illegal instruction.
  EXPECT_TRUE(was_trap_handler_called_);
  EXPECT_EQ(exception_code_, ExceptionCode::kIllegalInstruction);
}

// Overlap check for Vfncvtbf16ffw with LMUL=4.
TEST_F(CoralNPUM3InstructionsTest, Vfncvtbf16ffw_OverlapLMUL4) {
  // vtype = 10 (SEW=16, LMUL=4 for destination)
  SetupVectorState(/*vtype=*/10, /*vl=*/16);

  // Overlapping registers: dest starts at v4, source starts at v6.
  auto inst = CreateVfncvtbf16ffwInstruction(/*dest_reg_idx=*/4,
                                             /*num_regs_dst=*/4,
                                             /*src_reg_idx=*/6,
                                             /*num_regs_src=*/8);
  inst->Execute();

  // Should trap as illegal instruction.
  EXPECT_TRUE(was_trap_handler_called_);
  EXPECT_EQ(exception_code_, ExceptionCode::kIllegalInstruction);
}

// Overlap check for Vfwcvtbf16ffv with LMUL=2.
TEST_F(CoralNPUM3InstructionsTest, Vfwcvtbf16ffv_OverlapLMUL2) {
  // vtype = 8 (SEW=16, LMUL=1 for source, which is LMUL=2 for destination)
  SetupVectorState(/*vtype=*/8, /*vl=*/8);

  // Overlapping registers: dest starts at v2, source starts at v2.
  auto inst = CreateVfwcvtbf16ffvInstruction(/*dest_reg_idx=*/2,
                                             /*num_regs_dst=*/2,
                                             /*src_reg_idx=*/2,
                                             /*num_regs_src=*/1);
  inst->Execute();

  // Should trap as illegal instruction.
  EXPECT_TRUE(was_trap_handler_called_);
  EXPECT_EQ(exception_code_, ExceptionCode::kIllegalInstruction);
}

// Test vstart element-wise offset skipping for Vfwcvtbf16ffv.
TEST_F(CoralNPUM3InstructionsTest, Vfwcvtbf16ffv_VStartSkipping) {
  SetupVectorState(/*vtype=*/8, /*vl=*/4);
  rv_vector_->set_vstart(2);

  // Initialize source elements.
  v_regs_[1]->data_buffer()->Set<uint16_t>(0, 0x3f80);  // 1.0
  v_regs_[1]->data_buffer()->Set<uint16_t>(1, 0x4000);  // 2.0
  v_regs_[1]->data_buffer()->Set<uint16_t>(2, 0x4040);  // 3.0
  v_regs_[1]->data_buffer()->Set<uint16_t>(3, 0x4080);  // 4.0

  // Set initial destination register values to 0.
  v_regs_[2]->data_buffer()->Set<float>(0, 0.0f);
  v_regs_[2]->data_buffer()->Set<float>(1, 0.0f);
  v_regs_[2]->data_buffer()->Set<float>(2, 0.0f);
  v_regs_[2]->data_buffer()->Set<float>(3, 0.0f);

  auto inst = CreateVfwcvtbf16ffvInstruction(/*dest_reg_idx=*/2,
                                             /*num_regs_dst=*/2,
                                             /*src_reg_idx=*/1,
                                             /*num_regs_src=*/1);
  inst->Execute();

  // Verify: elements 0 and 1 are skipped (unchanged, i.e., 0.0f).
  // Elements 2 and 3 are converted (3.0f and 4.0f).
  EXPECT_EQ(v_regs_[2]->data_buffer()->Get<float>(0), 0.0f);
  EXPECT_EQ(v_regs_[2]->data_buffer()->Get<float>(1), 0.0f);
  EXPECT_EQ(v_regs_[2]->data_buffer()->Get<float>(2), 3.0f);
  EXPECT_EQ(v_regs_[2]->data_buffer()->Get<float>(3), 4.0f);
}

// Test vstart element-wise offset skipping for Vfncvtbf16ffw.
TEST_F(CoralNPUM3InstructionsTest, Vfncvtbf16ffw_VStartSkipping) {
  SetupVectorState(/*vtype=*/8, /*vl=*/4);
  rv_vector_->set_vstart(2);

  // Initialize source elements.
  v_regs_[2]->data_buffer()->Set<float>(0, 1.0f);
  v_regs_[2]->data_buffer()->Set<float>(1, 2.0f);
  v_regs_[2]->data_buffer()->Set<float>(2, 3.0f);
  v_regs_[2]->data_buffer()->Set<float>(3, 4.0f);

  // Set initial destination register values to 0.
  v_regs_[1]->data_buffer()->Set<uint16_t>(0, 0);
  v_regs_[1]->data_buffer()->Set<uint16_t>(1, 0);
  v_regs_[1]->data_buffer()->Set<uint16_t>(2, 0);
  v_regs_[1]->data_buffer()->Set<uint16_t>(3, 0);

  auto inst = CreateVfncvtbf16ffwInstruction(/*dest_reg_idx=*/1,
                                             /*num_regs_dst=*/1,
                                             /*src_reg_idx=*/2,
                                             /*num_regs_src=*/2);
  inst->Execute();

  // Verify: elements 0 and 1 are skipped (unchanged, i.e., 0).
  // Elements 2 and 3 are converted (0x4040 and 0x4080).
  EXPECT_EQ(v_regs_[1]->data_buffer()->Get<uint16_t>(0), 0);
  EXPECT_EQ(v_regs_[1]->data_buffer()->Get<uint16_t>(1), 0);
  EXPECT_EQ(v_regs_[1]->data_buffer()->Get<uint16_t>(2), 0x4040);
  EXPECT_EQ(v_regs_[1]->data_buffer()->Get<uint16_t>(3), 0x4080);
}

// Test vstart is cleared upon completion for Vfwcvtbf16ffv.
TEST_F(CoralNPUM3InstructionsTest, Vfwcvtbf16ffv_VStartZeroing) {
  SetupVectorState(/*vtype=*/8, /*vl=*/4);
  rv_vector_->set_vstart(1);

  auto inst = CreateVfwcvtbf16ffvInstruction(/*dest_reg_idx=*/2,
                                             /*num_regs_dst=*/2,
                                             /*src_reg_idx=*/1,
                                             /*num_regs_src=*/1);
  inst->Execute();

  EXPECT_EQ(rv_vector_->vstart(), 0);
}

// Test vstart is cleared upon completion for Vfncvtbf16ffw.
TEST_F(CoralNPUM3InstructionsTest, Vfncvtbf16ffw_VStartZeroing) {
  SetupVectorState(/*vtype=*/8, /*vl=*/4);
  rv_vector_->set_vstart(1);

  auto inst = CreateVfncvtbf16ffwInstruction(/*dest_reg_idx=*/1,
                                             /*num_regs_dst=*/1,
                                             /*src_reg_idx=*/2,
                                             /*num_regs_src=*/2);
  inst->Execute();

  EXPECT_EQ(rv_vector_->vstart(), 0);
}

// Test invalid dynamic rounding mode trap on Vfncvtbf16ffw (modes 5, 6, 7).
TEST_F(CoralNPUM3InstructionsTest, Vfncvtbf16ffw_FrmInvalidTrap) {
  for (uint32_t invalid_frm : {5U, 6U, 7U}) {
    was_trap_handler_called_ = false;
    SetupVectorState(/*vtype=*/8, /*vl=*/4);

    rv_fp_->frm()->Write(invalid_frm);

    auto inst = CreateVfncvtbf16ffwInstruction(/*dest_reg_idx=*/1,
                                               /*num_regs_dst=*/1,
                                               /*src_reg_idx=*/2,
                                               /*num_regs_src=*/2);
    inst->Execute();

    EXPECT_TRUE(was_trap_handler_called_);
    EXPECT_EQ(exception_code_, ExceptionCode::kIllegalInstruction);
  }
}

// Test Vfncvtbf16ffw with all rounding modes (0-4).
TEST_F(CoralNPUM3InstructionsTest, Vfncvtbf16ffw_RoundingModes0To4) {
  struct RoundingTestCase {
    FPRoundingMode mode;
    uint16_t expected_pos;
    uint16_t expected_neg;
  };

  std::vector<RoundingTestCase> test_cases = {
      {FPRoundingMode::kRoundToNearest, 0x3f80, 0xbf80},          // RNE
      {FPRoundingMode::kRoundTowardsZero, 0x3f80, 0xbf80},        // RTZ
      {FPRoundingMode::kRoundDown, 0x3f80, 0xbf81},               // RDN
      {FPRoundingMode::kRoundUp, 0x3f81, 0xbf80},                 // RUP
      {FPRoundingMode::kRoundToNearestTiesToMax, 0x3f81, 0xbf81}  // RMM
  };

  for (const auto& tc : test_cases) {
    SetupVectorState(/*vtype=*/8, /*vl=*/2);
    rv_fp_->fflags()->Write(0U);
    rv_fp_->frm()->Write(static_cast<uint32_t>(tc.mode));

    // Inputs: 1.00390625f (0x3f808000) and -1.00390625f (0xbf808000)
    float pos_val = absl::bit_cast<float>(0x3f808000U);
    float neg_val = absl::bit_cast<float>(0xbf808000U);

    v_regs_[2]->data_buffer()->Set<float>(0, pos_val);
    v_regs_[2]->data_buffer()->Set<float>(1, neg_val);

    // Set destination v1 elements to 0
    v_regs_[1]->data_buffer()->Set<uint16_t>(0, 0);
    v_regs_[1]->data_buffer()->Set<uint16_t>(1, 0);

    auto inst = CreateVfncvtbf16ffwInstruction(/*dest_reg_idx=*/1,
                                               /*num_regs_dst=*/1,
                                               /*src_reg_idx=*/2,
                                               /*num_regs_src=*/2);
    inst->Execute();

    uint16_t res_pos = v_regs_[1]->data_buffer()->Get<uint16_t>(0);
    uint16_t res_neg = v_regs_[1]->data_buffer()->Get<uint16_t>(1);

    EXPECT_EQ(res_pos, tc.expected_pos)
        << "Failed for mode " << static_cast<int>(tc.mode)
        << " (positive input)";
    EXPECT_EQ(res_neg, tc.expected_neg)
        << "Failed for mode " << static_cast<int>(tc.mode)
        << " (negative input)";
  }
}

// Test Overflow exceptions signaling in fflags for Vfncvtbf16ffw.
TEST_F(CoralNPUM3InstructionsTest, Vfncvtbf16ffw_OverflowFlag) {
  SetupVectorState(/*vtype=*/8, /*vl=*/4);
  rv_fp_->fflags()->Write(0U);
  rv_fp_->frm()->Write(static_cast<uint32_t>(FPRoundingMode::kRoundToNearest));

  // 1e38f is a large finite value within BFloat16 range (maximum is ~3.39e38f),
  // as BFloat16 shares the same 8-bit exponent as Float32. It should NOT
  // trigger overflow, but should raise the inexact flag because of precision
  // loss.
  v_regs_[2]->data_buffer()->Set<float>(0, 1e38f);

  auto inst1 = CreateVfncvtbf16ffwInstruction(/*dest_reg_idx=*/1,
                                              /*num_regs_dst=*/1,
                                              /*src_reg_idx=*/2,
                                              /*num_regs_src=*/2);
  inst1->Execute();

  uint32_t fflags1 = rv_fp_->fflags()->GetUint32();
  EXPECT_EQ(fflags1 & static_cast<uint32_t>(FPExceptions::kOverflow), 0);
  EXPECT_NE(fflags1 & static_cast<uint32_t>(FPExceptions::kInexact), 0);

  // Clear fflags and run with max finite float, which rounds up to infinity
  // in BFloat16, triggering an Overflow.
  rv_fp_->fflags()->Write(0U);
  v_regs_[2]->data_buffer()->Set<float>(0, std::numeric_limits<float>::max());

  auto inst2 = CreateVfncvtbf16ffwInstruction(/*dest_reg_idx=*/1,
                                              /*num_regs_dst=*/1,
                                              /*src_reg_idx=*/2,
                                              /*num_regs_src=*/2);
  inst2->Execute();

  uint32_t fflags2 = rv_fp_->fflags()->GetUint32();
  EXPECT_NE(fflags2 & static_cast<uint32_t>(FPExceptions::kOverflow), 0);
  EXPECT_NE(fflags2 & static_cast<uint32_t>(FPExceptions::kInexact), 0);
}

// Test Underflow exceptions signaling in fflags for Vfncvtbf16ffw.
TEST_F(CoralNPUM3InstructionsTest, Vfncvtbf16ffw_UnderflowFlag) {
  SetupVectorState(/*vtype=*/8, /*vl=*/4);
  rv_fp_->fflags()->Write(0U);
  rv_fp_->frm()->Write(static_cast<uint32_t>(FPRoundingMode::kRoundToNearest));

  // BF16 minimum normal is 1.17e-38, minimum subnormal is 9.18e-41.
  // 1e-42f will definitely underflow BF16 and inexactly round to 0.
  v_regs_[2]->data_buffer()->Set<float>(0, 1e-42f);

  auto inst = CreateVfncvtbf16ffwInstruction(/*dest_reg_idx=*/1,
                                             /*num_regs_dst=*/1,
                                             /*src_reg_idx=*/2,
                                             /*num_regs_src=*/2);
  inst->Execute();

  uint32_t fflags = rv_fp_->fflags()->GetUint32();
  EXPECT_NE(fflags & static_cast<uint32_t>(FPExceptions::kUnderflow), 0);
  EXPECT_NE(fflags & static_cast<uint32_t>(FPExceptions::kInexact), 0);
}

// Test tininess detection after rounding for Vfncvtbf16ffw.
// A value that is subnormal before rounding but rounds up to minimum normal
// should NOT trigger underflow if tininess is detected after rounding.
TEST_F(CoralNPUM3InstructionsTest, Vfncvtbf16ffw_TininessAfterRounding) {
  SetupVectorState(/*vtype=*/8, /*vl=*/4);
  rv_fp_->fflags()->Write(0U);
  rv_fp_->frm()->Write(static_cast<uint32_t>(FPRoundingMode::kRoundToNearest));

  // 0x007F8000 corresponds to a float value that is subnormal in BF16
  // (exponent 0) but has a guard bit of 1 and LSB of 1, so it rounds up to
  // 0x0080 (minimum normal).
  uint32_t f32_bits = 0x007F8000;
  float f32 = absl::bit_cast<float>(f32_bits);
  v_regs_[2]->data_buffer()->Set<float>(0, f32);

  auto inst = CreateVfncvtbf16ffwInstruction(/*dest_reg_idx=*/1,
                                             /*num_regs_dst=*/1,
                                             /*src_reg_idx=*/2,
                                             /*num_regs_src=*/2);
  inst->Execute();

  // Verify result is 0x0080.
  uint16_t res = v_regs_[1]->data_buffer()->Get<uint16_t>(0);
  EXPECT_EQ(res, 0x0080);

  // Should be inexact but NOT underflow.
  uint32_t fflags = rv_fp_->fflags()->GetUint32();
  EXPECT_NE(fflags & static_cast<uint32_t>(FPExceptions::kInexact), 0);
  EXPECT_EQ(fflags & static_cast<uint32_t>(FPExceptions::kUnderflow), 0);
}

// Test Inexact exceptions signaling in fflags for Vfncvtbf16ffw.
TEST_F(CoralNPUM3InstructionsTest, Vfncvtbf16ffw_InexactFlag) {
  SetupVectorState(/*vtype=*/8, /*vl=*/4);
  rv_fp_->fflags()->Write(0U);
  rv_fp_->frm()->Write(static_cast<uint32_t>(FPRoundingMode::kRoundToNearest));

  // 1.0001f is inexact in BF16 (requires more than 7 fraction bits to represent
  // precisely).
  v_regs_[2]->data_buffer()->Set<float>(0, 1.0001f);

  auto inst = CreateVfncvtbf16ffwInstruction(/*dest_reg_idx=*/1,
                                             /*num_regs_dst=*/1,
                                             /*src_reg_idx=*/2,
                                             /*num_regs_src=*/2);
  inst->Execute();

  uint32_t fflags = rv_fp_->fflags()->GetUint32();
  EXPECT_NE(fflags & static_cast<uint32_t>(FPExceptions::kInexact), 0);
}

// Test Quiet NaN propagation on Vfwcvtbf16ffv.
TEST_F(CoralNPUM3InstructionsTest, Vfwcvtbf16ffv_QNaNPropagation) {
  SetupVectorState(/*vtype=*/8, /*vl=*/4);
  rv_fp_->fflags()->Write(0U);

  // Quiet NaN in BF16: exponent=0xFF, fraction=0x40.
  uint16_t qnan = 0x7FC0;
  v_regs_[1]->data_buffer()->Set<uint16_t>(0, qnan);

  auto inst = CreateVfwcvtbf16ffvInstruction(/*dest_reg_idx=*/2,
                                             /*num_regs_dst=*/2,
                                             /*src_reg_idx=*/1,
                                             /*num_regs_src=*/1);
  inst->Execute();

  // Should result in a canonical Float32 NaN (0x7FC00000).
  float res = v_regs_[2]->data_buffer()->Get<float>(0);
  uint32_t res_bits = absl::bit_cast<uint32_t>(res);
  EXPECT_EQ(res_bits, 0x7FC00000);

  // Under standard behavior, quiet NaN propagation does NOT signal Invalid
  // Operation.
  EXPECT_EQ(rv_fp_->fflags()->GetUint32() &
                static_cast<uint32_t>(FPExceptions::kInvalidOp),
            0);
}

// Test Signaling NaN exception signaling on Vfwcvtbf16ffv.
TEST_F(CoralNPUM3InstructionsTest, Vfwcvtbf16ffv_SNaNSignaling) {
  SetupVectorState(/*vtype=*/8, /*vl=*/4);
  rv_fp_->fflags()->Write(0U);

  // Signaling NaN in BF16: exponent=0xFF, fraction=0x01 (bit 6 of fraction is
  // 0).
  uint16_t snan = 0x7F81;
  v_regs_[1]->data_buffer()->Set<uint16_t>(0, snan);

  auto inst = CreateVfwcvtbf16ffvInstruction(/*dest_reg_idx=*/2,
                                             /*num_regs_dst=*/2,
                                             /*src_reg_idx=*/1,
                                             /*num_regs_src=*/1);
  inst->Execute();

  // Should propagate canonical Float32 NaN.
  float res = v_regs_[2]->data_buffer()->Get<float>(0);
  uint32_t res_bits = absl::bit_cast<uint32_t>(res);
  EXPECT_EQ(res_bits, 0x7FC00000);

  // Must signal Invalid Operation.
  EXPECT_NE(rv_fp_->fflags()->GetUint32() &
                static_cast<uint32_t>(FPExceptions::kInvalidOp),
            0);
}

// Test Quiet NaN propagation on Vfncvtbf16ffw.
TEST_F(CoralNPUM3InstructionsTest, Vfncvtbf16ffw_QNaNPropagation) {
  SetupVectorState(/*vtype=*/8, /*vl=*/4);
  rv_fp_->fflags()->Write(0U);

  // Quiet NaN in Float32: 0x7FC00000 (exponent=0xFF, fraction bit 22 = 1).
  uint32_t qnan_bits = 0x7FC00000;
  float qnan = absl::bit_cast<float>(qnan_bits);
  v_regs_[2]->data_buffer()->Set<float>(0, qnan);

  auto inst = CreateVfncvtbf16ffwInstruction(/*dest_reg_idx=*/1,
                                             /*num_regs_dst=*/1,
                                             /*src_reg_idx=*/2,
                                             /*num_regs_src=*/2);
  inst->Execute();

  // Should result in a canonical BF16 NaN (0x7FC0).
  uint16_t res = v_regs_[1]->data_buffer()->Get<uint16_t>(0);
  EXPECT_EQ(res, 0x7FC0);

  // Under standard behavior, quiet NaN propagation does NOT signal Invalid
  // Operation.
  EXPECT_EQ(rv_fp_->fflags()->GetUint32() &
                static_cast<uint32_t>(FPExceptions::kInvalidOp),
            0);
}

// Test Signaling NaN exception signaling on Vfncvtbf16ffw.
TEST_F(CoralNPUM3InstructionsTest, Vfncvtbf16ffw_SNaNSignaling) {
  SetupVectorState(/*vtype=*/8, /*vl=*/4);
  rv_fp_->fflags()->Write(0U);

  // Signaling NaN in Float32: 0x7F800001 (exponent=0xFF, fraction bit 22 = 0).
  uint32_t snan_bits = 0x7F800001;
  float snan = absl::bit_cast<float>(snan_bits);
  v_regs_[2]->data_buffer()->Set<float>(0, snan);

  auto inst = CreateVfncvtbf16ffwInstruction(/*dest_reg_idx=*/1,
                                             /*num_regs_dst=*/1,
                                             /*src_reg_idx=*/2,
                                             /*num_regs_src=*/2);
  inst->Execute();

  // Should propagate canonical BF16 NaN.
  uint16_t res = v_regs_[1]->data_buffer()->Get<uint16_t>(0);
  EXPECT_EQ(res, 0x7FC0);

  // Must signal Invalid Operation.
  EXPECT_NE(rv_fp_->fflags()->GetUint32() &
                static_cast<uint32_t>(FPExceptions::kInvalidOp),
            0);
}

// Property-Based Testing for Vfwcvtbf16ffv.
TEST_F(CoralNPUM3InstructionsTest, PropertyBasedVfwcvtbf16ffv) {
  SetupVectorState(/*vtype=*/8, /*vl=*/8);

  // Generate 8 test cases spanning zero, normals, subnormals.
  uint16_t test_cases[8] = {
      0x0000,  // +0.0
      0x8000,  // -0.0
      0x3f80,  // 1.0f
      0xbf80,  // -1.0f
      0x4000,  // 2.0f
      0x0001,  // Smallest subnormal
      0x7f7f,  // Largest finite normal
      0xff7f   // Negative largest finite normal
  };

  for (int element_idx = 0; element_idx < 8; ++element_idx) {
    v_regs_[1]->data_buffer()->Set<uint16_t>(element_idx,
                                             test_cases[element_idx]);
  }

  auto inst = CreateVfwcvtbf16ffvInstruction(/*dest_reg_idx=*/2,
                                             /*num_regs_dst=*/2,
                                             /*src_reg_idx=*/1,
                                             /*num_regs_src=*/1);
  inst->Execute();

  // Validate properties: converting back to BF16 (by shifting down) must yield
  // the exact original bit pattern.
  for (int element_idx = 0; element_idx < 8; ++element_idx) {
    int reg_idx = 2 + (element_idx / 4);
    int offset = element_idx % 4;
    float res = v_regs_[reg_idx]->data_buffer()->Get<float>(offset);
    uint32_t res_bits = absl::bit_cast<uint32_t>(res);
    uint16_t res_bf16 = (res_bits >> 16) & 0xFFFF;
    EXPECT_EQ(res_bf16, test_cases[element_idx]);
  }
}

// Property-Based Testing for Vfncvtbf16ffw.
TEST_F(CoralNPUM3InstructionsTest, PropertyBasedVfncvtbf16ffw) {
  SetupVectorState(/*vtype=*/8, /*vl=*/4);
  rv_fp_->frm()->Write(static_cast<uint32_t>(FPRoundingMode::kRoundToNearest));

  float test_cases[4] = {0.0f, -0.0f, 1.0f, -2.0f};

  for (int element_idx = 0; element_idx < 4; ++element_idx) {
    v_regs_[2]->data_buffer()->Set<float>(element_idx, test_cases[element_idx]);
  }

  auto inst = CreateVfncvtbf16ffwInstruction(/*dest_reg_idx=*/1,
                                             /*num_regs_dst=*/1,
                                             /*src_reg_idx=*/2,
                                             /*num_regs_src=*/2);
  inst->Execute();

  uint16_t expected_patterns[4] = {0x0000, 0x8000, 0x3f80, 0xc000};
  for (int element_idx = 0; element_idx < 4; ++element_idx) {
    uint16_t res = v_regs_[1]->data_buffer()->Get<uint16_t>(element_idx);
    EXPECT_EQ(res, expected_patterns[element_idx]);
  }
}

// Test Null Pointer Dereference vulnerability protection (Vfwcvtbf16ffv).
TEST_F(CoralNPUM3InstructionsTest, Vfwcvtbf16ffv_NullOperandTrap) {
  SetupVectorState(/*vtype=*/9, /*vl=*/8);
  auto inst = std::make_unique<Instruction>(0, state_);
  inst->set_semantic_function(&coralnpu::sim::Vfwcvtbf16ffv);

  inst->Execute();

  EXPECT_TRUE(was_trap_handler_called_);
  EXPECT_EQ(exception_code_, ExceptionCode::kIllegalInstruction);
}

TEST_F(CoralNPUM3InstructionsTest, Vfwcvtbf16ffv_NullDestinationOperandTrap) {
  SetupVectorState(/*vtype=*/9, /*vl=*/8);
  auto inst = std::make_unique<Instruction>(0, state_);
  inst->set_semantic_function(&coralnpu::sim::Vfwcvtbf16ffv);

  std::vector<RegisterBase*> src_regs = {v_regs_[2], v_regs_[3]};
  inst->AppendSource(
      new RV32VectorSourceOperand(absl::MakeSpan(src_regs), "v2"));
  inst->AppendSource(new RV32VectorTrueOperand(state_));

  inst->Execute();

  EXPECT_TRUE(was_trap_handler_called_);
  EXPECT_EQ(exception_code_, ExceptionCode::kIllegalInstruction);
}

TEST_F(CoralNPUM3InstructionsTest, Vfwcvtbf16ffv_NullSourceOperandTrap) {
  SetupVectorState(/*vtype=*/9, /*vl=*/8);
  auto inst = std::make_unique<Instruction>(0, state_);
  inst->set_semantic_function(&coralnpu::sim::Vfwcvtbf16ffv);

  std::vector<RegisterBase*> dest_regs = {v_regs_[4], v_regs_[5]};
  inst->AppendDestination(new RV32VectorDestinationOperand(
      absl::MakeSpan(dest_regs), /*latency=*/0, "v4"));

  inst->Execute();

  EXPECT_TRUE(was_trap_handler_called_);
  EXPECT_EQ(exception_code_, ExceptionCode::kIllegalInstruction);
}

// Test Null Pointer Dereference vulnerability protection (Vfncvtbf16ffw).
TEST_F(CoralNPUM3InstructionsTest, Vfncvtbf16ffw_NullOperandTrap) {
  SetupVectorState(/*vtype=*/8, /*vl=*/4);
  auto inst = std::make_unique<Instruction>(0, state_);
  inst->set_semantic_function(&coralnpu::sim::Vfncvtbf16ffw);

  inst->Execute();

  EXPECT_TRUE(was_trap_handler_called_);
  EXPECT_EQ(exception_code_, ExceptionCode::kIllegalInstruction);
}

TEST_F(CoralNPUM3InstructionsTest, Vfncvtbf16ffw_NullDestinationOperandTrap) {
  SetupVectorState(/*vtype=*/8, /*vl=*/4);
  auto inst = std::make_unique<Instruction>(0, state_);
  inst->set_semantic_function(&coralnpu::sim::Vfncvtbf16ffw);

  std::vector<RegisterBase*> src_regs = {v_regs_[2], v_regs_[3]};
  inst->AppendSource(
      new RV32VectorSourceOperand(absl::MakeSpan(src_regs), "v2"));
  inst->AppendSource(new RV32VectorTrueOperand(state_));

  inst->Execute();

  EXPECT_TRUE(was_trap_handler_called_);
  EXPECT_EQ(exception_code_, ExceptionCode::kIllegalInstruction);
}

TEST_F(CoralNPUM3InstructionsTest, Vfncvtbf16ffw_NullSourceOperandTrap) {
  SetupVectorState(/*vtype=*/8, /*vl=*/4);
  auto inst = std::make_unique<Instruction>(0, state_);
  inst->set_semantic_function(&coralnpu::sim::Vfncvtbf16ffw);

  std::vector<RegisterBase*> dest_regs = {v_regs_[1]};
  inst->AppendDestination(new RV32VectorDestinationOperand(
      absl::MakeSpan(dest_regs), /*latency=*/0, "v1"));

  inst->Execute();

  EXPECT_TRUE(was_trap_handler_called_);
  EXPECT_EQ(exception_code_, ExceptionCode::kIllegalInstruction);
}

TEST_F(CoralNPUM3InstructionsTest, Vfwcvtbf16ffv_VectorDisabledTrap) {
  was_trap_handler_called_ = false;
  SetupVectorState(/*vtype=*/9, /*vl=*/8);

  // Disable vector extension in mstatus.VS (bits 10-9 = 0).
  state_->mstatus()->Set(
      static_cast<uint64_t>(state_->mstatus()->GetUint64() & ~0x600ULL));

  auto inst = CreateVfwcvtbf16ffvInstruction(/*dest_reg_idx=*/4,
                                             /*num_regs_dst=*/4,
                                             /*src_reg_idx=*/2,
                                             /*num_regs_src=*/2);
  inst->Execute();

  EXPECT_TRUE(was_trap_handler_called_);
  EXPECT_EQ(exception_code_, ExceptionCode::kIllegalInstruction);
}

TEST_F(CoralNPUM3InstructionsTest, Vfncvtbf16ffw_VectorDisabledTrap) {
  was_trap_handler_called_ = false;
  SetupVectorState(/*vtype=*/8, /*vl=*/4);

  // Disable vector extension in mstatus.VS (bits 10-9 = 0).
  state_->mstatus()->Set(
      static_cast<uint64_t>(state_->mstatus()->GetUint64() & ~0x600ULL));

  auto inst = CreateVfncvtbf16ffwInstruction(/*dest_reg_idx=*/1,
                                             /*num_regs_dst=*/1,
                                             /*src_reg_idx=*/2,
                                             /*num_regs_src=*/2);
  inst->Execute();

  EXPECT_TRUE(was_trap_handler_called_);
  EXPECT_EQ(exception_code_, ExceptionCode::kIllegalInstruction);
}

// Unit test for masked Vfwcvtbf16ffv with a non-trivial mask.
TEST_F(CoralNPUM3InstructionsTest, Vfwcvtbf16ffv_Masked) {
  SetupVectorState(/*vtype=*/8, /*vl=*/4);

  // Initialize v1 (source) with BF16 values: 1.0f, 2.0f, 3.0f, 4.0f.
  uint16_t src_bf16[] = {0x3f80, 0x4000, 0x4040, 0x4080};
  for (int i = 0; i < 4; ++i) {
    v_regs_[1]->data_buffer()->Set<uint16_t>(i, src_bf16[i]);
  }

  // Initialize v2 (destination) with 0.0f.
  for (int i = 0; i < 4; ++i) {
    v_regs_[2]->data_buffer()->Set<float>(i, 0.0f);
  }

  // Set mask v0: 0b1010 (elements 1 and 3 active).
  auto mask_db = v_regs_[0]->data_buffer();
  mask_db->Set<uint8_t>(0, 0x0a);

  auto mask_op = std::make_unique<RV32VectorSourceOperand>(v_regs_[0], "v0");
  auto inst =
      CreateVfwcvtbf16ffvInstruction(/*dest_reg_idx=*/2,
                                     /*num_regs_dst=*/2,
                                     /*src_reg_idx=*/1,
                                     /*num_regs_src=*/1, mask_op.release());
  inst->Execute();

  // Verify v2:
  // Element 0: Masked out, remains 0.0f.
  // Element 1: Active, becomes 2.0f.
  // Element 2: Masked out, remains 0.0f.
  // Element 3: Active, becomes 4.0f.
  EXPECT_EQ(v_regs_[2]->data_buffer()->Get<float>(0), 0.0f);
  EXPECT_EQ(v_regs_[2]->data_buffer()->Get<float>(1), 2.0f);
  EXPECT_EQ(v_regs_[2]->data_buffer()->Get<float>(2), 0.0f);
  EXPECT_EQ(v_regs_[2]->data_buffer()->Get<float>(3), 4.0f);
}

// Unit test for masked Vfncvtbf16ffw with a non-trivial mask.
TEST_F(CoralNPUM3InstructionsTest, Vfncvtbf16ffw_Masked) {
  SetupVectorState(/*vtype=*/8, /*vl=*/4);

  // Initialize v2 (source) with FP32 values: 1.0f, 2.0f, 3.0f, 4.0f.
  float src_f32[] = {1.0f, 2.0f, 3.0f, 4.0f};
  for (int i = 0; i < 4; ++i) {
    v_regs_[2]->data_buffer()->Set<float>(i, src_f32[i]);
  }

  // Initialize v1 (destination) with 4.0f BF16 (0x4080).
  for (int i = 0; i < 4; ++i) {
    v_regs_[1]->data_buffer()->Set<uint16_t>(i, 0x4080);
  }

  // Set mask v0: 0b0101 (elements 0 and 2 active).
  auto mask_db = v_regs_[0]->data_buffer();
  mask_db->Set<uint8_t>(0, 0x05);

  auto mask_op = std::make_unique<RV32VectorSourceOperand>(v_regs_[0], "v0");
  auto inst =
      CreateVfncvtbf16ffwInstruction(/*dest_reg_idx=*/1,
                                     /*num_regs_dst=*/1,
                                     /*src_reg_idx=*/2,
                                     /*num_regs_src=*/2, mask_op.release());
  inst->Execute();

  // Verify v1:
  // Element 0: Active, becomes 1.0f BF16 (0x3f80).
  // Element 1: Masked out, remains 4.0f BF16 (0x4080).
  // Element 2: Active, becomes 3.0f BF16 (0x4040).
  // Element 3: Masked out, remains 4.0f BF16 (0x4080).
  EXPECT_EQ(v_regs_[1]->data_buffer()->Get<uint16_t>(0), 0x3f80);
  EXPECT_EQ(v_regs_[1]->data_buffer()->Get<uint16_t>(1), 0x4080);
  EXPECT_EQ(v_regs_[1]->data_buffer()->Get<uint16_t>(2), 0x4040);
  EXPECT_EQ(v_regs_[1]->data_buffer()->Get<uint16_t>(3), 0x4080);
}

// Correct SEW=32 vs SEW=16 discrepancy in Vfncvtbf16ffw test assertions.
TEST_F(CoralNPUM3InstructionsTest, Vfncvtbf16ffw_SEWCorrectness) {
  // vtype = 8 (SEW=16, LMUL=1, which means EMUL=2 for narrowing source)
  SetupVectorState(/*vtype=*/8, /*vl=*/2);
  float src_f32[] = {1.0f, 2.0f};
  v_regs_[2]->data_buffer()->Set<float>(0, src_f32[0]);
  v_regs_[2]->data_buffer()->Set<float>(1, src_f32[1]);
  auto inst = CreateVfncvtbf16ffwInstruction(/*dest_reg_idx=*/1,
                                             /*num_regs_dst=*/1,
                                             /*src_reg_idx=*/2,
                                             /*num_regs_src=*/2);
  inst->Execute();
  EXPECT_EQ(v_regs_[1]->data_buffer()->Get<uint16_t>(0), 0x3f80);
  EXPECT_EQ(v_regs_[1]->data_buffer()->Get<uint16_t>(1), 0x4000);
}

// Scalar BFloat16 conversion unit tests.
TEST_F(CoralNPUM3InstructionsTest, FcvtBf16S_Exact) {
  // Set rounding mode to RNE (0).
  state_->rv_fp()->fflags()->Set(0U);

  auto [frs1_reg, s_src] = state_->GetRegister<RVFpRegister>("f1");
  auto [frd_reg, s_dst] = state_->GetRegister<RVFpRegister>("f2");

  frs1_reg->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFF3F800000ULL);

  auto inst = CreateFcvtBf16SInstruction("f2", "f1", 0);
  inst->Execute();

  // 1.0f in BF16 is 0x3f80. The output should be NaN-boxed in 64 bits
  // (0xFFFFFFFFFFFF3F80).
  EXPECT_EQ(frd_reg->data_buffer()->Get<uint64_t>(0), 0xFFFFFFFFFFFF3F80ULL);
  EXPECT_EQ(state_->rv_fp()->fflags()->GetUint32(), 0);
}

TEST_F(CoralNPUM3InstructionsTest, FcvtBf16S_InvalidRoundingModeTrap) {
  // Try static rounding mode 5, which is invalid.
  auto inst = CreateFcvtBf16SInstruction("f2", "f1", 5);
  inst->Execute();

  EXPECT_TRUE(was_trap_handler_called_);
  EXPECT_EQ(exception_code_, ExceptionCode::kIllegalInstruction);
}

TEST_F(CoralNPUM3InstructionsTest, FcvtBf16S_LossyConversion) {
  state_->rv_fp()->fflags()->Set(0U);

  auto [frs1_reg, s_src] = state_->GetRegister<RVFpRegister>("f1");
  auto [frd_reg, s_dst] = state_->GetRegister<RVFpRegister>("f2");

  // A value that cannot be represented exactly in BF16 (e.g. 1.0001f ->
  // 0x3f800d1b).
  frs1_reg->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFF3F800D1BULL);

  auto inst = CreateFcvtBf16SInstruction("f2", "f1", 0);
  inst->Execute();

  EXPECT_EQ(frd_reg->data_buffer()->Get<uint64_t>(0), 0xFFFFFFFFFFFF3F80ULL);
  EXPECT_EQ(state_->rv_fp()->fflags()->GetUint32(),
            static_cast<uint32_t>(FPExceptions::kInexact));
}

TEST_F(CoralNPUM3InstructionsTest, FcvtBf16S_NaNSHandling) {
  state_->rv_fp()->fflags()->Set(0U);

  auto [frs1_reg, s_src] = state_->GetRegister<RVFpRegister>("f1");
  auto [frd_reg, s_dst] = state_->GetRegister<RVFpRegister>("f2");

  // Signaling NaN (0x7F800001)
  frs1_reg->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFF7F800001ULL);

  auto inst = CreateFcvtBf16SInstruction("f2", "f1", 0);
  inst->Execute();

  // Signaling NaN triggers InvalidOp exception and results in canonical quiet
  // NaN (0x7FC0 boxed to 0xFFFFFFFFFFFF7FC0).
  EXPECT_EQ(frd_reg->data_buffer()->Get<uint64_t>(0), 0xFFFFFFFFFFFF7FC0ULL);
  EXPECT_EQ(state_->rv_fp()->fflags()->GetUint32(),
            static_cast<uint32_t>(FPExceptions::kInvalidOp));
}

TEST_F(CoralNPUM3InstructionsTest, FcvtSBf16_Exact) {
  state_->rv_fp()->fflags()->Set(0U);

  auto [frs1_reg, s_src] = state_->GetRegister<RVFpRegister>("f1");
  auto [frd_reg, s_dst] = state_->GetRegister<RVFpRegister>("f2");

  // Properly NaN-boxed 1.0f BF16 (0xFFFFFFFFFFFF3F80)
  frs1_reg->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFFFFFF3F80ULL);

  auto inst = CreateFcvtSBf16Instruction("f2", "f1");
  inst->Execute();

  EXPECT_EQ(frd_reg->data_buffer()->Get<uint64_t>(0), 0xFFFFFFFF3F800000ULL);
  EXPECT_EQ(state_->rv_fp()->fflags()->GetUint32(), 0);
}

TEST_F(CoralNPUM3InstructionsTest, FcvtSBf16_ImproperlyBoxed) {
  state_->rv_fp()->fflags()->Set(0U);

  auto [frs1_reg, s_src] = state_->GetRegister<RVFpRegister>("f1");
  auto [frd_reg, s_dst] = state_->GetRegister<RVFpRegister>("f2");

  // Improperly NaN-boxed BF16 (e.g. upper bits are 0x0000 instead of all 1s)
  frs1_reg->data_buffer()->Set<uint64_t>(0, 0x0000000000003F80ULL);

  auto inst = CreateFcvtSBf16Instruction("f2", "f1");
  inst->Execute();

  // Treated as canonical Quiet NaN (0xFFFFFFFF7FC00000)
  EXPECT_EQ(frd_reg->data_buffer()->Get<uint64_t>(0), 0xFFFFFFFF7FC00000ULL);
  EXPECT_EQ(state_->rv_fp()->fflags()->GetUint32(), 0);
}

TEST_F(CoralNPUM3InstructionsTest, FcvtSBf16_NaNSHandling) {
  state_->rv_fp()->fflags()->Set(0U);

  auto [frs1_reg, s_src] = state_->GetRegister<RVFpRegister>("f1");
  auto [frd_reg, s_dst] = state_->GetRegister<RVFpRegister>("f2");

  // Signaling NaN (0x7F81) properly boxed (0xFFFFFFFFFFFF7F81)
  frs1_reg->data_buffer()->Set<uint64_t>(0, 0xFFFFFFFFFFFF7F81ULL);

  auto inst = CreateFcvtSBf16Instruction("f2", "f1");
  inst->Execute();

  // Triggers InvalidOp and propagates canonical quiet NaN (0xFFFFFFFF7FC00000).
  EXPECT_EQ(frd_reg->data_buffer()->Get<uint64_t>(0), 0xFFFFFFFF7FC00000ULL);
  EXPECT_EQ(state_->rv_fp()->fflags()->GetUint32(),
            static_cast<uint32_t>(FPExceptions::kInvalidOp));
}

TEST_F(CoralNPUM3InstructionsTest, FcvtSBf16_InvalidRoundingModeTrap) {
  // Try rounding mode 1 (RTZ), which is invalid for widening conversion (must
  // be 0/RNE).
  auto inst = CreateFcvtSBf16Instruction("f2", "f1", 1);
  inst->Execute();

  EXPECT_TRUE(was_trap_handler_called_);
  EXPECT_EQ(exception_code_, ExceptionCode::kIllegalInstruction);
}

}  // namespace
