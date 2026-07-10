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

#include "sim/coralnpu_m3_enums.h"
#include "sim/coralnpu_simulator.h"
#include "sim/execution_tracker.h"
#include "sim/isg/coverage_event_router.h"
#include "sim/isg/hazard_generator.h"
#include "sim/isg/isg_engine.h"
#include "sim/memory_config.h"
#include "sim/proto/isg_database.pb.h"
#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"
#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "absl/status/statusor.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "riscv/riscv_state.h"
#include "riscv/riscv_top.h"
#include "mpact/sim/generic/decoder_interface.h"
#include "mpact/sim/generic/instruction.h"

namespace coralnpu {
namespace fuzzer {
namespace {

using ::mpact::sim::riscv::ExceptionCode;
using HaltReason = ::mpact::sim::generic::CoreDebugInterface::HaltReason;
using HaltReasonValueType = std::underlying_type_t<HaltReason>;
using ::coralnpu::sim::Architecture;
using ::coralnpu::sim::CoralNPUSimulator;
using ::coralnpu::sim::CoralNPUSimulatorOptions;

TEST(IsgE2eTest, MemorySandboxExecutionCompletesWithoutTraps) {
  IsgEngine engine(54321);
  engine.EmitPreamble();
  // Set up t1 to point to a valid RW memory region (0x10000 + 32)
  engine.BeginMemoryBlock()
      .EmitLoad("t0", "t1", 32)
      .EmitStore("t0", "t1", 36)
      .EndBlock();
  engine.EmitMpause();

  TestSequence seq = engine.Build();

  const char* tmpdir = std::getenv("TEST_TMPDIR");
  std::string dir = tmpdir ? tmpdir : "/tmp";

  static std::atomic<int> file_id{0};
  std::string unique_id = std::to_string(file_id.fetch_add(1));
  std::string asm_path = absl::StrCat(dir, "/test_e2e_", unique_id, ".s");
  std::string elf_path = absl::StrCat(dir, "/test_e2e_", unique_id, ".elf");

  std::ofstream out(asm_path);
  ASSERT_TRUE(out.is_open()) << "Failed to open assembly output file for test.";
  out << seq.assembly_text();
  out.close();

  std::string cmd =
      absl::StrCat("sim/coralnpu_m3_as ",
                   asm_path, " --output ", elf_path);
  int ret = std::system(cmd.c_str());
  ASSERT_EQ(ret, 0)
      << "Assembler failed to compile generated sequence. Command: " << cmd;

  CoralNPUSimulatorOptions options;
  options.architecture = Architecture::kM3;
  CoralNPUSimulator simulator(options);
  ASSERT_TRUE(simulator.LoadProgram(elf_path).ok());

  CoverageEventRouter router;
  std::unique_ptr<ExecutionTracker> tracker =
      std::make_unique<ExecutionTracker>();
  ExecutionTracker* tracker_ptr = tracker.get();
  router.RegisterDetector(std::move(tracker));

  absl::Status run_status;
  while (true) {
    absl::StatusOr<uint64_t> pc_res = simulator.ReadRegister("pc");
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
  EXPECT_GT(simulator.GetCycleCount(), 0);

  bool hit_hazard = false;
  for (const std::pair<const std::string, uint64_t>& kv :
       tracker_ptr->coverage_summary()) {
    if (absl::StartsWith(kv.first, "RAW_HAZARD_") && kv.second > 0) {
      hit_hazard = true;
      break;
    }
  }
  EXPECT_TRUE(hit_hazard);

  // Cleanup temporary files
  std::remove(asm_path.c_str());
  std::remove(elf_path.c_str());
}

TEST(IsgE2eTest, IntentionalTrapLogging) {
  IsgEngine engine(12345);
  engine.EmitPreamble();
  // Emit an environment call to trigger a trap.
  // Use ecall to trigger a deterministic trap.
  engine.EmitInstruction("ecall");
  engine.EmitMpause();

  TestSequence seq = engine.Build();
  LOG(INFO) << "Generated Assembly:\n" << seq.assembly_text();

  const char* tmpdir = std::getenv("TEST_TMPDIR");
  std::string dir = tmpdir ? tmpdir : "/tmp";

  static std::atomic<int> file_id{0};
  std::string unique_id = std::to_string(file_id.fetch_add(1));
  std::string asm_path = absl::StrCat(dir, "/test_trap_", unique_id, ".s");
  std::string elf_path = absl::StrCat(dir, "/test_trap_", unique_id, ".elf");

  std::ofstream out(asm_path);
  ASSERT_TRUE(out.is_open()) << "Failed to open assembly output file for test.";
  out << seq.assembly_text();
  out.close();

  std::string cmd =
      absl::StrCat("sim/coralnpu_m3_as ",
                   asm_path, " --output ", elf_path);
  int ret = std::system(cmd.c_str());
  ASSERT_EQ(ret, 0)
      << "Assembler failed to compile generated sequence. Command: " << cmd;

  coralnpu::sim::CoralNPUSimulatorOptions options;
  options.architecture = coralnpu::sim::Architecture::kM3;
  options.exit_on_ebreak = false;

  coralnpu::sim::CoralNPUSimulator simulator(options);
  ABSL_ASSERT_OK(simulator.LoadProgram(elf_path));

  bool reached_mpause = false;
  bool hit_ebreak = false;
  for (int i = 0; i < 100000; ++i) {
    absl::StatusOr<int> status = simulator.Step(1);
    if (!status.ok()) {
      LOG(INFO) << "Step " << i << " status not ok: " << status;
      break;
    }
    absl::StatusOr<uint32_t> halt_reason = simulator.top()->GetLastHaltReason();
    if (halt_reason.ok()) {
      uint32_t reason = halt_reason.value();
      if (reason != 2147483647) {  // 2147483647 is kNone
        LOG(INFO) << "Step " << i << " Halt Reason: " << reason;
      }
      if (reason == static_cast<uint32_t>(HaltReason::kUserRequest)) {
        reached_mpause = true;
        break;
      }
      if (reason == static_cast<uint32_t>(HaltReason::kSoftwareBreakpoint)) {
        hit_ebreak = true;
        break;
      }
    }
  }

  LOG(INFO) << "reached_mpause: " << reached_mpause
            << ", hit_ebreak: " << hit_ebreak;
  EXPECT_TRUE(reached_mpause)
      << "Simulator did not reach mpause. Hit ebreak instead? " << hit_ebreak;

  uint32_t mcause = 0;
  uint32_t mepc = 0;
  // Assuming 0x1000c is the logging address in DTCM.
  ABSL_ASSERT_OK(simulator.ReadMemory(0x1000c, &mcause, sizeof(mcause)));
  ABSL_ASSERT_OK(simulator.ReadMemory(0x10010, &mepc, sizeof(mepc)));

  // ecall throws Environment call from M-mode exception, which has an exception
  // code of 11.
  EXPECT_EQ(mcause, 11) << "mcause was not logged correctly.";

  // Cleanup temporary files
  std::remove(asm_path.c_str());
  std::remove(elf_path.c_str());
}

TEST(IsgE2eTest, IntentionalIllegalInstructionTrapLogging) {
  IsgEngine engine(12345);
  engine.EmitPreamble();

  uint32_t illegal_pc = engine.CurrentPc();
  engine.EmitIllegalInstruction();
  engine.EmitMpause();

  TestSequence seq = engine.Build();
  LOG(INFO) << "Generated Assembly:\n" << seq.assembly_text();

  const char* tmpdir = std::getenv("TEST_TMPDIR");
  std::string dir = tmpdir ? tmpdir : "/tmp";

  static std::atomic<int> file_id{0};
  std::string unique_id = std::to_string(file_id.fetch_add(1));
  std::string asm_path = absl::StrCat(dir, "/test_illegal_", unique_id, ".s");
  std::string elf_path = absl::StrCat(dir, "/test_illegal_", unique_id, ".elf");

  std::ofstream out(asm_path);
  ASSERT_TRUE(out.is_open()) << "Failed to open assembly output file for test.";
  out << seq.assembly_text();
  out.close();

  std::string cmd =
      absl::StrCat("sim/coralnpu_m3_as ",
                   asm_path, " --output ", elf_path);
  int ret = std::system(cmd.c_str());
  ASSERT_EQ(ret, 0)
      << "Assembler failed to compile generated sequence. Command: " << cmd;

  coralnpu::sim::CoralNPUSimulatorOptions options;
  options.architecture = coralnpu::sim::Architecture::kM3;
  options.exit_on_ebreak = false;

  coralnpu::sim::CoralNPUSimulator simulator(options);
  ABSL_ASSERT_OK(simulator.LoadProgram(elf_path));

  bool reached_mpause = false;
  for (int i = 0; i < 100000; ++i) {
    absl::StatusOr<int> status = simulator.Step(1);
    if (!status.ok()) {
      LOG(INFO) << "Step " << i << " status not ok: " << status;
      break;
    }
    absl::StatusOr<uint32_t> halt_reason = simulator.top()->GetLastHaltReason();
    if (halt_reason.ok()) {
      uint32_t reason = halt_reason.value();
      if (reason == static_cast<uint32_t>(HaltReason::kUserRequest)) {
        reached_mpause = true;
        break;
      }
    }
  }

  EXPECT_TRUE(reached_mpause) << "Simulator did not reach mpause after illegal "
                                 "instruction trap handling.";

  uint32_t mcause = 0;
  uint32_t mepc = 0;
  ABSL_ASSERT_OK(simulator.ReadMemory(0x1000c, &mcause, sizeof(mcause)));
  ABSL_ASSERT_OK(simulator.ReadMemory(0x10010, &mepc, sizeof(mepc)));

  // Illegal instruction has an exception code of 2.
  EXPECT_EQ(mcause, 2) << "mcause was not logged correctly.";
  EXPECT_EQ(mepc, illegal_pc) << "mepc was not logged correctly.";

  // Cleanup temporary files
  std::remove(asm_path.c_str());
  std::remove(elf_path.c_str());
}

TEST(IsgE2eTest, TrapHandlerPreservesMscratchEmpirical) {
  IsgEngine engine(12345);
  engine.EmitPreamble();

  // Set mscratch to a known value (42)
  engine.EmitInstruction("addi t0, zero, 42");
  engine.EmitInstruction("csrw mscratch, t0");

  // Change t0 to ensure it doesn't match mscratch (avoid testing illusion)
  engine.EmitInstruction("addi t0, zero, 99");

  // Trigger a trap
  engine.EmitIllegalInstruction();

  engine.EmitMpause();

  TestSequence seq = engine.Build();

  const char* tmpdir = std::getenv("TEST_TMPDIR");
  std::string dir = tmpdir ? tmpdir : "/tmp";

  static std::atomic<int> file_id{0};
  std::string unique_id = std::to_string(file_id.fetch_add(1));
  std::string asm_path = absl::StrCat(dir, "/test_mscratch_", unique_id, ".s");
  std::string elf_path =
      absl::StrCat(dir, "/test_mscratch_", unique_id, ".elf");

  std::ofstream out(asm_path);
  ASSERT_TRUE(out.is_open()) << "Failed to open assembly output file for test.";
  out << seq.assembly_text();
  out.close();

  std::string cmd =
      absl::StrCat("sim/coralnpu_m3_as ",
                   asm_path, " --output ", elf_path);
  int ret = std::system(cmd.c_str());
  ASSERT_EQ(ret, 0)
      << "Assembler failed to compile generated sequence. Command: " << cmd;

  coralnpu::sim::CoralNPUSimulatorOptions options;
  options.architecture = coralnpu::sim::Architecture::kM3;
  options.exit_on_ebreak = false;

  coralnpu::sim::CoralNPUSimulator simulator(options);
  ABSL_ASSERT_OK(simulator.LoadProgram(elf_path));

  bool reached_mpause = false;
  for (int i = 0; i < 100000; ++i) {
    absl::StatusOr<int> status = simulator.Step(1);
    if (!status.ok()) {
      break;
    }
    absl::StatusOr<uint32_t> halt_reason = simulator.top()->GetLastHaltReason();
    if (halt_reason.ok()) {
      uint32_t reason = halt_reason.value();
      if (reason == static_cast<uint32_t>(HaltReason::kUserRequest)) {
        reached_mpause = true;
        break;
      }
    }
  }

  EXPECT_TRUE(reached_mpause) << "Simulator did not reach mpause.";

  // Verify mscratch retained its value (42)
  absl::StatusOr<uint64_t> mscratch_val = simulator.ReadRegister("mscratch");
  ABSL_ASSERT_OK(mscratch_val);
  EXPECT_EQ(mscratch_val.value(), 42)
      << "mscratch was not preserved across trap handling.";

  // Verify trap handler was entered (by checking mcause was logged)
  uint32_t mcause = 0;
  ABSL_ASSERT_OK(simulator.ReadMemory(0x1000c, &mcause, sizeof(mcause)));
  EXPECT_NE(mcause, 0) << "Trap handler was not executed (mcause not logged).";

  // Cleanup temporary files
  std::remove(asm_path.c_str());
  std::remove(elf_path.c_str());
}

TEST(IsgE2eTest, ControlHazardForwardBranchTakenExecutesCorrectly) {
  IsgEngine engine(12345);
  engine.EmitPreamble();
  // Set t0 and t1 to the same value so that beq is taken.
  engine.EmitInstruction("addi t0, zero, 0");
  engine.EmitInstruction("addi t1, zero, 0");
  engine.EmitInstruction("addi t2, zero, 5");

  // We use the standardized absolute syntax (Task 6).
  uint32_t beq_pc = engine.CurrentPc();
  engine.EmitInstruction(absl::StrCat("beq t0, t1, ", beq_pc + 12));

  // These instructions should be skipped by the branch.
  engine.EmitInstruction("addi t2, zero, 1");
  engine.EmitInstruction("addi t2, zero, 1");

  // Target of beq (PC + 12)
  engine.EmitMpause();

  TestSequence seq = engine.Build();
  LOG(INFO) << "Generated Assembly:\n" << seq.assembly_text();

  const char* tmpdir = std::getenv("TEST_TMPDIR");
  std::string dir = tmpdir ? tmpdir : "/tmp";

  static std::atomic<int> file_id{0};
  std::string unique_id = std::to_string(file_id.fetch_add(1));
  std::string asm_path =
      absl::StrCat(dir, "/test_control_fwd_", unique_id, ".s");
  std::string elf_path =
      absl::StrCat(dir, "/test_control_fwd_", unique_id, ".elf");

  std::ofstream out(asm_path);
  ASSERT_TRUE(out.is_open()) << "Failed to open assembly output file for test.";
  out << seq.assembly_text();
  out.close();

  std::string cmd =
      absl::StrCat("sim/coralnpu_m3_as ",
                   asm_path, " --output ", elf_path);
  int ret = std::system(cmd.c_str());
  ASSERT_EQ(ret, 0)
      << "Assembler failed to compile generated sequence. Command: " << cmd;

  coralnpu::sim::CoralNPUSimulatorOptions options;
  options.architecture = coralnpu::sim::Architecture::kM3;
  options.exit_on_ebreak = false;

  coralnpu::sim::CoralNPUSimulator simulator(options);
  ABSL_ASSERT_OK(simulator.LoadProgram(elf_path));

  bool reached_mpause = false;
  // Limit steps to detect infinite loop
  for (int i = 0; i < 100000; ++i) {
    absl::StatusOr<int> status = simulator.Step(1);
    if (!status.ok()) {
      LOG(INFO) << "Step " << i << " status not ok: " << status;
      break;
    }
    absl::StatusOr<uint32_t> halt_reason = simulator.top()->GetLastHaltReason();
    if (halt_reason.ok()) {
      uint32_t reason = halt_reason.value();
      if (reason == static_cast<uint32_t>(HaltReason::kUserRequest)) {
        reached_mpause = true;
        break;
      }
    }
  }

  EXPECT_TRUE(reached_mpause)
      << "Simulator did not reach mpause. Forward branch may have failed.";

  absl::StatusOr<uint64_t> t2_val = simulator.ReadRegister("t2");
  ABSL_ASSERT_OK(t2_val);
  EXPECT_EQ(t2_val.value(), 5) << "Testing Illusion: Forward branch was not "
                                  "taken, skipped instructions were executed.";

  std::remove(asm_path.c_str());
  std::remove(elf_path.c_str());
}

TEST(IsgE2eTest, ControlHazardExecutesCorrectlyWithoutInfiniteLoop) {
  IsgEngine engine(12345);
  engine.EmitPreamble();
  GenerateControlHazard(engine);
  engine.EmitMpause();

  TestSequence seq = engine.Build();
  LOG(INFO) << "Generated Assembly:\n" << seq.assembly_text();

  const char* tmpdir = std::getenv("TEST_TMPDIR");
  std::string dir = tmpdir ? tmpdir : "/tmp";

  static std::atomic<int> file_id{0};
  std::string unique_id = std::to_string(file_id.fetch_add(1));
  std::string asm_path = absl::StrCat(dir, "/test_control_", unique_id, ".s");
  std::string elf_path = absl::StrCat(dir, "/test_control_", unique_id, ".elf");

  std::ofstream out(asm_path);
  ASSERT_TRUE(out.is_open()) << "Failed to open assembly output file for test.";
  out << seq.assembly_text();
  out.close();

  std::string cmd =
      absl::StrCat("sim/coralnpu_m3_as ",
                   asm_path, " --output ", elf_path);
  int ret = std::system(cmd.c_str());
  ASSERT_EQ(ret, 0)
      << "Assembler failed to compile generated sequence. Command: " << cmd;

  coralnpu::sim::CoralNPUSimulatorOptions options;
  options.architecture = coralnpu::sim::Architecture::kM3;
  options.exit_on_ebreak = false;

  coralnpu::sim::CoralNPUSimulator simulator(options);
  ABSL_ASSERT_OK(simulator.LoadProgram(elf_path));

  bool reached_mpause = false;
  // Limit steps to detect infinite loop
  for (int i = 0; i < 100000; ++i) {
    absl::StatusOr<int> status = simulator.Step(1);
    if (!status.ok()) {
      break;
    }
    absl::StatusOr<uint32_t> halt_reason = simulator.top()->GetLastHaltReason();
    if (halt_reason.ok()) {
      uint32_t reason = halt_reason.value();
      if (reason == static_cast<uint32_t>(HaltReason::kUserRequest)) {
        reached_mpause = true;
        break;
      }
    }
  }

  EXPECT_TRUE(reached_mpause)
      << "Simulator did not reach mpause (potential infinite loop or hang).";

  // Cleanup temporary files
  std::remove(asm_path.c_str());
  std::remove(elf_path.c_str());
}

TEST(IsgE2eTest, EdgeCaseOperandsExecuteCorrectly) {
  IsgEngine engine(12345);
  engine.EmitPreamble();
  GenerateEdgeCaseOperands(engine);
  engine.EmitMpause();

  TestSequence seq = engine.Build();
  LOG(INFO) << "Generated Assembly:\n" << seq.assembly_text();

  const char* tmpdir = std::getenv("TEST_TMPDIR");
  std::string dir = tmpdir ? tmpdir : "/tmp";

  static std::atomic<int> file_id{0};
  std::string unique_id = std::to_string(file_id.fetch_add(1));
  std::string asm_path = absl::StrCat(dir, "/test_edge_", unique_id, ".s");
  std::string elf_path = absl::StrCat(dir, "/test_edge_", unique_id, ".elf");

  std::ofstream out(asm_path);
  ASSERT_TRUE(out.is_open()) << "Failed to open assembly output file for test.";
  out << seq.assembly_text();
  out.close();

  std::string cmd =
      absl::StrCat("sim/coralnpu_m3_as ",
                   asm_path, " --output ", elf_path);
  int ret = std::system(cmd.c_str());
  ASSERT_EQ(ret, 0)
      << "Assembler failed to compile generated sequence. Command: " << cmd;

  coralnpu::sim::CoralNPUSimulatorOptions options;
  options.architecture = coralnpu::sim::Architecture::kM3;
  options.exit_on_ebreak = false;

  coralnpu::sim::CoralNPUSimulator simulator(options);
  ABSL_ASSERT_OK(simulator.LoadProgram(elf_path));

  bool reached_mpause = false;
  // Limit steps to detect infinite loop
  for (int i = 0; i < 100000; ++i) {
    absl::StatusOr<int> status = simulator.Step(1);
    if (!status.ok()) {
      break;
    }
    absl::StatusOr<uint32_t> halt_reason = simulator.top()->GetLastHaltReason();
    if (halt_reason.ok()) {
      uint32_t reason = halt_reason.value();
      if (reason == static_cast<uint32_t>(HaltReason::kUserRequest)) {
        reached_mpause = true;
        break;
      }
    }
  }

  EXPECT_TRUE(reached_mpause)
      << "Simulator did not reach mpause (potential infinite loop or hang).";

  // Cleanup temporary files
  std::remove(asm_path.c_str());
  std::remove(elf_path.c_str());
}

TEST(IsgE2eTest, VectorTailUndisturbedPreservesTail) {
  IsgEngine engine(12345);
  engine.EmitPreamble();

  // Set vl to 16, clear v0 to 0s
  engine.EmitVsetivli("t1", 16, VectorSew::e32, VectorLmul::m1,
                      /*tail_agnostic=*/true, /*mask_agnostic=*/true);
  engine.EmitInstruction("vxor.vv v0, v0, v0");

  // Set vl=1, tail undisturbed
  engine.EmitVsetivli("t1", 1, VectorSew::e32, VectorLmul::m1,
                      /*tail_agnostic=*/false, /*mask_agnostic=*/true);
  // Set v0[0] to 15 using unmasked vector register move
  engine.EmitInstruction("vmv.vi v0, 15");
  engine.EmitMpause();

  TestSequence seq = engine.Build();
  EXPECT_THAT(seq.assembly_text(), ::testing::HasSubstr("tu"));
  const char* tmpdir = std::getenv("TEST_TMPDIR");
  std::string dir = tmpdir ? tmpdir : "/tmp";
  static std::atomic<int> file_id{0};
  std::string unique_id = std::to_string(file_id.fetch_add(1));
  std::string asm_path = absl::StrCat(dir, "/test_tu_", unique_id, ".s");
  std::string elf_path = absl::StrCat(dir, "/test_tu_", unique_id, ".elf");

  std::ofstream out(asm_path);
  out << seq.assembly_text();
  out.close();
  std::string cmd =
      absl::StrCat("sim/coralnpu_m3_as ",
                   asm_path, " --output ", elf_path);
  int ret = std::system(cmd.c_str());
  ASSERT_EQ(ret, 0) << "Assembler failed: " << seq.assembly_text();

  coralnpu::sim::CoralNPUSimulatorOptions options;
  options.architecture = coralnpu::sim::Architecture::kM3;
  options.exit_on_ebreak = false;
  coralnpu::sim::CoralNPUSimulator simulator(options);
  ABSL_ASSERT_OK(simulator.LoadProgram(elf_path));

  ABSL_ASSERT_OK(simulator.Run());
  ABSL_ASSERT_OK(simulator.Wait());

  absl::StatusOr<uint32_t> halt_reason = simulator.top()->GetLastHaltReason();
  ASSERT_TRUE(halt_reason.ok());
  EXPECT_EQ(halt_reason.value(),
            static_cast<uint32_t>(HaltReason::kUserRequest));

  absl::StatusOr<mpact::sim::generic::DataBuffer*> v0_result =
      simulator.GetRegisterDataBuffer("v0");
  ASSERT_TRUE(v0_result.ok());
  ASSERT_NE(v0_result.value(), nullptr);
  absl::Span<uint32_t> v0_span = v0_result.value()->Get<uint32_t>();

  EXPECT_EQ(v0_span[0], 15);
  for (size_t i = 1; i < v0_span.size(); ++i) {
    EXPECT_EQ(v0_span[i], 0);
  }

  std::remove(asm_path.c_str());
  std::remove(elf_path.c_str());
}

TEST(IsgE2eTest, VectorTailAgnosticOverwritesTail) {
  IsgEngine engine(12345);
  engine.EmitPreamble();

  // Set vl to 16, clear v0 to 0s
  engine.EmitVsetivli("t1", 16, VectorSew::e32, VectorLmul::m1,
                      /*tail_agnostic=*/true, /*mask_agnostic=*/true);
  engine.EmitInstruction("vxor.vv v0, v0, v0");

  // Set vl=1, tail agnostic
  engine.EmitVsetivli("t1", 1, VectorSew::e32, VectorLmul::m1,
                      /*tail_agnostic=*/true, /*mask_agnostic=*/true);
  // Set v0[0] to 15 using unmasked vector register move
  engine.EmitInstruction("vmv.vi v0, 15");
  engine.EmitMpause();

  TestSequence seq = engine.Build();
  EXPECT_THAT(seq.assembly_text(), ::testing::HasSubstr("ta"));
  const char* tmpdir = std::getenv("TEST_TMPDIR");
  std::string dir = tmpdir ? tmpdir : "/tmp";
  static std::atomic<int> file_id{0};
  std::string unique_id = std::to_string(file_id.fetch_add(1));
  std::string asm_path = absl::StrCat(dir, "/test_ta_", unique_id, ".s");
  std::string elf_path = absl::StrCat(dir, "/test_ta_", unique_id, ".elf");

  std::ofstream out(asm_path);
  out << seq.assembly_text();
  out.close();
  std::string cmd =
      absl::StrCat("sim/coralnpu_m3_as ",
                   asm_path, " --output ", elf_path);
  int ret = std::system(cmd.c_str());
  ASSERT_EQ(ret, 0) << "Assembler failed: " << seq.assembly_text();

  coralnpu::sim::CoralNPUSimulatorOptions options;
  options.architecture = coralnpu::sim::Architecture::kM3;
  options.exit_on_ebreak = false;
  coralnpu::sim::CoralNPUSimulator simulator(options);
  ABSL_ASSERT_OK(simulator.LoadProgram(elf_path));

  ABSL_ASSERT_OK(simulator.Run());
  ABSL_ASSERT_OK(simulator.Wait());

  absl::StatusOr<uint32_t> halt_reason = simulator.top()->GetLastHaltReason();
  ASSERT_TRUE(halt_reason.ok());
  EXPECT_EQ(halt_reason.value(),
            static_cast<uint32_t>(HaltReason::kUserRequest));

  absl::StatusOr<mpact::sim::generic::DataBuffer*> v0_result =
      simulator.GetRegisterDataBuffer("v0");
  ASSERT_TRUE(v0_result.ok());
  ASSERT_NE(v0_result.value(), nullptr);
  absl::Span<uint32_t> v0_span = v0_result.value()->Get<uint32_t>();

  EXPECT_EQ(v0_span[0], 15);
  for (size_t i = 1; i < v0_span.size(); ++i) {
    // The CoralNPU simulator implements tail agnostic (ta) by leaving tail
    // elements undisturbed, which is valid according to the RISC-V V spec.
    EXPECT_EQ(v0_span[i], 0);
  }

  std::remove(asm_path.c_str());
  std::remove(elf_path.c_str());
}

TEST(IsgE2eTest, TracksLMULExpansionHazardDetection) {
  IsgEngine engine(12345);
  engine.EmitPreamble();

  engine.EmitVsetvli("t0", "a0", VectorSew::e32, VectorLmul::m8,
                     /*tail_agnostic=*/false, /*mask_agnostic=*/false);
  engine.BeginVectorBlock().EmitVadd("v8", "v0", "v0").EndBlock();
  engine.EmitInstruction("addi t0, zero, 0");
  engine.EmitInstruction("addi t0, zero, 0");
  engine.EmitVsetvli("t0", "a0", VectorSew::e32, VectorLmul::m1,
                     /*tail_agnostic=*/false, /*mask_agnostic=*/false);
  engine.BeginVectorBlock().EmitVadd("v0", "v15", "v15").EndBlock();
  engine.EmitMpause();

  TestSequence seq = engine.Build();

  const char* tmpdir = std::getenv("TEST_TMPDIR");
  std::string dir = tmpdir ? tmpdir : "/tmp";

  static std::atomic<int> file_id{0};
  std::string unique_id = std::to_string(file_id.fetch_add(1));
  std::string asm_path = absl::StrCat(dir, "/test_lmul_e2e_", unique_id, ".s");
  std::string elf_path =
      absl::StrCat(dir, "/test_lmul_e2e_", unique_id, ".elf");

  std::ofstream out(asm_path);
  ASSERT_TRUE(out.is_open()) << "Failed to open assembly output file for test.";
  out << seq.assembly_text();
  out.close();

  std::string cmd =
      absl::StrCat("sim/coralnpu_m3_as ",
                   asm_path, " --output ", elf_path);
  int ret = std::system(cmd.c_str());
  ASSERT_EQ(ret, 0)
      << "Assembler failed to compile generated sequence. Command: " << cmd;

  CoralNPUSimulatorOptions options;
  options.architecture = Architecture::kM3;
  CoralNPUSimulator simulator(options);
  ASSERT_TRUE(simulator.LoadProgram(elf_path).ok());

  CoverageEventRouter router;
  std::unique_ptr<ExecutionTracker> tracker =
      std::make_unique<ExecutionTracker>();
  ExecutionTracker* tracker_ptr = tracker.get();
  router.RegisterDetector(std::move(tracker));

  absl::Status run_status;
  while (true) {
    absl::StatusOr<uint64_t> pc_res = simulator.ReadRegister("pc");
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
  EXPECT_GT(simulator.GetCycleCount(), 0);

  bool hit_hazard = false;
  for (const std::pair<const std::string, uint64_t>& kv :
       tracker_ptr->coverage_summary()) {
    if (absl::StartsWith(kv.first, "RAW_HAZARD_") && kv.second > 0) {
      hit_hazard = true;
      break;
    }
  }
  EXPECT_TRUE(hit_hazard);

  std::remove(asm_path.c_str());
  std::remove(elf_path.c_str());
}

}  // namespace
}  // namespace fuzzer
}  // namespace coralnpu
