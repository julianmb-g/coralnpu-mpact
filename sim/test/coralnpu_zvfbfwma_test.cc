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

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "sim/coralnpu_architecture.h"
#include "sim/coralnpu_simulator.h"
#include "sim/coralnpu_v2_state.h"
#include "googletest/include/gtest/gtest.h"
#include "riscv/riscv_register.h"
#include "riscv/riscv_vector_state.h"
#include "mpact/sim/generic/data_buffer.h"
#include "mpact/sim/generic/decoder_interface.h"
#include "mpact/sim/generic/instruction.h"
#include "mpact/sim/util/memory/memory_interface.h"

namespace coralnpu::sim::test {
namespace {

using ::mpact::sim::generic::DataBuffer;
using ::mpact::sim::generic::Instruction;

constexpr uint32_t kMemoryStart = 0x0;
constexpr uint32_t kMemorySize = 0x1000;

class CoralNPUZvfbfwmaTest : public ::testing::Test {
 public:
  void SetUp() override {
    CoralNPUSimulatorOptions options;
    options.architecture = Architecture::kM3;
    options.memory_regions.push_back(
        {kMemoryStart, kMemorySize, MemoryPermission::kReadExecute});
    simulator_ = std::make_unique<CoralNPUSimulator>(options);

    state_ = simulator_->state();
    memory_ = simulator_->memory();
    decoder_ = simulator_->decoder();
    rv_vector_state_ = state_->rv_vector();

    // Set strictly LMUL=1 and SEW=16 (Vector type index 8)
    rv_vector_state_->SetVectorType(8);
    int vd_size = state_->vector_register_width() / sizeof(uint16_t);
    rv_vector_state_->set_vector_length(vd_size);
    rv_vector_state_->clear_vector_exception();
  }

  Instruction* Decode(uint32_t instruction_word, uint64_t address = 0) {
    DataBuffer* inst_db = state_->db_factory()->Allocate<uint32_t>(1);
    inst_db->Set<uint32_t>(0, instruction_word);
    memory_->Store(address, inst_db);
    Instruction* instruction = decoder_->DecodeInstruction(address);
    inst_db->DecRef();
    return instruction;
  }

  template <typename T>
  void SetVectorRegister(const std::string& reg_name,
                         const std::vector<T>& values) {
    auto* vreg =
        state_->GetRegister<::mpact::sim::riscv::RVVectorRegister>(reg_name)
            .first;
    auto* db = state_->db_factory()->Allocate<T>(values.size());
    db->Set(absl::MakeConstSpan(values));
    vreg->SetDataBuffer(db);
    db->DecRef();
  }

  template <typename T>
  std::vector<T> GetVectorRegister(const std::string& reg_name) {
    auto* vreg =
        state_->GetRegister<::mpact::sim::riscv::RVVectorRegister>(reg_name)
            .first;
    auto* db = vreg->data_buffer();
    auto span = db->Get<T>();
    return std::vector<T>(span.begin(), span.end());
  }

  void SetFloatRegister(const std::string& reg_name, float value) {
    auto* reg =
        state_->GetRegister<::mpact::sim::riscv::RVFpRegister>(reg_name).first;
    auto* db = state_->db_factory()->Allocate<uint64_t>(1);
    uint32_t u32;
    std::memcpy(&u32, &value, sizeof(float));
    uint16_t bf16 = u32 >> 16;
    uint64_t u64 = 0xFFFFFFFFFFFF0000ULL | bf16;
    db->Set<uint64_t>(0, u64);
    reg->SetDataBuffer(db);
    db->DecRef();
  }

  void ClearFflags() {
    auto fcsr =
        state_->GetRegister<::mpact::sim::riscv::RV32Register>("fcsr").first;
    auto* db = state_->db_factory()->Allocate<uint32_t>(1);
    db->Set<uint32_t>(0, 0);
    fcsr->SetDataBuffer(db);
    db->DecRef();
  }

  uint32_t GetFflags() {
    auto fcsr =
        state_->GetRegister<::mpact::sim::riscv::RV32Register>("fcsr").first;
    return fcsr->data_buffer()->Get<uint32_t>(0) & 0x1F;
  }

  void AdvanceDelayLines() {
    for (int i = 0; i < 5; ++i) {
      state_->AdvanceDelayLines();
    }
  }

