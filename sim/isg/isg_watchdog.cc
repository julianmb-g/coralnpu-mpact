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

#include "sim/isg/isg_watchdog.h"

#include <cstdint>

#include "absl/status/status.h"
#include "absl/strings/str_format.h"

namespace coralnpu {
namespace fuzzer {

IsgWatchdog::IsgWatchdog(uint64_t max_iterations)
    : max_iterations_(max_iterations), current_iterations_(0) {}

absl::Status IsgWatchdog::Tick() {
  if (++current_iterations_ > max_iterations_) {
    return absl::DeadlineExceededError(
        absl::StrFormat("IsgWatchdog exceeded maximum iteration limit (%u). "
                        "Infinite loop detected in generator.",
                        max_iterations_));
  }
  return absl::OkStatus();
}

void IsgWatchdog::Reset() { current_iterations_ = 0; }

}  // namespace fuzzer
}  // namespace coralnpu