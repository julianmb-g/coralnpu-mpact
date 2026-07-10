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

#ifndef SIM_ISG_COVERAGE_EVENT_ROUTER_H_
#define SIM_ISG_COVERAGE_EVENT_ROUTER_H_

#include <memory>
#include <vector>

#include "sim/isg/coverage_detector.h"
#include "mpact/sim/generic/instruction.h"

namespace coralnpu {
namespace fuzzer {

// Central router that broadcasts simulation events to attached
// CoverageDetectors.
class CoverageEventRouter {
 public:
  void RegisterDetector(std::unique_ptr<CoverageDetector> detector) {
    detectors_.push_back(std::move(detector));
  }

  void RouteInstruction(const ::mpact::sim::generic::Instruction* inst) {
    if (inst == nullptr) return;
    for (auto& detector : detectors_) {
      detector->OnInstruction(inst);
    }
  }

  void RouteTrap(uint64_t trap_code) {
    for (auto& detector : detectors_) {
      detector->OnTrap(trap_code);
    }
  }

  void RouteRegisterWrite(absl::string_view reg_name, uint64_t value) {
    for (auto& detector : detectors_) {
      detector->OnRegisterWrite(reg_name, value);
    }
  }

 private:
  std::vector<std::unique_ptr<CoverageDetector>> detectors_;
};

}  // namespace fuzzer
}  // namespace coralnpu

#endif  // SIM_ISG_COVERAGE_EVENT_ROUTER_H_
