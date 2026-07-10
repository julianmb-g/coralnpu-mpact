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
#include <memory>
#include <string>
#include <vector>

#include "sim/coralnpu_simulator.h"
#include "sim/execution_tracker.h"
#include "sim/isg/coverage_event_router.h"
#include "sim/isg/isg_engine.h"
#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"
#include "absl/cleanup/cleanup.h"
#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"

namespace coralnpu {
namespace sim {
namespace isg {
namespace {

using ::testing::Contains;
using ::testing::HasSubstr;
using ::testing::Not;

void EmitLi(::coralnpu::fuzzer::IsgEngine& engine, const std::string& reg,
            uint32_t val) {
  uint32_t upper = (val + 0x800) >> 12;
  int32_t lower = static_cast<int32_t>(val) - (upper << 12);
  engine.EmitInstructionFormat("lui %s, 0x%x", reg, upper);
  if (lower != 0 || upper == 0) {
    engine.EmitInstructionFormat("addi %s, %s, %d", reg, reg, lower);
  }
}

absl::flat_hash_map<std::string, uint64_t> RunEngineAndGetCoverage(
    ::coralnpu::fuzzer::IsgEngine& engine) {
  engine.EmitMpause();
  ::coralnpu::fuzzer::TestSequence seq = engine.Build();

  const char* tmpdir = std::getenv("TEST_TMPDIR");
  std::string dir = tmpdir ? tmpdir : "/tmp";

  static std::atomic<int> file_id{0};
  std::string unique_id = std::to_string(file_id.fetch_add(1));
  std::string asm_path = dir + "/cov_sim_fp_pill_" + unique_id + ".s";
  std::string elf_path = dir + "/cov_sim_fp_pill_" + unique_id + ".elf";

  absl::Cleanup cleanup = [&asm_path, &elf_path] {
    std::remove(asm_path.c_str());
    std::remove(elf_path.c_str());
  };

  std::ofstream out(asm_path);
  EXPECT_TRUE(out.is_open()) << "Failed to open assembly output file for test.";
  out << seq.assembly_text();
  out.close();

  std::string cmd = "sim/coralnpu_m3_as " +
                    asm_path + " --output " + elf_path;
  int ret = std::system(cmd.c_str());
  EXPECT_EQ(ret, 0) << "Assembler failed. Command: " << cmd;

  CoralNPUSimulatorOptions options;
  options.architecture = Architecture::kM3;
  CoralNPUSimulator simulator(options);
  EXPECT_TRUE(simulator.LoadProgram(elf_path).ok());

  ::coralnpu::fuzzer::CoverageEventRouter router;
  std::unique_ptr<::coralnpu::fuzzer::ExecutionTracker> tracker =
      std::make_unique<::coralnpu::fuzzer::ExecutionTracker>();
  ::coralnpu::fuzzer::ExecutionTracker* tracker_ptr = tracker.get();
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

  return tracker_ptr->coverage_summary();
}

// Tests that a NaN-boxed scalar is tracked correctly, and DOES NOT trigger a
// Signaling NaN false positive.
TEST(IsgFpPoisonPillTest, TracksNaNBoxedScalarWithoutSemanticCamouflage) {
  ::coralnpu::fuzzer::IsgEngine engine(12345);
  engine.EmitPreamble();
  // NaN-boxed scalar (0xFFFFFFFF)
  EmitLi(engine, "t0", 0xFFFFFFFF);
  engine.EmitInstruction("mv.w.x f1, t0");
  engine.EmitInstruction("fadd.s f7, f1, f1");

  absl::flat_hash_map<std::string, uint64_t> summary =
      RunEngineAndGetCoverage(engine);

  EXPECT_GT(summary["fp_poison_pill_NaN-boxed_scalar"], 0);
  // This is the core fix for the Semantic Camouflage mutant:
  // A NaN-boxed scalar should NOT be evaluated as a Signaling NaN.
  EXPECT_FALSE(summary.contains("fp_poison_pill_Signaling_NaN"));
}

// Tests that an actual Signaling NaN is tracked correctly.
TEST(IsgFpPoisonPillTest, TracksSignalingNaN) {
  ::coralnpu::fuzzer::IsgEngine engine(12345);
  engine.EmitPreamble();
  // Signaling NaN (e.g., 0x7F800001)
  EmitLi(engine, "t0", 0x7F800001);
  engine.EmitInstruction("mv.w.x f2, t0");
  engine.EmitInstruction("fadd.s f7, f2, f2");

  absl::flat_hash_map<std::string, uint64_t> summary =
      RunEngineAndGetCoverage(engine);

  EXPECT_GT(summary["fp_poison_pill_Signaling_NaN"], 0);
}

}  // namespace
}  // namespace isg
}  // namespace sim
}  // namespace coralnpu
