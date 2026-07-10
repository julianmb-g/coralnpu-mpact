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

#ifndef SIM_ISG_COVERAGE_DETECTOR_H_
#define SIM_ISG_COVERAGE_DETECTOR_H_

#include <string>

#include "mpact/sim/generic/instruction.h"

namespace coralnpu {
namespace fuzzer {

// Interface for coverage detectors. Detectors are event-driven observers
// that update their state based on simulation events.
class CoverageDetector {
 public:
  virtual ~CoverageDetector() = default;

  // Called when an instruction is decoded or retired.
  virtual void OnInstruction(
      const ::mpact::sim::generic::Instruction* inst) = 0;

  // Called when a trap occurs.
  virtual void OnTrap(uint64_t trap_code) = 0;

  // Called when a register is written.
  virtual void OnRegisterWrite(absl::string_view reg_name, uint64_t value) = 0;

  // Returns the unique name of the coverpoint.
  virtual std::string GetName() const = 0;
};

}  // namespace fuzzer
}  // namespace coralnpu

#endif  // SIM_ISG_COVERAGE_DETECTOR_H_
