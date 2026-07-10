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

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

#include "sim/proto/isg_database.pb.h"
#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"
#include "absl/log/log.h"
#include "absl/strings/str_join.h"

namespace coralnpu {
namespace fuzzer {
namespace {

std::string ReadFile(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) return "";
  return std::string((std::istreambuf_iterator<char>(in)),
                     std::istreambuf_iterator<char>());
}

TEST(IsgDatabaseExportTest, DeterminismAndBasicValidation) {
  // Get the fuzzer binary from environment variables.
  const char* srcdir = std::getenv("TEST_SRCDIR");
  const char* workspace = std::getenv("TEST_WORKSPACE");
  ASSERT_NE(srcdir, nullptr) << "TEST_SRCDIR environment variable not set.";
  ASSERT_NE(workspace, nullptr)
      << "TEST_WORKSPACE environment variable not set.";
  std::string fuzzer_bin =
      std::string(srcdir) + "/" + workspace +
      "/"
      "sim/coralnpu_random_sim";

  const char* tmpdir = std::getenv("TEST_TMPDIR");
  ASSERT_NE(tmpdir, nullptr) << "TEST_TMPDIR environment variable not set.";
  std::string output_db1 = std::string(tmpdir) + "/test_database1.pb";
  std::string output_db2 = std::string(tmpdir) + "/test_database2.pb";

  // Run 1
  std::string cmd1 = "\"" + fuzzer_bin +
                     "\" --num_tests=2 --seed=12345 --output_db=\"" +
                     output_db1 + "\"";
  int ret1 = std::system(cmd1.c_str());
  ASSERT_EQ(ret1, 0) << "Fuzzer execution 1 failed: " << cmd1;

  // Run 2 (same seed)
  std::string cmd2 = "\"" + fuzzer_bin +
                     "\" --num_tests=2 --seed=12345 --output_db=\"" +
                     output_db2 + "\"";
  int ret2 = std::system(cmd2.c_str());
  ASSERT_EQ(ret2, 0) << "Fuzzer execution 2 failed: " << cmd2;

  // Assert identical byte-level outputs (Determinism)
  std::string content1 = ReadFile(output_db1);
  std::string content2 = ReadFile(output_db2);
  ASSERT_FALSE(content1.empty());
  if (content1 != content2) {
    coralnpu::sim::proto::Database db1, db2;
    if (db1.ParseFromString(content1) && db2.ParseFromString(content2)) {
      std::vector<uint64_t> seeds1, seeds2;
      for (const auto& seq : db1.test_sequences()) {
        seeds1.push_back(seq.prng_seed());
      }
      for (const auto& seq : db2.test_sequences()) {
        seeds2.push_back(seq.prng_seed());
      }
      std::sort(seeds1.begin(), seeds1.end());
      std::sort(seeds2.begin(), seeds2.end());
      LOG(INFO) << "DB1 seeds (sorted): " << absl::StrJoin(seeds1, ", ");
      LOG(INFO) << "DB2 seeds (sorted): " << absl::StrJoin(seeds2, ", ");
    } else {
      LOG(ERROR) << "Failed to parse databases for debug print.";
    }
  }
  EXPECT_EQ(content1, content2)
      << "Fuzzer outputs are not identical for the same seed.";

  // Parse and validate basic properties
  coralnpu::sim::proto::Database db;
  ASSERT_TRUE(db.ParseFromString(content1))
      << "Failed to parse database proto.";

  EXPECT_EQ(db.master_seed(), 12345);
  EXPECT_EQ(db.test_sequences_size(), 2) << "Expected 2 test sequences.";

  if (db.test_sequences_size() >= 2) {
    const auto& seq0 = db.test_sequences(0);
    EXPECT_GT(seq0.prng_seed(), 0);
    EXPECT_FALSE(seq0.assembly_text().empty());
    EXPECT_GT(seq0.terminal_state().cycles(), 0);
    if (seq0.terminal_state().pc() == 0) {
      std::cerr << "seq0 terminal_state: "
                << seq0.terminal_state().DebugString() << "\n";
      std::cerr << "seq0 expected_terminal_state: "
                << seq0.expected_terminal_state() << "\n";
    }
    EXPECT_GT(seq0.terminal_state().pc(), 0);
    EXPECT_GT(seq0.terminal_state().registers_size(), 0);
    EXPECT_GT(seq0.memory_dump().memory_blobs_size(), 0);
  }
}

}  // namespace
}  // namespace fuzzer
}  // namespace coralnpu
