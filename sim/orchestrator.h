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

#ifndef SIM_ORCHESTRATOR_H_
#define SIM_ORCHESTRATOR_H_

#include <string>
#include <vector>

#include "sim/behavioral_archive.h"
#include "sim/fuzzer_types.h"
#include "sim/random_simulator.h"

namespace coralnpu {
namespace fuzzer {

struct SearchResult {
  TestSequence seq;
  TestSequence parent;
  bool was_mutated;
};

class Orchestrator {
 public:
  Orchestrator(IsgEngine& engine, uint32_t random_start_pc, uint64_t step_limit,
               uint64_t seed);

  void RunEvolutionLoop(int iterations);

  const BehavioralArchive& GetArchive() const { return archive_; }
  const CoverageSummary& GetGlobalCoverage() const { return global_coverage_; }

 private:
  TestSequence MutateSequence(const TestSequence& parent);
  SearchResult PerformPhase1Search();
  void PerformPhase2Finalize(
      TestSequence& seq,
      const ::coralnpu::sim::proto::TerminalState& terminal_state);

  std::mt19937_64 prng_;
  IsgEngine& engine_;
  uint32_t random_start_pc_;
  uint64_t step_limit_;

  BehavioralArchive archive_;
  CoverageSummary global_coverage_;
};

}  // namespace fuzzer
}  // namespace coralnpu

#endif  // SIM_ORCHESTRATOR_H_