 protected:
  std::unique_ptr<CoralNPUSimulator> simulator_;
  CoralNPUV2State* state_;
  ::mpact::sim::riscv::RiscVVectorState* rv_vector_state_;
  ::mpact::sim::util::MemoryInterface* memory_;
  ::mpact::sim::generic::DecoderInterface* decoder_;
};

TEST_F(CoralNPUZvfbfwmaTest, Vfwmaccvv) {
  ClearFflags();

  int num_elts = state_->vector_register_width() / sizeof(float);
  int num_wide_elts = num_elts * 2;
  std::vector<float> vd_init(num_wide_elts, 10.0f);
  std::vector<uint16_t> vs2_init(num_wide_elts, 0);
  std::vector<uint16_t> vs1_init(num_wide_elts, 0);

  for (int i = 0; i < num_wide_elts; ++i) {
    float vs2_val = 2.0f;
    uint32_t vs2_u32;
    std::memcpy(&vs2_u32, &vs2_val, sizeof(float));
    vs2_init[i] = vs2_u32 >> 16;

    float vs1_val = 3.0f;
    uint32_t vs1_u32;
    std::memcpy(&vs1_u32, &vs1_val, sizeof(float));
    vs1_init[i] = vs1_u32 >> 16;
  }

  std::vector<float> vd_init_v2(vd_init.begin(), vd_init.begin() + num_elts);
  std::vector<float> vd_init_v3(vd_init.begin() + num_elts, vd_init.end());

  SetVectorRegister("v2", vd_init_v2);
  SetVectorRegister("v3", vd_init_v3);
  SetVectorRegister("v4", vs2_init);
  SetVectorRegister("v6", vs1_init);

  auto* inst = Decode(0xEE431157);
  ASSERT_NE(inst, nullptr);
  inst->Execute();
  AdvanceDelayLines();
  inst->DecRef();

  auto vd_result_v2 = GetVectorRegister<float>("v2");
  auto vd_result_v3 = GetVectorRegister<float>("v3");
  for (int i = 0; i < num_elts; ++i) {
    EXPECT_FLOAT_EQ(vd_result_v2[i], 16.0f);
    EXPECT_FLOAT_EQ(vd_result_v3[i], 16.0f);
  }
  EXPECT_EQ(GetFflags(), 0);
}

TEST_F(CoralNPUZvfbfwmaTest, Vfwmaccvf) {
  ClearFflags();

  int num_elts = state_->vector_register_width() / sizeof(float);
  int num_wide_elts = num_elts * 2;
  std::vector<float> vd_init(num_wide_elts, 5.0f);
  std::vector<uint16_t> vs2_init(num_wide_elts, 0);

  for (int i = 0; i < num_wide_elts; ++i) {
    float vs2_val = 4.0f;
    uint32_t vs2_u32;
    std::memcpy(&vs2_u32, &vs2_val, sizeof(float));
    vs2_init[i] = vs2_u32 >> 16;
  }

  std::vector<float> vd_init_v2(vd_init.begin(), vd_init.begin() + num_elts);
  std::vector<float> vd_init_v3(vd_init.begin() + num_elts, vd_init.end());

  SetVectorRegister("v2", vd_init_v2);
  SetVectorRegister("v3", vd_init_v3);
  SetVectorRegister("v4", vs2_init);

  SetFloatRegister("f1", 2.0f);

  auto* inst = Decode(0xEE40D157);
  ASSERT_NE(inst, nullptr);
  inst->Execute();
  AdvanceDelayLines();
  inst->DecRef();

  auto vd_result_v2 = GetVectorRegister<float>("v2");
  auto vd_result_v3 = GetVectorRegister<float>("v3");
  for (int i = 0; i < num_elts; ++i) {
    EXPECT_FLOAT_EQ(vd_result_v2[i], 13.0f);
    EXPECT_FLOAT_EQ(vd_result_v3[i], 13.0f);
  }
  EXPECT_EQ(GetFflags(), 0);
}

TEST_F(CoralNPUZvfbfwmaTest, VfwmaccvfInvalidNaNBoxed) {
  ClearFflags();

  int num_elts = state_->vector_register_width() / sizeof(float);
  int num_wide_elts = num_elts * 2;
  std::vector<float> vd_init(num_wide_elts, 5.0f);
  std::vector<uint16_t> vs2_init(num_wide_elts, 0);

  for (int i = 0; i < num_wide_elts; ++i) {
    float vs2_val = 4.0f;
    uint32_t vs2_u32;
    std::memcpy(&vs2_u32, &vs2_val, sizeof(float));
    vs2_init[i] = vs2_u32 >> 16;
  }

  std::vector<float> vd_init_v2(vd_init.begin(), vd_init.begin() + num_elts);
  std::vector<float> vd_init_v3(vd_init.begin() + num_elts, vd_init.end());

  SetVectorRegister("v2", vd_init_v2);
  SetVectorRegister("v3", vd_init_v3);
  SetVectorRegister("v4", vs2_init);

  auto* reg =
      state_->GetRegister<::mpact::sim::riscv::RVFpRegister>("f1").first;
  auto* db = state_->db_factory()->Allocate<uint64_t>(1);
  db->Set<uint64_t>(0, 0x12344000);  // Invalid NaN-boxing
  reg->SetDataBuffer(db);
  db->DecRef();

  auto* inst = Decode(0xEE40D157);
  ASSERT_NE(inst, nullptr);
  inst->Execute();
  AdvanceDelayLines();
  inst->DecRef();

  auto vd_result_v2 = GetVectorRegister<uint32_t>("v2");
  auto vd_result_v3 = GetVectorRegister<uint32_t>("v3");
  for (int i = 0; i < num_elts; ++i) {
    EXPECT_EQ(vd_result_v2[i], 0x7FC00000);
    EXPECT_EQ(vd_result_v3[i], 0x7FC00000);
  }
  // Invalid NaN-boxing must NOT set the NV flag (0x10)
  EXPECT_EQ(GetFflags() & 0x10, 0);
}

TEST_F(CoralNPUZvfbfwmaTest, VfwmaccAlignmentTrap) {
  ClearFflags();

  auto* inst = Decode(0xEE4310D7);
  ASSERT_NE(inst, nullptr);
  inst->Execute();
  AdvanceDelayLines();
  inst->DecRef();

  EXPECT_TRUE(rv_vector_state_->vector_exception());
}

TEST_F(CoralNPUZvfbfwmaTest, VfwmaccvvIllegalOverlapVs1) {
  ClearFflags();
  // vfwmaccbf16.vv v2, v4, v2 (vs1 == vd, lowest-part overlap, illegal)
  // Hex: 0xEE411157
  auto* inst = Decode(0xEE411157);
  ASSERT_NE(inst, nullptr);
  inst->Execute();
  AdvanceDelayLines();
  inst->DecRef();
  EXPECT_TRUE(rv_vector_state_->vector_exception());
}

TEST_F(CoralNPUZvfbfwmaTest, VfwmaccvvLegalOverlapVs1_Highest) {
  ClearFflags();
  // vfwmaccbf16.vv v2, v4, v3 (vs1 == vd + 1, highest-part overlap, legal)
  // Hex: 0xEE419157
  auto* inst = Decode(0xEE419157);
  ASSERT_NE(inst, nullptr);
  inst->Execute();
  AdvanceDelayLines();
  inst->DecRef();
  EXPECT_FALSE(rv_vector_state_->vector_exception());
}

TEST_F(CoralNPUZvfbfwmaTest, VfwmaccvvIllegalOverlapVs2_Eq) {
  ClearFflags();
  // vfwmaccbf16.vv v2, v2, v6 (vs2 == vd, lowest-part overlap, illegal)
  // Hex: 0xEE231157
  auto* inst = Decode(0xEE231157);
  ASSERT_NE(inst, nullptr);
  inst->Execute();
  AdvanceDelayLines();
  inst->DecRef();
  EXPECT_TRUE(rv_vector_state_->vector_exception());
}

TEST_F(CoralNPUZvfbfwmaTest, VfwmaccvvLegalOverlapVs2_Highest) {
  ClearFflags();
  // vfwmaccbf16.vv v2, v3, v6 (vs2 == vd + 1, highest-part overlap, legal)
  // Hex: 0xEE331157
  auto* inst = Decode(0xEE331157);
  ASSERT_NE(inst, nullptr);
  inst->Execute();
  AdvanceDelayLines();
  inst->DecRef();
  EXPECT_FALSE(rv_vector_state_->vector_exception());
}

TEST_F(CoralNPUZvfbfwmaTest, VfwmaccvfIllegalOverlapVs2_Eq) {
  ClearFflags();
  // vfwmaccbf16.vf v2, v2, f1 (vs2 == vd, lowest-part overlap, illegal)
  // Hex: 0xEE20D157
  auto* inst = Decode(0xEE20D157);
  ASSERT_NE(inst, nullptr);
  inst->Execute();
  AdvanceDelayLines();
  inst->DecRef();
  EXPECT_TRUE(rv_vector_state_->vector_exception());
}

TEST_F(CoralNPUZvfbfwmaTest, VfwmaccvfLegalOverlapVs2_Highest) {
  ClearFflags();
  // vfwmaccbf16.vf v2, v3, f1 (vs2 == vd + 1, highest-part overlap, legal)
  // Hex: 0xEE30D157
  auto* inst = Decode(0xEE30D157);
  ASSERT_NE(inst, nullptr);
  inst->Execute();
  AdvanceDelayLines();
  inst->DecRef();
  EXPECT_FALSE(rv_vector_state_->vector_exception());
}

// Task 322.2: Update Vfwcvtbf16Overlap test to expect success (no exception)
// for legal configurations like vd == vs2 + 1
TEST_F(CoralNPUZvfbfwmaTest, Vfwcvtbf16LegalOverlap) {
  state_->mstatus()->set_fs(1);
  state_->mstatus()->Submit();
  ClearFflags();
  // vfwcvtbf16.f.f.v v2, v1, v0 (vd == vs2 + 1)
  // Encoding: func6=0b010010 (0x12), vs2=0b00001 (0x1), vs1=0b01101 (0x0D),
  // func3=0b001 (0x1), vd=0b00010 (0x2), opcode=0b1010111 (0x57) Hex:
  // 0x4A169157
  auto* inst = Decode(0x4A169157);
  ASSERT_NE(inst, nullptr);
  inst->Execute();
  AdvanceDelayLines();
  inst->DecRef();

  // Expect success (no exception) for legal overlap (vd == vs2 + 1)
  EXPECT_FALSE(rv_vector_state_->vector_exception());
}

TEST_F(CoralNPUZvfbfwmaTest, Vfwcvtbf16IllegalOverlap) {
  state_->mstatus()->set_fs(1);
  state_->mstatus()->Submit();
  ClearFflags();
  // vfwcvtbf16.f.f.v v2, v3, v0 (vd == vs2 - 1)
  // Encoding: func6=0b010010 (0x12), vs2=0b00011 (0x3), vs1=0b01101 (0x0D),
  // func3=0b001 (0x1), vd=0b00010 (0x2), opcode=0b1010111 (0x57) Hex:
  // 0x48369157
  auto* inst = Decode(0x48369157);
  ASSERT_NE(inst, nullptr);
  inst->Execute();
  AdvanceDelayLines();
  inst->DecRef();

  // Expect success (no exception) for legal highest part overlap (vd == vs2 -
  // 1)
  EXPECT_FALSE(rv_vector_state_->vector_exception());
}

TEST_F(CoralNPUZvfbfwmaTest, Vfwcvtbf16IllegalOverlapLower) {
  state_->mstatus()->set_fs(1);
  state_->mstatus()->Submit();
  ClearFflags();
  // vfwcvtbf16.f.f.v v2, v2, v0 (vd == vs2, lowest-part overlap, illegal)
  // Hex: 0x4A269157
  auto* inst = Decode(0x4A269157);
  ASSERT_NE(inst, nullptr);
  inst->Execute();
  AdvanceDelayLines();
  inst->DecRef();
  EXPECT_TRUE(rv_vector_state_->vector_exception());
}

TEST_F(CoralNPUZvfbfwmaTest, Vfwcvtbf16IllegalOverlapUnaligned) {
  state_->mstatus()->set_fs(1);
  state_->mstatus()->Submit();
  ClearFflags();
  // vfwcvtbf16.f.f.v v3, v2, v0 (vd = 3 unaligned, illegal)
  // Hex: 0x4A2691D7
  auto* inst = Decode(0x4A2691D7);
  ASSERT_NE(inst, nullptr);
  inst->Execute();
  AdvanceDelayLines();
  inst->DecRef();
  EXPECT_TRUE(rv_vector_state_->vector_exception());
}

}  // namespace
}  // namespace coralnpu::sim::test
