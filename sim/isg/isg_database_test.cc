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

#include "sim/isg/isg_database.h"

#include "sim/coverage_summary.h"
#include "sim/proto/isg_database.pb.h"
#include "googletest/include/gtest/gtest.h"

namespace coralnpu {
namespace fuzzer {
namespace {

TEST(IsgDatabaseTest, FitnessCalculation) {
  IsgDatabase db;
  CoverageSummary summary;
  summary["cov1"] = 1;
  summary["cov2"] = 1;

  // Initial fitness should be 2 for 2 new points.
  EXPECT_EQ(db.CalculateFitness(summary), 2);

  // Retain the first sequence.
  ::coralnpu::sim::proto::TestSequence seq1;
  EXPECT_TRUE(db.RetainIfNovel(seq1, summary));

  // Now fitness for the same summary should be 0.
  EXPECT_EQ(db.CalculateFitness(summary), 0);

  // New coverpoint should have fitness 1.
  CoverageSummary summary2;
  summary2["cov1"] = 1;
  summary2["cov3"] = 1;
  EXPECT_EQ(db.CalculateFitness(summary2), 1);
}

TEST(IsgDatabaseTest, UniqueCoverageRetention) {
  IsgDatabase db;

  ::coralnpu::sim::proto::TestSequence seq1;
  seq1.set_assembly_text("seq1");
  CoverageSummary summary1;
  summary1["cov1"] = 1;

  ::coralnpu::sim::proto::TestSequence seq2;
  seq2.set_assembly_text("seq2");
  CoverageSummary summary2;
  summary1["cov1"] = 1;  // redundant

  EXPECT_TRUE(db.RetainIfNovel(seq1, summary1));
  EXPECT_FALSE(db.RetainIfNovel(seq2, summary2));  // cov1 already hit

  EXPECT_EQ(db.proto().test_sequences_size(), 1);
  EXPECT_EQ(db.proto().test_sequences(0).assembly_text(), "seq1");

  ::coralnpu::sim::proto::TestSequence seq3;
  seq3.set_assembly_text("seq3");
  CoverageSummary summary3;
  summary3["cov2"] = 1;

  EXPECT_TRUE(db.RetainIfNovel(seq3, summary3));
  EXPECT_EQ(db.proto().test_sequences_size(), 2);
}

}  // namespace
}  // namespace fuzzer
}  // namespace coralnpu
