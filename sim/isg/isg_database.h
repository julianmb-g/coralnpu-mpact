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

#ifndef KELVIN_SIM_ISG_ISG_DATABASE_H_
#define KELVIN_SIM_ISG_ISG_DATABASE_H_

#include <string>
#include <vector>

#include "sim/coverage_summary.h"
#include "sim/proto/isg_database.pb.h"
#include "absl/container/flat_hash_set.h"

namespace coralnpu {
namespace fuzzer {

class IsgDatabase {
 public:
  IsgDatabase() = default;

  // Calculates the fitness of a program based on its contribution to
  // global coverage (Set Cover). For the atomic task, we define fitness
  // as the number of new coverpoints this sequence hits.
  uint64_t CalculateFitness(const CoverageSummary& summary) const {
    uint64_t score = 0;
    for (const auto& [name, count] : summary) {
      if (global_coverage_.find(name) == global_coverage_.end()) {
        score++;
      }
    }
    return score;
  }

  // Retains a program if it hits novel coverpoints.
  bool RetainIfNovel(const ::coralnpu::sim::proto::TestSequence& seq,
                     const CoverageSummary& summary) {
    bool novel = false;
    for (const auto& [name, count] : summary) {
      if (global_coverage_.insert(name).second) {
        novel = true;
      }
    }
    if (novel) {
      db_.add_test_sequences()->CopyFrom(seq);
    }
    return novel;
  }

  const ::coralnpu::sim::proto::Database& proto() const { return db_; }

 private:
  ::coralnpu::sim::proto::Database db_;
  absl::flat_hash_set<std::string> global_coverage_;
};

}  // namespace fuzzer
}  // namespace coralnpu

#endif  // KELVIN_SIM_ISG_ISG_DATABASE_H_
