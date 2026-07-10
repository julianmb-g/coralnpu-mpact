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

#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#include "sim/orchestrator.h"
#include "sim/proto/isg_database.pb.h"
#include "sim/random_simulator.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/log.h"
#include "absl/time/clock.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"

ABSL_FLAG(std::string, mode, "default",
          "Mode to run: 'default' or 'blind_refine'");
ABSL_FLAG(int, refinement_iterations, 10,
          "Number of blind refinement iterations");
ABSL_FLAG(bool, log_pc_discontinuity, false,
          "Enable logging of PC discontinuities");
ABSL_FLAG(bool, debug_trace, false,
          "Enable verbose instruction tracing for debugging");
ABSL_FLAG(uint64_t, seed, 0,
          "Deterministic PRNG seed. If 0, a random seed is generated.");
ABSL_FLAG(std::string, output_db, "",
          "Path to the output protobuf database file.");
ABSL_FLAG(int, num_tests, 0,
          "Number of test sequences to generate (archive size target).");
ABSL_FLAG(int, iterations, 20,
          "Number of evolution iterations (used if num_tests is 0).");
ABSL_FLAG(uint64_t, max_instructions, 150000,
          "Execution step limit for each fuzzer run.");

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  uint32_t seed = absl::GetFlag(FLAGS_seed);
  if (seed == 0) {
    seed = absl::ToUnixMicros(absl::Now()) ^ getpid();
  }
  LOG(INFO) << "Using PRNG seed: " << seed;
  ::coralnpu::fuzzer::IsgEngine engine(seed);
  engine.EmitPreamble();
  uint32_t random_start_pc = engine.CurrentPc();

  std::string mode = absl::GetFlag(FLAGS_mode);
  if (mode == "blind_refine") {
    // Blind refine mode was previously here, but currently disabled in wrapper.
    LOG(ERROR) << "blind_refine mode is temporarily disabled in wrapper.";
    return 1;
  } else {
    uint64_t max_instr = absl::GetFlag(FLAGS_max_instructions);
    ::coralnpu::fuzzer::Orchestrator orchestrator(engine, random_start_pc,
                                                  max_instr, seed);

    int num_tests = absl::GetFlag(FLAGS_num_tests);
    int iterations = absl::GetFlag(FLAGS_iterations);
    if (num_tests > 0) {
      // Increase multiplier to 100 to provide sufficient generation budget for
      // reaching the requested unique test count (Finding #183).
      int max_iterations = num_tests * 100;
      int run_iters = 0;
      while (orchestrator.GetArchive().Size() < num_tests &&
             run_iters < max_iterations) {
        orchestrator.RunEvolutionLoop(1);
        run_iters++;
      }
      LOG(INFO) << "Reached archive size: " << orchestrator.GetArchive().Size()
                << " after " << run_iters << " iterations.";
    } else {
      orchestrator.RunEvolutionLoop(iterations);
    }

    LOG(INFO) << "Evolution Loop completed!";
    LOG(INFO) << "Global Coverage DB size: "
              << orchestrator.GetArchive().Size();
    LOG(INFO) << "--- Global Coverage Summary ---";
    for (const auto& [key, count] : orchestrator.GetGlobalCoverage()) {
      LOG(INFO) << key << ": " << count;
    }

    std::string output_db = absl::GetFlag(FLAGS_output_db);
    if (!output_db.empty()) {
      coralnpu::sim::proto::Database db;
      db.set_master_seed(seed);
      db.set_execution_limit(max_instr);

      const absl::flat_hash_map<coralnpu::fuzzer::BehavioralDescriptor,
                                coralnpu::sim::proto::TestSequence>& grid =
          orchestrator.GetArchive().GetGrid();
      std::vector<coralnpu::sim::proto::TestSequence> sorted_seqs;
      for (const auto& [desc, seq] : grid) {
        sorted_seqs.push_back(seq);
      }
      std::sort(sorted_seqs.begin(), sorted_seqs.end(),
                [](const coralnpu::sim::proto::TestSequence& a,
                   const coralnpu::sim::proto::TestSequence& b) {
                  return a.assembly_text() < b.assembly_text();
                });
      for (const auto& seq : sorted_seqs) {
        *db.add_test_sequences() = seq;
      }

      std::ofstream out(output_db, std::ios::binary);
      if (!out) {
        LOG(ERROR) << "Failed to open " << output_db << " for writing.";
        return 1;
      }

      // Enforce deterministic serialization (Finding #156)
      google::protobuf::io::OstreamOutputStream raw_output(&out);
      google::protobuf::io::CodedOutputStream coded_output(&raw_output);
      coded_output.SetSerializationDeterministic(true);
      if (!db.SerializeToCodedStream(&coded_output)) {
        LOG(ERROR) << "Failed to write protobuf database.";
        return 1;
      }
      LOG(INFO) << "Successfully wrote " << db.test_sequences_size()
                << " sequences to " << output_db;
    }
    return 0;
  }
}
