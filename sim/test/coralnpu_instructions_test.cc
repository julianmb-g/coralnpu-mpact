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

#include "sim/coralnpu_instructions.h"

#include <cstdint>
#include <ios>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "sim/coralnpu_m3_user_decoder.h"
#include "sim/coralnpu_v2_state.h"
#include "googletest/include/gtest/gtest.h"
#include "riscv/riscv_fp_info.h"
#include "riscv/riscv_fp_state.h"
#include "riscv/riscv_register.h"
#include "riscv/riscv_state.h"
#include "mpact/sim/generic/instruction.h"
#include "mpact/sim/util/memory/flat_demand_memory.h"

namespace coralnpu::sim::test {
namespace {

using ::coralnpu::sim::CoralNPUM3UserDecoder;
using ::coralnpu::sim::CoralNPUV2State;
using ::mpact::sim::generic::Instruction;
using ::mpact::sim::util::FlatDemandMemory;

class CoralNPUInstructionsTest : public testing::Test {
 protected:
  void SetUp() override {
    memory_ = new FlatDemandMemory(0);
    state_ = new CoralNPUV2State("test", mpact::sim::riscv::RiscVXlen::RV32,
                                 memory_);
    fp_state_ = new mpact::sim::riscv::RiscVFPState(state_->csr_set(), state_);
    fp_state_->SetRoundingMode(
        mpact::sim::riscv::FPRoundingMode::kRoundToNearest);
    state_->set_rv_fp(fp_state_);
    state_->AddMemoryRegion(0, 0x100000,
                            coralnpu::sim::MemoryPermission::kReadWriteExecute);
    decoder_ = new CoralNPUM3UserDecoder(state_, memory_);
  }

  void TearDown() override {
    delete decoder_;
    delete fp_state_;
    delete state_;
    delete memory_;
  }

  std::unique_ptr<Instruction, void (*)(Instruction*)> CreateInstructionFromHex(
      uint32_t inst_word) {
    auto* db = state_->db_factory()->Allocate<uint32_t>(1);
    db->Set<uint32_t>(0, inst_word);
    memory_->Store(next_instruction_address_, db);
    db->DecRef();
    std::unique_ptr<Instruction, void (*)(Instruction*)> inst(
        decoder_->DecodeInstruction(next_instruction_address_),
        [](Instruction* inst) { inst->DecRef(); });
    next_instruction_address_ += 4;
    return inst;
  }

  template <typename T>
  void SetFpRegisterValues(
      const std::vector<std::pair<std::string, T>>& values) {
    for (const auto& pair : values) {
      auto* reg =
          state_->GetRegister<mpact::sim::riscv::RVFpRegister>(pair.first)
              .first;
      auto* db = state_->db_factory()->Allocate<T>(1);
      db->template Set<T>(0, pair.second);
      reg->SetDataBuffer(db);
      db->DecRef();
    }
  }

  void ExecuteAndAdvance(Instruction* inst) {
    inst->Execute(nullptr);
    for (int i = 0; i < 5; ++i) state_->AdvanceDelayLines();
  }

