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

#include "sim/isg/isg_mutator.h"

#include <cstdint>
#include <cstring>
#include <string>

#include "sim/isg/isg_engine.h"
#include "sim/proto/isg_database.pb.h"
#include "googletest/include/gtest/gtest.h"
#include "mpact/sim/generic/data_buffer.h"

namespace coralnpu {
namespace fuzzer {
namespace {

// Concrete mutator for testing the interface
class TestMutator : public MutationOperator {
 public:
  absl::Status Mutate(::coralnpu::sim::proto::TestSequence* seq) override {
    std::string itcm(seq->itcm_binary());
    if (itcm.size() >= 4) {
      itcm[0] ^= 0xFF;
    }
    seq->set_itcm_binary(itcm);
    return absl::OkStatus();
  }
  std::string name() const override { return "TestMutator"; }
};

TEST(IsgMutatorTest, Interface) {
  TestMutator mutator;
  ::coralnpu::sim::proto::TestSequence seq;
  uint32_t nop = 0x00000013;
  seq.set_itcm_binary(std::string(reinterpret_cast<const char*>(&nop), 4));

  absl::Status status = mutator.Mutate(&seq);
  EXPECT_TRUE(status.ok());
  EXPECT_EQ(mutator.name(), "TestMutator");

  uint32_t mutated_nop;
  std::memcpy(&mutated_nop, seq.itcm_binary().data(), 4);
  EXPECT_NE(mutated_nop, nop);
}

TEST(IsgMutatorTest, RandomBitFlip) {
  RandomBitFlipMutator mutator(12345);
  ::coralnpu::sim::proto::TestSequence seq;
  uint32_t original = 0x12345678;
  seq.set_itcm_binary(std::string(reinterpret_cast<const char*>(&original), 4));

  absl::Status status = mutator.Mutate(&seq);
  EXPECT_TRUE(status.ok());

  uint32_t mutated;
  std::memcpy(&mutated, seq.itcm_binary().data(), 4);
  EXPECT_NE(mutated, original);

  // Verify only 1 bit is flipped
  uint32_t diff = mutated ^ original;
  EXPECT_EQ(__builtin_popcount(diff), 1);
}

TEST(IsgMutatorTest, Crossover) {
  CrossoverMutator mutator(12345);

  ::coralnpu::sim::proto::TestSequence seq1;
  uint32_t insts1[] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
  seq1.set_itcm_binary(std::string(reinterpret_cast<const char*>(insts1), 16));

  ::coralnpu::sim::proto::TestSequence seq2;
  uint32_t insts2[] = {0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC, 0xDDDDDDDD};
  seq2.set_itcm_binary(std::string(reinterpret_cast<const char*>(insts2), 16));

  mutator.SetOtherSequence(seq2);

  absl::Status status = mutator.Mutate(&seq1);
  EXPECT_TRUE(status.ok());

  EXPECT_EQ(seq1.itcm_binary().size(), 16);

  const uint32_t* result_insts =
      reinterpret_cast<const uint32_t*>(seq1.itcm_binary().data());

  // Verify it's a mix of both.
  // Crossover point is at least 1 instruction (4 bytes).
  // So result[0] should be insts1[0].
  EXPECT_EQ(result_insts[0], insts1[0]);

  // At least some instructions from seq2 should be present if crossover
  // happened.
  bool from_seq2 = false;
  for (int i = 1; i < 4; ++i) {
    if (result_insts[i] == insts2[i]) {
      from_seq2 = true;
      break;
    }
  }
  EXPECT_TRUE(from_seq2);
}

TEST(IsgMutatorTest, RandomInstruction) {
  IsgEngine engine(42);
  RandomInstructionMutator mutator(&engine);

  ::coralnpu::sim::proto::TestSequence seq;
  uint32_t original = 0x12345678;
  seq.set_itcm_binary(std::string(reinterpret_cast<const char*>(&original), 4));
  seq.set_mutable_start(0);
  seq.set_mutable_end(4);

  absl::Status status = mutator.Mutate(&seq);
  EXPECT_TRUE(status.ok()) << status.ToString();

  uint32_t mutated;
  std::memcpy(&mutated, seq.itcm_binary().data(), 4);
  EXPECT_NE(mutated, original);

  // Verify the mutated instruction is decodable and valid (not kNone).
  ::mpact::sim::generic::DataBuffer* db_check =
      engine.shared_state()->db_factory()->Allocate<uint32_t>(1);
  db_check->Set<uint32_t>(0, mutated);
  engine.shared_memory()->Store(::coralnpu::sim::kDefaultRxRegionStart,
                                db_check);

  ::mpact::sim::generic::Instruction* decoded_inst =
      engine.shared_decoder()->DecodeInstruction(
          ::coralnpu::sim::kDefaultRxRegionStart);
  db_check->DecRef();

  ASSERT_NE(decoded_inst, nullptr);
  EXPECT_NE(decoded_inst->opcode(),
            static_cast<int>(::coralnpu::sim::isa32_m3::OpcodeEnum::kNone));
  decoded_inst->DecRef();
}

TEST(IsgMutatorTest, RandomInstructionUsesSharedContext) {
  IsgEngine engine(42);
  RandomInstructionMutator mutator(&engine);

  ::coralnpu::sim::proto::TestSequence seq;
  uint32_t original = 0x12345678;
  seq.set_itcm_binary(std::string(reinterpret_cast<const char*>(&original), 4));
  seq.set_mutable_start(0);
  seq.set_mutable_end(4);

  // Pre-fill shared memory with something different
  ::mpact::sim::generic::DataBuffer* db_init =
      engine.shared_state()->db_factory()->Allocate<uint32_t>(1);
  db_init->Set<uint32_t>(0, 0xDEADBEEF);
  engine.shared_memory()->Store(::coralnpu::sim::kDefaultRxRegionStart,
                                db_init);
  db_init->DecRef();

  absl::Status status = mutator.Mutate(&seq);
  EXPECT_TRUE(status.ok()) << status.ToString();

  uint32_t mutated;
  std::memcpy(&mutated, seq.itcm_binary().data(), 4);
  EXPECT_NE(mutated, original);

  // Verify shared memory was updated
  ::mpact::sim::generic::DataBuffer* db_read =
      engine.shared_state()->db_factory()->Allocate<uint32_t>(1);
  engine.shared_memory()->Load(::coralnpu::sim::kDefaultRxRegionStart, db_read,
                               nullptr, nullptr);
  uint32_t loaded = db_read->Get<uint32_t>(0);
  db_read->DecRef();

  EXPECT_EQ(loaded, mutated) << "Shared memory was not updated by mutator!";
}

}  // namespace
}  // namespace fuzzer
}  // namespace coralnpu
