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

#ifndef SIM_EXECUTION_TRACKER_H_
#define SIM_EXECUTION_TRACKER_H_

#include <cstdint>
#include <string>
#include <vector>

#include "sim/coralnpu_m3_enums.h"
#include "sim/isg/coverage_detector.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"

namespace coralnpu {
namespace fuzzer {

class ExecutionTracker : public CoverageDetector {
 public:
  ExecutionTracker() = default;

  // CoverageDetector interface.
  void OnInstruction(const ::mpact::sim::generic::Instruction* inst) override;
  void OnTrap(uint64_t trap_code) override;
  void OnRegisterWrite(absl::string_view reg_name, uint64_t value) override;
  std::string GetName() const override { return "ExecutionTracker"; }

  // Log the opcode of the instruction.
  void LogOpcode(::coralnpu::sim::isa32_m3::OpcodeEnum opcode);

  // Calculate hazard distances based on instruction.
  void CalculateHazardDistance(const ::mpact::sim::generic::Instruction* inst);

  // Register a mapping from instruction address/PC to its invoking C++
  // generator function.
  void RegisterGeneratorMapping(uint32_t address,
                                const std::string& generator_function);

  // Get the invoking C++ generator function name for a given hazard name.
  absl::StatusOr<std::string> GetGeneratorForHazard(
      const std::string& hazard_name) const;

  // Get the physical trace address (PC) for a given hazard name.
  absl::StatusOr<uint32_t> GetTraceAddressForHazard(
      const std::string& hazard_name) const;

  // Register an expected hazard that should be covered during execution.
  void RegisterExpectedHazard(const std::string& hazard_name);

  // Validate FP poison pills.
  void VerifyFpPoisonPills(const ::mpact::sim::generic::Instruction* inst);

  // Return a list of registered hazards that were not executed.
  std::vector<std::string> GetCoverageGaps() const;

  // Generate an ASCII summary table of coverage gaps.
  std::string GetCoverageGapSummary() const;

  // Generate a JSON summary of coverage.
  std::string GetCoverageJson() const;

  // Generate a LCOV summary of coverage.
  std::string GetCoverageLcov() const;

  // Getter for coverage summary.
  const absl::flat_hash_map<std::string, uint64_t>& coverage_summary() const {
    return coverage_summary_;
  }

 private:
  uint64_t instruction_count_ = 0;
  absl::flat_hash_map<std::string, uint64_t> last_written_cycle_;
  absl::flat_hash_map<std::string, uint64_t> coverage_summary_;
  absl::flat_hash_map<uint32_t, std::string> address_to_generator_;
  absl::flat_hash_map<std::string, std::string> hazard_to_generator_;
  absl::flat_hash_map<std::string, uint32_t> hazard_to_address_;
  absl::flat_hash_set<std::string> expected_hazards_;
};

}  // namespace fuzzer
}  // namespace coralnpu

#endif  // SIM_EXECUTION_TRACKER_H_
