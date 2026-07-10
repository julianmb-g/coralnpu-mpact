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

#include "sim/isg/isg_engine.h"
#include "sim/proto/isg_database.pb.h"
#include "googletest/include/gtest/gtest.h"

namespace coralnpu {
namespace fuzzer {
namespace {

TEST(IsgDatabaseIntegrationTest, SerializeAndDeserializeDatabase) {
  coralnpu::sim::proto::Database out_db;
  out_db.set_master_seed(12345);
  out_db.set_execution_limit(500000);

  IsgEngine engine(42);
  engine.EmitPreamble().EmitMpause();
  auto seq = engine.Build();
  seq.set_expected_terminal_state("mpause");

  coralnpu::sim::proto::TerminalState terminal_state;
  terminal_state.set_pc(0x104);
  terminal_state.set_cycles(150);
  *seq.mutable_terminal_state() = terminal_state;

  *out_db.add_test_sequences() = seq;

  std::string serialized_db;
  EXPECT_TRUE(out_db.SerializeToString(&serialized_db));
  EXPECT_GT(serialized_db.size(), 0);

  coralnpu::sim::proto::Database in_db;
  EXPECT_TRUE(in_db.ParseFromString(serialized_db));

  EXPECT_EQ(in_db.master_seed(), 12345);
  EXPECT_EQ(in_db.execution_limit(), 500000);
  EXPECT_EQ(in_db.test_sequences_size(), 1);

  const auto& in_seq = in_db.test_sequences(0);
  EXPECT_EQ(in_seq.prng_seed(), 42);
  EXPECT_EQ(in_seq.expected_terminal_state(), "mpause");
  EXPECT_EQ(in_seq.terminal_state().pc(), 0x104);
  EXPECT_EQ(in_seq.terminal_state().cycles(), 150);
  EXPECT_FALSE(in_seq.assembly_text().empty());
}

}  // namespace
}  // namespace fuzzer
}  // namespace coralnpu
