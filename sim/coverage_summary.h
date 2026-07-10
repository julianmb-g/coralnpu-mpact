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

#ifndef SIM_COVERAGE_SUMMARY_H_
#define SIM_COVERAGE_SUMMARY_H_

#include <cstdint>
#include <string>
#include <utility>

#include "absl/container/flat_hash_map.h"

namespace coralnpu {
namespace fuzzer {

struct CoverageSummary : public absl::flat_hash_map<std::string, uint64_t> {
  using absl::flat_hash_map<std::string, uint64_t>::flat_hash_map;

  CoverageSummary() = default;
  CoverageSummary(const CoverageSummary&) = default;
  CoverageSummary(CoverageSummary&&) = default;
  CoverageSummary& operator=(const CoverageSummary&) = default;
  CoverageSummary& operator=(CoverageSummary&&) = default;

  // Constructor from base map
  CoverageSummary(const absl::flat_hash_map<std::string, uint64_t>& other)
      : absl::flat_hash_map<std::string, uint64_t>(other) {}
  CoverageSummary(absl::flat_hash_map<std::string, uint64_t>&& other)
      : absl::flat_hash_map<std::string, uint64_t>(std::move(other)) {}

  // Assignment from base map
  CoverageSummary& operator=(
      const absl::flat_hash_map<std::string, uint64_t>& other) {
    absl::flat_hash_map<std::string, uint64_t>::operator=(other);
    return *this;
  }
  CoverageSummary& operator=(
      absl::flat_hash_map<std::string, uint64_t>&& other) {
    absl::flat_hash_map<std::string, uint64_t>::operator=(std::move(other));
    return *this;
  }
};

}  // namespace fuzzer
}  // namespace coralnpu

#endif  // SIM_COVERAGE_SUMMARY_H_
