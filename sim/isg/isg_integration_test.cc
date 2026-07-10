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

#include <atomic>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <variant>

#include "sim/coralnpu_m3_enums.h"
#include "sim/coralnpu_simulator.h"
#include "sim/execution_tracker.h"
#include "sim/isg/coverage_event_router.h"
#include "sim/isg/isg_engine.h"
#include "sim/memory_config.h"
#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"
#include "absl/debugging/leak_check.h"
#include "absl/strings/match.h"
#include "mpact/sim/generic/decoder_interface.h"
#include "mpact/sim/generic/instruction.h"

namespace coralnpu {
namespace fuzzer {
namespace {

using ::coralnpu::sim::CoralNPUSimulator;
using ::coralnpu::sim::CoralNPUSimulatorOptions;

TEST(IsgIntegrationTest, ValidatesOpcodeCoverageCounters) {
  IsgEngine engine(12345);
  engine.EmitPreamble();
  engine.EmitInstruction("add t0, t1, t2");
  engine.EmitInstruction("sub t0, t1, t2");
  engine.EmitInstruction("and t0, t1, t2");
  engine.EmitInstruction("or t0, t1, t2");
  engine.EmitInstruction("xor t0, t1, t2");
  engine.EmitInstruction("sll t0, t1, t2");
  engine.EmitInstruction("srl t0, t1, t2");
  engine.EmitInstruction("sra t0, t1, t2");
  engine.EmitInstruction("slt t0, t1, t2");
  engine.EmitInstruction("sltu t0, t1, t2");
  engine.EmitInstruction("mul t0, t1, t2");
  engine.EmitInstruction("div t0, t1, t2");
  engine.EmitInstruction("rem t0, t1, t2");
  engine.EmitInstruction("lui t0, 0x10");
  engine.EmitInstruction("auipc t0, 0x10");
  engine.EmitInstruction("vadd.vv v1, v2, v3");
  engine.EmitInstruction("fadd.s f1, f2, f3");
  engine.EmitInstruction("csrrw t0, mscratch, t1");
  engine.BeginMemoryBlock().EmitLoad("t0", "t1", 1).EndBlock();
  engine.EmitMpause();

  TestSequence seq = engine.Build();

  const char* tmpdir = std::getenv("TEST_TMPDIR");
  std::string dir = tmpdir ? tmpdir : "/tmp";

  // Use a unique file identifier per run to avoid collisions
  static std::atomic<int> file_id{0};
  std::string unique_id = std::to_string(file_id.fetch_add(1));
  std::string asm_path = dir + "/test_counters_" + unique_id + ".s";
  std::string elf_path = dir + "/test_counters_" + unique_id + ".elf";

  std::ofstream out(asm_path);
  ASSERT_TRUE(out.is_open()) << "Failed to open assembly output file for test.";
  out << seq.assembly_text();
  out.close();

  std::string cmd = "sim/coralnpu_m3_as " +
                    asm_path + " --output " + elf_path;
  int ret = std::system(cmd.c_str());
  ASSERT_EQ(ret, 0)
      << "Assembler failed to compile generated sequence. Command: " << cmd;

  ::coralnpu::sim::CoralNPUSimulatorOptions options;
  options.architecture = ::coralnpu::sim::Architecture::kM3;
  ::coralnpu::sim::CoralNPUSimulator sim(options);
  ASSERT_TRUE(sim.LoadProgram(elf_path).ok());

  sim.top()->EnableStatistics();
  ASSERT_TRUE(sim.Run().ok());
  ASSERT_TRUE(sim.Wait().ok());

  int covered_scalar_opcodes = 0;
  int covered_vector_opcodes = 0;
  int covered_fp_opcodes = 0;
  int covered_csr_opcodes = 0;

  for (const auto& [name, counter] : sim.top()->counter_map()) {
    if (absl::StartsWith(name, "num_") && name != "num_instructions" &&
        name != "num_cycles") {
      uint64_t val = std::get<uint64_t>(counter->GetCounterValue());
      if (val > 0) {
        if (absl::StartsWith(name, "num_V")) {
          covered_vector_opcodes++;
        } else if (absl::StartsWith(name, "num_F")) {
          covered_fp_opcodes++;
        } else if (absl::StartsWith(name, "num_C")) {
          covered_csr_opcodes++;
        } else {
          covered_scalar_opcodes++;
        }
      }
    }
  }

  EXPECT_EQ(covered_scalar_opcodes, 25);
  EXPECT_EQ(covered_vector_opcodes, 3);
  EXPECT_EQ(covered_fp_opcodes, 2);
  EXPECT_EQ(covered_csr_opcodes, 2);

  // Cleanup temporary files
  std::remove(asm_path.c_str());
  std::remove(elf_path.c_str());
  google::protobuf::ShutdownProtobufLibrary();
}

TEST(IsgIntegrationTest, ValidatesRawHazardExtraction) {
  IsgEngine engine(12345);
  engine.EmitPreamble();
  engine.EmitDataHazard(::coralnpu::sim::isa32_m3::OpcodeEnum::kAdd,
                        ::coralnpu::sim::isa32_m3::OpcodeEnum::kSub);
  engine.EmitMpause();

  TestSequence seq = engine.Build();

  const char* tmpdir = std::getenv("TEST_TMPDIR");
  std::string dir = tmpdir ? tmpdir : "/tmp";

  // Use a unique file identifier per run to avoid collisions
  static std::atomic<int> file_id{0};
  std::string unique_id = std::to_string(file_id.fetch_add(1));
  std::string asm_path = dir + "/test_hazard_" + unique_id + ".s";
  std::string elf_path = dir + "/test_hazard_" + unique_id + ".elf";

  std::ofstream out(asm_path);
  ASSERT_TRUE(out.is_open()) << "Failed to open assembly output file for test.";
  out << seq.assembly_text();
  out.close();

  std::string cmd = "sim/coralnpu_m3_as " +
                    asm_path + " --output " + elf_path;
  int ret = std::system(cmd.c_str());
  ASSERT_EQ(ret, 0)
      << "Assembler failed to compile generated sequence. Command: " << cmd;

  CoralNPUSimulatorOptions options;
  options.architecture = ::coralnpu::sim::Architecture::kM3;
  CoralNPUSimulator simulator(options);
  ASSERT_TRUE(simulator.LoadProgram(elf_path).ok());

  CoverageEventRouter router;
  std::unique_ptr<ExecutionTracker> tracker =
      std::make_unique<ExecutionTracker>();
  ExecutionTracker* tracker_ptr = tracker.get();
  router.RegisterDetector(std::move(tracker));

  absl::Status run_status;
  while (true) {
    absl::StatusOr<uint32_t> pc_res = simulator.ReadRegister("pc");
    if (!pc_res.ok()) break;
    uint32_t current_pc = pc_res.value();

    ::mpact::sim::generic::Instruction* inst =
        simulator.decoder()->DecodeInstruction(current_pc);
    if (inst) {
      router.RouteInstruction(inst);
      inst->DecRef();
    }

    run_status = simulator.Step(1).status();
    if (!run_status.ok()) break;
    absl::StatusOr<uint32_t> halt_reason_or =
        simulator.top()->GetLastHaltReason();
    if (halt_reason_or.ok() &&
        halt_reason_or.value() !=
            static_cast<uint32_t>(
                ::mpact::sim::generic::CoreDebugInterface::HaltReason::kNone)) {
      break;
    }
  }
  EXPECT_TRUE(run_status.ok());

  const absl::flat_hash_map<std::string, uint64_t>& summary =
      tracker_ptr->coverage_summary();
  absl::flat_hash_map<std::string, uint64_t>::const_iterator it =
      summary.find("RAW_HAZARD_1");
  ASSERT_NE(it, summary.end()) << "RAW_HAZARD_1 not found in coverage summary";
  EXPECT_GT(it->second, 0);

  // Cleanup temporary files
  std::remove(asm_path.c_str());
  std::remove(elf_path.c_str());
}

}  // namespace
}  // namespace fuzzer
}  // namespace coralnpu
