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

#include "sim/behavioral_archive.h"

#include <random>

#include "sim/coverage_summary.h"
#include "sim/proto/isg_database.pb.h"
#include "googletest/include/gtest/gtest.h"

namespace coralnpu {
namespace fuzzer {
namespace {

TEST(BehavioralArchiveTest, ExtractionAndBinning) {
  BehavioralArchive archive;
  CoverageSummary summary;

  // No detectors
  auto d1 = archive.ExtractDescriptor(summary);
  EXPECT_EQ(d1.detector_name, "");

  // Multiple detectors, both empty. Should pick lexicographical first.
  summary["detector_B"] = 2;
  summary["detector_A"] = 4;
  auto d2 = archive.ExtractDescriptor(summary);
  EXPECT_EQ(d2.detector_name, "detector_A");

  // Add detector_A to archive (simulating it is full)
  ::coralnpu::sim::proto::TestSequence seq;
  archive.AddIfNovel({"detector_A"}, seq, summary);

  // Now detector_A is full. Should pick detector_B (next empty).
  auto d3 = archive.ExtractDescriptor(summary);
  EXPECT_EQ(d3.detector_name, "detector_B");

  // Add detector_B to archive
  archive.AddIfNovel({"detector_B"}, seq, summary);

  // Now both are full. Should fallback to highest count.
  auto d4 = archive.ExtractDescriptor(summary);
  EXPECT_EQ(d4.detector_name, "detector_A");
}

TEST(BehavioralArchiveTest, BehavioralBinning) {
  BehavioralArchive archive;
  BehavioralDescriptor desc = {"detector_A"};

  ::coralnpu::sim::proto::TestSequence seq1;
  seq1.set_assembly_text("seq1");
  CoverageSummary summary1;
  summary1["cov1"] = 1;

  // First program in bin
  EXPECT_TRUE(archive.AddIfNovel(desc, seq1, summary1));
  EXPECT_EQ(archive.Size(), 1);

  // Second program in same bin with same fitness
  ::coralnpu::sim::proto::TestSequence seq2;
  seq2.set_assembly_text("seq2");
  EXPECT_FALSE(archive.AddIfNovel(desc, seq2, summary1));
  EXPECT_EQ(archive.Size(), 1);
  EXPECT_EQ(archive.GetGrid().at(desc).assembly_text(), "seq1");

  // Third program in same bin with HIGHER fitness
  summary1["cov2"] = 1;
  EXPECT_TRUE(archive.AddIfNovel(desc, seq2, summary1));
  EXPECT_EQ(archive.Size(), 1);
  EXPECT_EQ(archive.GetGrid().at(desc).assembly_text(), "seq2");
}

TEST(BehavioralArchiveTest, RandomParent) {
  BehavioralArchive archive;
  std::mt19937_64 prng(42);

  // Empty archive returns empty sequence
  EXPECT_EQ(archive.GetRandomParent(prng).assembly_text(), "");

  ::coralnpu::sim::proto::TestSequence seq1;
  seq1.set_assembly_text("seq1");
  archive.AddIfNovel({"c1"}, seq1, {{"c1", 1}});

  ::coralnpu::sim::proto::TestSequence seq2;
  seq2.set_assembly_text("seq2");
  archive.AddIfNovel({"c2"}, seq2, {{"c2", 1}});

  // Selection should be one of them
  std::string result(archive.GetRandomParent(prng).assembly_text());
  EXPECT_TRUE(result == "seq1" || result == "seq2");
}

}  // namespace
}  // namespace fuzzer
}  // namespace coralnpu