  uint64_t next_instruction_address_ = 0x1000;
  static constexpr uint32_t kInstFcvtSBf16Rm0 = 0x40608053;
  static constexpr uint32_t kInstFcvtBf16SRm0 = 0x44808053;
  static constexpr uint32_t kInstFcvtBf16SRm7 = 0x4480F053;
  FlatDemandMemory* memory_;
  CoralNPUV2State* state_;
  mpact::sim::riscv::RiscVFPState* fp_state_;
  CoralNPUM3UserDecoder* decoder_;
};

TEST_F(CoralNPUInstructionsTest, RoundBFloat16) {
  uint32_t fflags = 0;
  // Test round to nearest, ties to even. (inst_rm = 0)
  // 0x40400000 -> 2.999999... -> 0x4040 (2.5)
  EXPECT_EQ(RoundBFloat16(0x40400000, 0, fp_state_, &fflags), 0x4040);
  EXPECT_EQ(fflags, 0);

  // 0x403F8000 -> 2.499999... -> 0x4040 (2.5)
  fflags = 0;
  EXPECT_EQ(RoundBFloat16(0x403F8000, 0, fp_state_, &fflags), 0x4040);
  EXPECT_EQ(fflags, 1);

  // 0x40408000 -> 2.500000... -> 0x4040 (2.5)
  fflags = 0;
  EXPECT_EQ(RoundBFloat16(0x40408000, 0, fp_state_, &fflags), 0x4040);
  EXPECT_EQ(fflags, 1);

  // 0x40410000 -> 2.515625 -> 0x4041 (2.515625)
  fflags = 0;
  EXPECT_EQ(RoundBFloat16(0x40410000, 0, fp_state_, &fflags), 0x4041);
  EXPECT_EQ(fflags, 0);

  // Test round towards zero. (inst_rm = 1)
  // 0x40408000 -> 2.5 -> 0x4040
  fflags = 0;
  EXPECT_EQ(RoundBFloat16(0x40408000, 1, fp_state_, &fflags), 0x4040);
  EXPECT_EQ(fflags, 1);

  // 0xC0408000 -> -2.5 -> 0xC040
  fflags = 0;
  EXPECT_EQ(RoundBFloat16(0xC0408000, 1, fp_state_, &fflags), 0xC040);
  EXPECT_EQ(fflags, 1);

  // Test round down. (inst_rm = 2)
  // 0x40408000 -> 3.0... -> 0x4040
  fflags = 0;
  EXPECT_EQ(RoundBFloat16(0x40408000, 2, fp_state_, &fflags), 0x4040);
  EXPECT_EQ(fflags, 1);

  // 0xC0408000 -> -3.0... -> 0xC041
  fflags = 0;
  EXPECT_EQ(RoundBFloat16(0xC0408000, 2, fp_state_, &fflags), 0xC041);
  EXPECT_EQ(fflags, 1);

  // Test round up. (inst_rm = 3)
  // 0x40408000 -> 3.0... -> 0x4041
  fflags = 0;
  EXPECT_EQ(RoundBFloat16(0x40408000, 3, fp_state_, &fflags), 0x4041);
  EXPECT_EQ(fflags, 1);

  // 0xC0408000 -> -3.0... -> 0xC040
  fflags = 0;
  EXPECT_EQ(RoundBFloat16(0xC0408000, 3, fp_state_, &fflags), 0xC040);
  EXPECT_EQ(fflags, 1);

  // Test round to nearest, ties to max. (inst_rm = 4)
  // 0x40408000 -> 2.5 -> 0x4041
  fflags = 0;
  EXPECT_EQ(RoundBFloat16(0x40408000, 4, fp_state_, &fflags), 0x4041);
  EXPECT_EQ(fflags, 1);

  // 0xC0408000 -> -2.5 -> 0xC041
  fflags = 0;
  EXPECT_EQ(RoundBFloat16(0xC0408000, 4, fp_state_, &fflags), 0xC041);
  EXPECT_EQ(fflags, 1);
}

TEST_F(CoralNPUInstructionsTest, FcvtSBf16) {
  constexpr uint64_t kSourceValue = 0xFFFFFFFFFFFFABCDULL;
  constexpr uint64_t kExpectedValue = 0xFFFFFFFFABCD0000ULL;

  // fcvt.s.bf16 f0, f1, rm=0
  constexpr uint32_t kInstWord = kInstFcvtSBf16Rm0;
  auto instruction = CreateInstructionFromHex(kInstWord);

  SetFpRegisterValues<uint64_t>({{"f1", kSourceValue}});

  ExecuteAndAdvance(instruction.get());

  auto* frd = state_->GetRegister<mpact::sim::riscv::RVFpRegister>("f0").first;
  auto* frd_db = frd->data_buffer();
  EXPECT_EQ(frd_db->Get<uint64_t>(0), kExpectedValue);
}

TEST_F(CoralNPUInstructionsTest, FcvtSBf16NaNBox) {
  constexpr uint32_t kSourceValue = 0x0000ABCD;
  constexpr uint64_t kExpectedValue = 0xFFFFFFFF7FC00000ULL;

  // fcvt.s.bf16 f0, f1, rm=0
  constexpr uint32_t kInstWord = kInstFcvtSBf16Rm0;
  auto instruction = CreateInstructionFromHex(kInstWord);

  SetFpRegisterValues<uint64_t>({{"f1", kSourceValue}});

  ExecuteAndAdvance(instruction.get());

  auto* frd = state_->GetRegister<mpact::sim::riscv::RVFpRegister>("f0").first;
  auto* frd_db = frd->data_buffer();
  EXPECT_EQ(frd_db->Get<uint64_t>(0), kExpectedValue);
}

TEST_F(CoralNPUInstructionsTest, FcvtSBf16NaNBoxInvalidUpper) {
  constexpr uint32_t kSourceValue = 0x7FFFABCD;
  constexpr uint64_t kExpectedValue = 0xFFFFFFFF7FC00000ULL;

  // fcvt.s.bf16 f0, f1, rm=0
  constexpr uint32_t kInstWord = kInstFcvtSBf16Rm0;
  auto instruction = CreateInstructionFromHex(kInstWord);

  SetFpRegisterValues<uint64_t>({{"f1", kSourceValue}});

  ExecuteAndAdvance(instruction.get());

  auto* frd = state_->GetRegister<mpact::sim::riscv::RVFpRegister>("f0").first;
  auto* frd_db = frd->data_buffer();
  EXPECT_EQ(frd_db->Get<uint64_t>(0), kExpectedValue);
}

TEST_F(CoralNPUInstructionsTest, FcvtSBf16NaNBox64BitInvalidUpper) {
  constexpr uint64_t kSourceValue =
      0x00000000FFFFABCDULL;  // Missing upper 48-bit NaN-boxing
                              // (0xFFFFFFFFFFFF)
  constexpr uint64_t kExpectedValue =
      0xFFFFFFFF7FC00000ULL;  // Expected to produce canonical NaN

  // fcvt.s.bf16 f0, f1, rm=0
  constexpr uint32_t kInstWord = kInstFcvtSBf16Rm0;
  auto instruction = CreateInstructionFromHex(kInstWord);

  SetFpRegisterValues<uint64_t>({{"f1", kSourceValue}});

  ExecuteAndAdvance(instruction.get());

  auto* frd = state_->GetRegister<mpact::sim::riscv::RVFpRegister>("f0").first;
  auto* frd_db = frd->data_buffer();
  EXPECT_EQ(frd_db->Get<uint64_t>(0), kExpectedValue);
}

TEST_F(CoralNPUInstructionsTest, FcvtSBf16SNaN) {
  // 0xFFFF7FA0 is SNaN in BF16, correctly NaN-boxed.
  constexpr uint64_t kSourceValue = 0xFFFFFFFFFFFF7FA0ULL;
  // Expected to be converted to canonical QNaN (0x7FC00000) and signal
  // InvalidOp.
  constexpr uint64_t kExpectedValue = 0xFFFFFFFF7FC00000ULL;

  // fcvt.s.bf16 f0, f1, rm=0
  constexpr uint32_t kInstWord = kInstFcvtSBf16Rm0;
  auto instruction = CreateInstructionFromHex(kInstWord);

  SetFpRegisterValues<uint64_t>({{"f1", kSourceValue}});

  ExecuteAndAdvance(instruction.get());

  auto* frd = state_->GetRegister<mpact::sim::riscv::RVFpRegister>("f0").first;
  auto* frd_db = frd->data_buffer();
  EXPECT_EQ(frd_db->Get<uint64_t>(0), kExpectedValue);

  ASSERT_NE(fp_state_->fflags(), nullptr);
  EXPECT_EQ(fp_state_->fflags()->GetUint32() & 0x10, 0x10);
}

TEST_F(CoralNPUInstructionsTest, FcvtSBf16DisabledExtensionTraps) {
  // fcvt.s.bf16 f0, f1, rm=0
  constexpr uint32_t kInstWord = kInstFcvtSBf16Rm0;
  auto instruction = CreateInstructionFromHex(kInstWord);

  bool trap_called = false;
  state_->set_on_trap([&trap_called](bool, uint64_t, uint64_t code, uint64_t,
                                     const Instruction*) -> bool {
    trap_called = true;
    EXPECT_EQ(static_cast<mpact::sim::riscv::ExceptionCode>(code),
              mpact::sim::riscv::ExceptionCode::kIllegalInstruction);
    return true;
  });

  auto* old_rv_fp = state_->rv_fp();
  state_->set_rv_fp(nullptr);
  ExecuteAndAdvance(instruction.get());
  EXPECT_TRUE(trap_called);
  state_->set_rv_fp(old_rv_fp);
}

TEST_F(CoralNPUInstructionsTest, FcvtSBf16Rm5Traps) {
  // fcvt.s.bf16 f0, f1, rm=5
  constexpr uint32_t kInstWord = kInstFcvtSBf16Rm0 | (5 << 12);
  auto instruction = CreateInstructionFromHex(kInstWord);

  bool trap_called = false;
  state_->set_on_trap([&trap_called](bool, uint64_t, uint64_t code, uint64_t,
                                     const Instruction*) -> bool {
    trap_called = true;
    EXPECT_EQ(static_cast<mpact::sim::riscv::ExceptionCode>(code),
              mpact::sim::riscv::ExceptionCode::kIllegalInstruction);
    return true;
  });

  ExecuteAndAdvance(instruction.get());
  EXPECT_TRUE(trap_called);
}

TEST_F(CoralNPUInstructionsTest, FcvtSBf16Rm6Traps) {
  // fcvt.s.bf16 f0, f1, rm=6
  constexpr uint32_t kInstWord = kInstFcvtSBf16Rm0 | (6 << 12);
  auto instruction = CreateInstructionFromHex(kInstWord);

  bool trap_called = false;
  state_->set_on_trap([&trap_called](bool, uint64_t, uint64_t code, uint64_t,
                                     const Instruction*) -> bool {
    trap_called = true;
    EXPECT_EQ(static_cast<mpact::sim::riscv::ExceptionCode>(code),
              mpact::sim::riscv::ExceptionCode::kIllegalInstruction);
    return true;
  });

  ExecuteAndAdvance(instruction.get());
  EXPECT_TRUE(trap_called);
}

TEST_F(CoralNPUInstructionsTest, FcvtBf16SNormalExecution) {
  // fcvt.bf16.s f0, f1, rm=0
  constexpr uint32_t kInstWord = kInstFcvtBf16SRm0;
  auto instruction = CreateInstructionFromHex(kInstWord);

  SetFpRegisterValues<uint64_t>(
      {{"f1", 0xFFFFFFFF3F800000ULL}});  // 1.0f (valid NaN-box)

  ExecuteAndAdvance(instruction.get());

  auto* frd = state_->GetRegister<mpact::sim::riscv::RVFpRegister>("f0").first;
  auto* frd_db = frd->data_buffer();
  // Correctly returns 0x3F80 NaN-boxed: 0xFFFFFFFFFFFF3F80
  EXPECT_EQ(frd_db->Get<uint64_t>(0), 0xFFFFFFFFFFFF3F80ULL);
}

TEST_F(CoralNPUInstructionsTest, FcvtBf16SInfinity) {
  auto instruction = CreateInstructionFromHex(kInstFcvtBf16SRm7);
  SetFpRegisterValues<uint64_t>({{"f1", 0xFFFFFFFF7F800000ULL}});  // +Infinity
  ExecuteAndAdvance(instruction.get());
  auto* frd = state_->GetRegister<mpact::sim::riscv::RVFpRegister>("f0").first;
  EXPECT_EQ(frd->data_buffer()->Get<uint64_t>(0), 0xFFFFFFFFFFFF7F80ULL);
}

TEST_F(CoralNPUInstructionsTest, FcvtBf16SNaN) {
  auto instruction = CreateInstructionFromHex(kInstFcvtBf16SRm7);
  SetFpRegisterValues<uint64_t>({{"f1", 0xFFFFFFFF7FC00000ULL}});  // NaN
  ExecuteAndAdvance(instruction.get());
  auto* frd = state_->GetRegister<mpact::sim::riscv::RVFpRegister>("f0").first;
  EXPECT_EQ(frd->data_buffer()->Get<uint64_t>(0), 0xFFFFFFFFFFFF7FC0ULL);
}

TEST_F(CoralNPUInstructionsTest, FcvtBf16S64BitInvalidUpper) {
  auto instruction = CreateInstructionFromHex(kInstFcvtBf16SRm7);
  SetFpRegisterValues<uint64_t>({{"f1", 0x00000000ABCD0000ULL}});
  ExecuteAndAdvance(instruction.get());
  auto* frd = state_->GetRegister<mpact::sim::riscv::RVFpRegister>("f0").first;
  EXPECT_EQ(frd->data_buffer()->Get<uint64_t>(0), 0xFFFFFFFFFFFF7FC0ULL);
}

TEST_F(CoralNPUInstructionsTest, FcvtBf16SRoundingModes) {
  struct RoundingTestCase {
    mpact::sim::riscv::FPRoundingMode rm;
    uint64_t src;       // f32 value in NaN-box
    uint64_t expected;  // expected rounded bf16 in NaN-box
  };

  RoundingTestCase test_cases[] = {
      // Positive exact tie: 1.00390625f (0x3F808000)
      {mpact::sim::riscv::FPRoundingMode::kRoundToNearest,
       0xFFFFFFFF3F808000ULL, 0xFFFFFFFFFFFF3F80ULL},
      {mpact::sim::riscv::FPRoundingMode::kRoundTowardsZero,
       0xFFFFFFFF3F808000ULL, 0xFFFFFFFFFFFF3F80ULL},
      {mpact::sim::riscv::FPRoundingMode::kRoundDown, 0xFFFFFFFF3F808000ULL,
       0xFFFFFFFFFFFF3F80ULL},
      {mpact::sim::riscv::FPRoundingMode::kRoundUp, 0xFFFFFFFF3F808000ULL,
       0xFFFFFFFFFFFF3F81ULL},
      {mpact::sim::riscv::FPRoundingMode::kRoundToNearestTiesToMax,
       0xFFFFFFFF3F808000ULL, 0xFFFFFFFFFFFF3F81ULL},

      // Negative exact tie: -1.00390625f (0xBF808000)
      {mpact::sim::riscv::FPRoundingMode::kRoundToNearest,
       0xFFFFFFFFBF808000ULL, 0xFFFFFFFFFFFFBF80ULL},
      {mpact::sim::riscv::FPRoundingMode::kRoundTowardsZero,
       0xFFFFFFFFBF808000ULL, 0xFFFFFFFFFFFFBF80ULL},
      {mpact::sim::riscv::FPRoundingMode::kRoundDown, 0xFFFFFFFFBF808000ULL,
       0xFFFFFFFFFFFFBF81ULL},
      {mpact::sim::riscv::FPRoundingMode::kRoundUp, 0xFFFFFFFFBF808000ULL,
       0xFFFFFFFFFFFFBF80ULL},
      {mpact::sim::riscv::FPRoundingMode::kRoundToNearestTiesToMax,
       0xFFFFFFFFBF808000ULL, 0xFFFFFFFFFFFFBF81ULL},
  };

  for (const auto& tc : test_cases) {
    auto instruction = CreateInstructionFromHex(kInstFcvtBf16SRm7);
    fp_state_->SetRoundingMode(tc.rm);
    SetFpRegisterValues<uint64_t>({{"f1", tc.src}});

    ExecuteAndAdvance(instruction.get());

    auto* frd =
        state_->GetRegister<mpact::sim::riscv::RVFpRegister>("f0").first;
    EXPECT_EQ(frd->data_buffer()->Get<uint64_t>(0), tc.expected)
        << "Failed for rm: " << static_cast<int>(tc.rm) << " src: " << std::hex
        << tc.src;
  }
}

TEST_F(CoralNPUInstructionsTest, FcvtBf16SSignalingNaN) {
  auto instruction = CreateInstructionFromHex(kInstFcvtBf16SRm7);
  // 0xFFFF7F800001 is SNaN in f32, correctly NaN-boxed.
  SetFpRegisterValues<uint64_t>({{"f1", 0xFFFFFFFF7F800001ULL}});
  ExecuteAndAdvance(instruction.get());
  auto* frd = state_->GetRegister<mpact::sim::riscv::RVFpRegister>("f0").first;
  // Expected to be converted to canonical NaN: 0x7FC0 NaN-boxed.
  EXPECT_EQ(frd->data_buffer()->Get<uint64_t>(0), 0xFFFFFFFFFFFF7FC0ULL);
  ASSERT_NE(fp_state_->fflags(), nullptr);
  EXPECT_EQ(fp_state_->fflags()->GetUint32() & 0x10, 0x10);
}

}  // namespace
}  // namespace coralnpu::sim::test
