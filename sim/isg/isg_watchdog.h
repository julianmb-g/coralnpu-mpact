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

#ifndef KELVIN_SIM_ISG_ISG_WATCHDOG_H_
#define KELVIN_SIM_ISG_ISG_WATCHDOG_H_

#include <cstdint>

#include "absl/status/status.h"

namespace coralnpu {
namespace fuzzer {

// A watchdog mechanism to catch infinite loops in the Instruction Stream
// Generator (ISG) engine.
class IsgWatchdog {
 public:
  explicit IsgWatchdog(uint64_t max_iterations);

  // Increments the iteration counter. Returns DeadlineExceededError if the max
  // is exceeded.
  absl::Status Tick();

  // Resets the counter.
  void Reset();

 private:
  uint64_t max_iterations_;
  uint64_t current_iterations_;
};

}  // namespace fuzzer
}  // namespace coralnpu

#endif  // KELVIN_SIM_ISG_ISG_WATCHDOG_H_