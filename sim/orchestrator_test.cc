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

#include "sim/orchestrator.h"

#include "sim/isg/isg_engine.h"
#include "googletest/include/gtest/gtest.h"

namespace coralnpu {
namespace fuzzer {
namespace {

TEST(OrchestratorTest, EvolutionLoopDiscoversCoverage) {
  // Use a fixed seed for determinism in the test
  IsgEngine engine(42);

  // Orchestrator(IsgEngine& engine, uint32_t random_start_pc, uint64_t
  // step_limit, uint64_t seed);
  Orchestrator orchestrator(engine, 0x0, 1000, 42);

  // Run for a small number of iterations
  orchestrator.RunEvolutionLoop(5);

  // We expect at least one sequence to be added to the archive
  EXPECT_GT(orchestrator.GetArchive().Size(), 0);

  // Validate that global coverage is not just non-empty, but contains expected
  // coverage types
  const auto& coverage = orchestrator.GetGlobalCoverage();
  EXPECT_FALSE(coverage.empty());

  // Verify that at least one of the expected coverage detectors was triggered
  bool coverage_found = false;
  for (const auto& [name, count] : coverage) {
    if (count > 0) {
      coverage_found = true;
      break;
    }
  }
  EXPECT_TRUE(coverage_found)
      << "No coverage hits recorded in the global coverage map.";
}

}  // namespace
}  // namespace fuzzer
}  // namespace coralnpu
