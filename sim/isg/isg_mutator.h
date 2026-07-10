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

#ifndef KELVIN_SIM_ISG_ISG_MUTATOR_H_
#define KELVIN_SIM_ISG_ISG_MUTATOR_H_

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "sim/coralnpu_m3_user_decoder.h"
#include "sim/coralnpu_v2_state.h"
#include "sim/isg/isg_engine.h"
#include "sim/memory_config.h"
#include "sim/proto/isg_database.pb.h"
#include "absl/random/random.h"
#include "absl/status/status.h"
#include "mpact/sim/util/memory/flat_demand_memory.h"

namespace coralnpu {
namespace fuzzer {

// Interface for mutation operators.
class MutationOperator {
 public:
  virtual ~MutationOperator() = default;

  // Mutates the given TestSequence in-place.
  // Preamble and self-checking hash regions must be protected.
  virtual absl::Status Mutate(::coralnpu::sim::proto::TestSequence* seq) = 0;

  // Returns the name of the mutation operator.
  virtual std::string name() const = 0;
};

// Flips exactly 1 random bit in the TestSequence binary.
class RandomBitFlipMutator : public MutationOperator {
 public:
  explicit RandomBitFlipMutator(uint64_t seed)
      : bit_gen_(absl::BitGen(absl::SeedSeq{seed})) {}

  absl::Status Mutate(::coralnpu::sim::proto::TestSequence* seq) override {
    std::string itcm(seq->itcm_binary());
    if (itcm.empty()) return absl::OkStatus();

    size_t bit_index = absl::Uniform(bit_gen_, 0u, itcm.size() * 8);
    size_t byte_index = bit_index / 8;
    size_t bit_in_byte = bit_index % 8;

    itcm[byte_index] ^= (1 << bit_in_byte);
    seq->set_itcm_binary(itcm);
    return absl::OkStatus();
  }

  std::string name() const override { return "RandomBitFlipMutator"; }

 private:
  absl::BitGen bit_gen_;
};

// Crossover between two programs at a random interval.
class CrossoverMutator : public MutationOperator {
 public:
  explicit CrossoverMutator(uint64_t seed)
      : bit_gen_(absl::BitGen(absl::SeedSeq{seed})) {}

  void SetOtherSequence(const ::coralnpu::sim::proto::TestSequence& other) {
    other_seq_ = other;
  }

  absl::Status Mutate(::coralnpu::sim::proto::TestSequence* seq) override {
    std::string itcm1(seq->itcm_binary());
    std::string itcm2(other_seq_.itcm_binary());
    if (itcm1.empty() || itcm2.empty()) return absl::OkStatus();

    size_t min_len = std::min(itcm1.size(), itcm2.size());
    if (min_len < 4) return absl::OkStatus();

    size_t num_inst = min_len / 4;
    size_t crossover_inst = absl::Uniform(bit_gen_, 1u, num_inst);
    size_t crossover_byte = crossover_inst * 4;

    std::string new_itcm =
        itcm1.substr(0, crossover_byte) + itcm2.substr(crossover_byte);
    seq->set_itcm_binary(new_itcm);
    return absl::OkStatus();
  }

  std::string name() const override { return "CrossoverMutator"; }

 private:
  absl::BitGen bit_gen_;
  ::coralnpu::sim::proto::TestSequence other_seq_;
};

// Replaces a random instruction with another valid random instruction.
class RandomInstructionMutator : public MutationOperator {
 public:
  explicit RandomInstructionMutator(IsgEngine* engine) : engine_(engine) {}

  absl::Status Mutate(::coralnpu::sim::proto::TestSequence* seq) override {
    std::string itcm(seq->itcm_binary());
    if (itcm.size() < 4) return absl::OkStatus();

    uint32_t start = seq->mutable_start();
    uint32_t end = seq->mutable_end();

    if (start >= end || end > itcm.size()) {
      return absl::InternalError("Invalid mutation bounds in TestSequence.");
    }

    size_t num_mutable_inst = (end - start) / 4;
    if (num_mutable_inst == 0) return absl::OkStatus();

    size_t target_idx = absl::Uniform(engine_->prng(), 0u, num_mutable_inst);
    size_t target_offset = start + (target_idx * 4);

    uint32_t new_inst_word = 0;
    int attempts = 0;
    bool valid = false;
    while (attempts < 1000000) {
      attempts++;
      new_inst_word = engine_->prng()();
      ::mpact::sim::generic::DataBuffer* db =
          engine_->shared_state()->db_factory()->Allocate<uint32_t>(1);
      db->Set<uint32_t>(0, new_inst_word);
      engine_->shared_memory()->Store(::coralnpu::sim::kDefaultRxRegionStart,
                                      db);

      ::mpact::sim::generic::Instruction* inst =
          engine_->shared_decoder()->DecodeInstruction(
              ::coralnpu::sim::kDefaultRxRegionStart);
      db->DecRef();

      valid = inst->opcode() !=
              static_cast<int>(::coralnpu::sim::isa32_m3::OpcodeEnum::kNone);
      inst->DecRef();

      if (valid) break;
    }

    if (!valid) {
      return absl::InternalError(
          "Failed to generate a valid random instruction after 1000000 "
          "attempts.");
    }

    std::memcpy(&itcm[target_offset], &new_inst_word, 4);
    seq->set_itcm_binary(itcm);
    return absl::OkStatus();
  }

  std::string name() const override { return "RandomInstructionMutator"; }

 private:
  IsgEngine* engine_;
};

}  // namespace fuzzer
}  // namespace coralnpu

#endif  // KELVIN_SIM_ISG_ISG_MUTATOR_H_
