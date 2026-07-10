#include "sim/isg/isg_engine.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>

#include "sim/coralnpu_simulator.h"
#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"
#include "absl/status/status_matchers.h"
#include "absl/strings/match.h"
#include "absl/strings/numbers.h"

/* start - new test macros */
#define ABSL_EXPECT_OK(expression) \
  EXPECT_THAT(expression, ::absl_testing::IsOk())
#define ABSL_ASSERT_OK(expression) \
  ASSERT_THAT(expression, ::absl_testing::IsOk())
/* end - new test macros */

/* start - helper test macro */
#define CONCAT_INNER(a, b) a##b
#define CONCAT(a, b) CONCAT_INNER(a, b)
#define ASSERT_OK_AND_ASSIGN(lhs, rexpr)                 \
  auto CONCAT(status_or_line_, __LINE__) = (rexpr);      \
  ABSL_ASSERT_OK(CONCAT(status_or_line_, __LINE__));          \
  lhs = std::move(CONCAT(status_or_line_, __LINE__)).value();
/* end - helper test macro */

namespace coralnpu {
namespace fuzzer {
namespace {

TEST(IsgEngineTest, SetSeedResetsPrngState) {
  IsgEngine engine(12345);
  uint64_t val1 = engine.prng()();
  uint64_t val2 = engine.prng()();

  // Finding #190: SetSeed should reset PRNG state
  engine.SetSeed(12345);
  uint64_t val3 = engine.prng()();
  uint64_t val4 = engine.prng()();

  EXPECT_EQ(val1, val3);
  EXPECT_EQ(val2, val4);
}

TEST(IsgEngineTest, AddHazardTagPopulatesProto) {
  IsgEngine engine(12345);
  engine.AddHazardTag("RAW_HAZARD");
  engine.AddHazardTag("FP_PILL");
  TestSequence seq = engine.Build();
  ASSERT_EQ(seq.hazard_tags_size(), 2);
  EXPECT_EQ(seq.hazard_tags(0), "RAW_HAZARD");
  EXPECT_EQ(seq.hazard_tags(1), "FP_PILL");
}

TEST(IsgEngineTest, SetExpectedTrapPopulatesProto) {
  IsgEngine engine(12345);
  engine.SetExpectedTrap(1, 0x1000, 0x2000);
  TestSequence seq = engine.Build();
  ASSERT_TRUE(seq.has_trap_event());
  EXPECT_EQ(seq.trap_event().mcause(), 1);
  EXPECT_EQ(seq.trap_event().mepc(), 0x1000);
  EXPECT_EQ(seq.trap_event().mtval(), 0x2000);
}

TEST(IsgEngineTest, MemorySandboxAlignment) {
  EXPECT_EQ(IsgEngine::kReservedDtcmBytes % 512, 0);
}

TEST(IsgEngineTest, EndToEndGeneration) {
  IsgEngine engine(12345);
  engine.EmitPreamble()
      .EmitVsetvli("t2", "t3", VectorSew::e8, VectorLmul::m1)
      .EmitInstruction("vadd.vv v0, v1, v2");
  // Mpause and padding should now be automatically handled by Build().

  TestSequence seq = engine.Build();

  EXPECT_EQ(seq.prng_seed(), 12345);
  EXPECT_TRUE(absl::StrContains(seq.assembly_text(), "vadd.vv v0, v1, v2"));
  EXPECT_TRUE(absl::StrContains(seq.assembly_text(), ".word 0x08000073"));
  EXPECT_TRUE(
      absl::StrContains(seq.assembly_text(), "vsetvli t2, t3, e8, m1, ta, ma"));
  EXPECT_EQ(seq.expected_terminal_state(), "mpause");

  const char* tmpdir = std::getenv("TEST_TMPDIR");
  std::string dir = tmpdir ? tmpdir : "/tmp";
  std::string asm_path =
      dir + "/test_e2e_" + std::to_string(std::rand()) + ".s";
  std::string elf_path =
      dir + "/test_e2e_" + std::to_string(std::rand()) + ".elf";

  std::ofstream out(asm_path);
  ASSERT_TRUE(out.is_open()) << "Failed to open assembly output file for test.";
  out << seq.assembly_text();
  out.close();

  std::string cmd = "sim/coralnpu_m3_as " +
                    asm_path + " --output " + elf_path;
  int ret = std::system(cmd.c_str());
  EXPECT_EQ(ret, 0)
      << "Assembler failed to compile generated sequence. Command: " << cmd;
}

TEST(IsgEngineTest, PreambleInitializesRegisters) {
  IsgEngine engine(12345);
  engine.EmitPreamble();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  // Assert X registers are initialized
  EXPECT_TRUE(absl::StrContains(text, "lui x1"));
  EXPECT_TRUE(absl::StrContains(text, "lui x31, "));

  // Assert F registers are initialized via float loads (or fmv)
  EXPECT_TRUE(absl::StrContains(text, "mv.w.x f0, "));
  EXPECT_TRUE(absl::StrContains(text, "mv.w.x f31, "));

  // Assert V registers are initialized
  EXPECT_TRUE(absl::StrContains(text, "vsetvli t2, t3, e8, m8, ta, ma"));
  EXPECT_TRUE(absl::StrContains(text, "vle8.v v0,(t0)"));
  EXPECT_TRUE(absl::StrContains(text, "vle8.v v24,(t0)"));

  // Assert fcsr is initialized
  EXPECT_TRUE(absl::StrContains(text, "csrw fcsr, "));
}

TEST(IsgEngineTest, PreambleSkipsTrapHandlerWithCorrectOffset) {
  IsgEngine engine(12345);
  engine.EmitPreamble();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  EXPECT_TRUE(absl::StrContains(text, "jal zero, 0x70"));
  EXPECT_FALSE(absl::StrContains(text, "jal zero, 0x64"));
}

TEST(IsgEngineTest, EmitLiNonZeroLowerBitEmitsAddi) {
  IsgEngine engine(12345);
  engine.EmitPreamble();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  // 0x6600 (26112) is loaded into t0 during preamble for mstatus.
  // upper = (26112 + 2048) >> 12 = 6
  // lower = 26112 - (6 << 12) = 1536
  // This test ensures the `lower != 0` condition in EmitLi correctly emits
  // addi.
  EXPECT_TRUE(absl::StrContains(text, "lui t0, 0x6"));
  EXPECT_TRUE(absl::StrContains(text, "addi t0, t0, 1536"));
}

TEST(IsgEngineTest, VectorFallbackInitializesLength) {
  IsgEngine engine(12345);
  // Do not call EmitPreamble or EmitVsetvli
  engine.EmitInstruction("vadd.vv v0, v1, v2");
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  EXPECT_TRUE(absl::StrContains(text, "addi t3, zero, "));
  EXPECT_TRUE(absl::StrContains(text, "vsetvli t2, t3"));
}

TEST(IsgEngineTest, ExplicitVsetvliPreventsFallback) {
  IsgEngine engine(12345);
  // User explicitly sets vsetvli to VLMAX
  engine.EmitVsetvli("t2", "zero", VectorSew::e32, VectorLmul::m8);
  engine.EmitInstruction("vmul.vv v0, v1, v2");
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  // The fallback vsetvli logic uses "addi t3, zero, "
  EXPECT_FALSE(absl::StrContains(text, "addi t3, zero, "));
  // Verify the explicit instruction is right before vmul.vv
  EXPECT_TRUE(absl::StrContains(
      text, "vsetvli t2, zero, e32, m8, ta, ma\nvmul.vv v0, v1, v2"));
}

TEST(IsgEngineTest, PreambleExecutionTimeoutFixed) {
  IsgEngine engine(12345);
  engine.EmitPreamble();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());
  // 1048576 corresponds to lui t2, 256. 8192 corresponds to lui t2, 2.
  // Make sure we are not looping 1,048,576 times for region 2.
  EXPECT_FALSE(absl::StrContains(text, "lui t2, 0x100"))
      << "Preamble loop is too large, will cause timeout!";
}

TEST(IsgEngineTest, MemorySandboxScalarEscapeFixed) {
  IsgEngine engine(12345);
  engine.BeginMemoryBlock().EmitLoad("t0", "t1", 0).EndBlock();
  TestSequence seq = engine.Build();
  // Ensure the base register is masked using the branchless dynamic mask
  EXPECT_TRUE(absl::StrContains(seq.assembly_text(), "remu t6, t6, t2"));
  EXPECT_FALSE(absl::StrContains(seq.assembly_text(), "0x7FFF"))
      << "Found unsafe 0x7FFF mask";
}

TEST(IsgEngineTest, MemorySandboxRemuUsage) {
  IsgEngine engine(12345);
  engine.BeginMemoryBlock().EmitStore("t0", "t1", 1024).EndBlock();
  TestSequence seq = engine.Build();
  // Phase 3 requirement: Sandboxing logic uses remu
  EXPECT_TRUE(absl::StrContains(seq.assembly_text(), "remu t6, t6, t2"))
      << "Sandboxing logic does not use remu instruction!";
}

TEST(IsgEngineTest, MemorySandboxRemuT2Constraints) {
  IsgEngine engine(12345);
  engine.BeginMemoryBlock().EmitStore("t0", "t1", 1024).EndBlock();
  TestSequence seq = engine.Build();
  // Finding #103: Ensure t2 is constrained before remu to prevent hang.
  std::string text = std::string(seq.assembly_text());
  EXPECT_TRUE(absl::StrContains(text, "sltiu t1, t2, 1"));
  EXPECT_TRUE(absl::StrContains(text, "add t2, t2, t1"));
}

TEST(IsgEngineTest, MemorySandboxRegion1BaseAddress) {
  IsgEngine engine(12345);
  engine.EmitInstruction(".text");
  engine.EmitInstruction(".global _start");
  engine.EmitInstruction("_start:");

  engine.BeginMemoryBlock().EmitStore("t0", "t1", 1024).EndBlock();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  // Finding #102: Region 1 base address calculation should use lui t1, 0x10.
  EXPECT_TRUE(absl::StrContains(text, "lui t1, 0x10"))
      << "Region 1 base address calculation is incorrect!";

  // Compile to ELF
  const char* tmpdir = std::getenv("TEST_TMPDIR");
  std::string dir = tmpdir ? tmpdir : "/tmp";
  std::string asm_path =
      dir + "/test_base_addr_" + std::to_string(std::rand()) + ".s";
  std::string elf_path =
      dir + "/test_base_addr_" + std::to_string(std::rand()) + ".elf";

  std::ofstream out(asm_path);
  ASSERT_TRUE(out.is_open()) << "Failed to open assembly output file for test.";
  out << text;
  out.close();
  std::cout << "--- BASE ADDR TEST ASSEMBLY BEGIN ---\n"
            << text << "--- BASE ADDR TEST ASSEMBLY END ---\n"
            << std::endl;

  std::string cmd = "sim/coralnpu_m3_as " +
                    asm_path + " --output " + elf_path;
  int ret = std::system(cmd.c_str());
  ASSERT_EQ(ret, 0)
      << "Assembler failed to compile generated sequence. Command: " << cmd;

  // Run in simulator
  ::coralnpu::sim::CoralNPUSimulatorOptions options;
  options.architecture = ::coralnpu::sim::Architecture::kM3;
  ::coralnpu::sim::CoralNPUSimulator sim(options);
  ABSL_ASSERT_OK(sim.LoadProgram(elf_path));

  // Initialize registers
  ABSL_ASSERT_OK(sim.WriteRegister("sp", 0x17FF0));
  ABSL_ASSERT_OK(sim.WriteRegister("t0", 0x12345678));
  ABSL_ASSERT_OK(sim.WriteRegister("t1", 0x10000));

  // Run
  ABSL_ASSERT_OK(sim.Run());
  ABSL_ASSERT_OK(sim.Wait());

  // Verify memory at 0x10000 + 1024 = 0x10400
  uint32_t mem_val = 0;
  auto read_status = sim.ReadMemory(0x10400, &mem_val, 4);
  ABSL_ASSERT_OK(read_status);
  EXPECT_EQ(mem_val, 0x12345678);

  // Cleanup
  std::remove(asm_path.c_str());
  std::remove(elf_path.c_str());
}

TEST(IsgEngineTest, MemorySandboxEmitStoreCollision) {
  IsgEngine engine(12345);
  engine.EmitInstruction(".text");
  engine.EmitInstruction(".global _start");
  engine.EmitInstruction("_start:");

  // Finding #109: rs1 = t1, rs2 = t4. Old code used t4 as scratch, overwriting
  // rs2.
  engine.BeginMemoryBlock().EmitStore("t4", "t1", 0).EndBlock();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  // Should NOT use t4 as scratch for immediate loading.
  EXPECT_TRUE(absl::StrContains(text, "lui t2, ") ||
              absl::StrContains(text, "lui t3, "))
      << "EmitStore failed to use alternative scratch register! Generated: "
      << text;
  EXPECT_FALSE(absl::StrContains(text, "lui t4, "))
      << "EmitStore used t4 as scratch, potential collision!";

  // Compile to ELF
  const char* tmpdir = std::getenv("TEST_TMPDIR");
  std::string dir = tmpdir ? tmpdir : "/tmp";
  std::string asm_path =
      dir + "/test_collision_" + std::to_string(std::rand()) + ".s";
  std::string elf_path =
      dir + "/test_collision_" + std::to_string(std::rand()) + ".elf";

  std::ofstream out(asm_path);
  ASSERT_TRUE(out.is_open()) << "Failed to open assembly output file for test.";
  out << seq.assembly_text();
  out.close();

  std::string cmd = "sim/coralnpu_m3_as " +
                    asm_path + " --output " + elf_path;
  int ret = std::system(cmd.c_str());
  ASSERT_EQ(ret, 0)
      << "Assembler failed to compile generated sequence. Command: " << cmd;

  // Run in simulator
  ::coralnpu::sim::CoralNPUSimulatorOptions options;
  options.architecture = ::coralnpu::sim::Architecture::kM3;
  ::coralnpu::sim::CoralNPUSimulator sim(options);
  ABSL_ASSERT_OK(sim.LoadProgram(elf_path));

  // Initialize registers
  ABSL_ASSERT_OK(sim.WriteRegister("sp", 0x17FF0));
  ABSL_ASSERT_OK(sim.WriteRegister("t1", 0x10400));
  ABSL_ASSERT_OK(sim.WriteRegister("t4", 0xDEADBEEF));

  // Run
  ABSL_ASSERT_OK(sim.Run());
  ABSL_ASSERT_OK(sim.Wait());

  // Verify
  auto t4_val = sim.ReadRegister("t4");
  ABSL_ASSERT_OK(t4_val);
  EXPECT_EQ(*t4_val, 0xDEADBEEF);

  uint32_t mem_val = 0;
  auto read_status = sim.ReadMemory(0x10400, &mem_val, 4);
  ABSL_ASSERT_OK(read_status);
  EXPECT_EQ(mem_val, 0xDEADBEEF);

  // Cleanup
  std::remove(asm_path.c_str());
  std::remove(elf_path.c_str());
}

TEST(IsgEngineTest, MemorySandboxRemuFallbackAuthentic) {
  IsgEngine engine(12345);
  engine.EmitInstruction(".text");
  engine.EmitInstruction(".global _start");
  engine.EmitInstruction("_start:");

  // kDefaultRwRegionLength is 32768. The dynamic mask subtracts 4 - 1, so the
  // modulo is 32765. 39997 % 32765 = 7232. Address accessed should be 0x10000 +
  // 7232 = 0x11c40
  engine.BeginMemoryBlock().EmitLoad("t0", "t1", 39997).EndBlock();

  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  // Compile to ELF
  const char* tmpdir = std::getenv("TEST_TMPDIR");
  std::string dir = tmpdir ? tmpdir : "/tmp";
  std::string asm_path =
      dir + "/test_remu_fallback_" + std::to_string(std::rand()) + ".s";
  std::string elf_path =
      dir + "/test_remu_fallback_" + std::to_string(std::rand()) + ".elf";

  std::ofstream out(asm_path);
  ASSERT_TRUE(out.is_open()) << "Failed to open assembly output file for test.";
  out << text;
  out.close();
  std::cout << "--- GENERATED ASSEMBLY BEGIN ---\n"
            << text << "--- GENERATED ASSEMBLY END ---\n"
            << std::endl;

  std::string cmd = "sim/coralnpu_m3_as " +
                    asm_path + " --output " + elf_path;
  int ret = std::system(cmd.c_str());
  ASSERT_EQ(ret, 0) << "Assembler failed. Command: " << cmd << "\nCode:\n"
                    << text;

  // Run in simulator
  ::coralnpu::sim::CoralNPUSimulatorOptions options;
  options.architecture = ::coralnpu::sim::Architecture::kM3;
  ::coralnpu::sim::CoralNPUSimulator sim(options);
  ABSL_ASSERT_OK(sim.LoadProgram(elf_path));

  // Initialize registers
  ABSL_ASSERT_OK(sim.WriteRegister("sp", 0x17FF0));
  ABSL_ASSERT_OK(sim.WriteRegister("t1", 0x10000));

  // We need to initialize the region length register t2.
  // Wait, does EmitLoad initialize t2 or assume it's initialized?
  // Let's initialize t2 to 32768, just in case.
  ABSL_ASSERT_OK(sim.WriteRegister("t2", 32768));

  // Write the expected value at the modulo address.
  uint32_t expected_val = 0x8899AABB;
  ABSL_ASSERT_OK(sim.WriteMemory(0x11c40, &expected_val, 4));

  // Write a wrong value at the original un-modulo'd address.
  uint32_t wrong_val = 0xBADBAD;
  ABSL_ASSERT_OK(sim.WriteMemory(0x19c40, &wrong_val, 4));

  int step_count = 0;
  while (step_count < 300) {
    absl::StatusOr<uint64_t> pc_or = sim.ReadRegister("pc");
    uint32_t current_pc = pc_or.value();
    ::mpact::sim::generic::Instruction* inst =
        sim.decoder()->DecodeInstruction(current_pc);
    std::string disasm = inst ? inst->AsString() : "unknown";
    if (inst) inst->DecRef();

    absl::StatusOr<uint64_t> t1_or = sim.ReadRegister("t1");
    absl::StatusOr<uint64_t> t2_or = sim.ReadRegister("t2");
    absl::StatusOr<uint64_t> t5_or = sim.ReadRegister("t5");
    absl::StatusOr<uint64_t> t6_or = sim.ReadRegister("t6");
    absl::StatusOr<uint64_t> sp_or = sim.ReadRegister("sp");
    absl::StatusOr<uint64_t> t0_or = sim.ReadRegister("t0");
    uint64_t t1_val = t1_or.ok() ? t1_or.value() : 0;
    uint64_t t2_val = t2_or.ok() ? t2_or.value() : 0;
    uint64_t t5_val = t5_or.ok() ? t5_or.value() : 0;
    uint64_t t6_val = t6_or.ok() ? t6_or.value() : 0;
    uint64_t sp_val = sp_or.ok() ? sp_or.value() : 0;
    uint64_t t0_val = t0_or.ok() ? t0_or.value() : 0;

    std::cout << "[STEP DEBUG] Step " << std::dec << step_count << ": PC=0x"
              << std::hex << current_pc << " | t0=0x" << t0_val << " t1=0x"
              << t1_val << " t2=0x" << t2_val << " t5=0x" << t5_val << " t6=0x"
              << t6_val << " sp=0x" << sp_val << " | " << disasm << std::endl;

    auto step_res = sim.Step(1);
    if (!step_res.ok()) {
      std::cout << "[STEP DEBUG] Step failed with status: " << step_res.status()
                << std::endl;
      break;
    }
    step_count++;

    absl::StatusOr<uint32_t> halt_reason_or = sim.top()->GetLastHaltReason();
    if (halt_reason_or.ok() &&
        halt_reason_or.value() !=
            static_cast<uint32_t>(
                ::mpact::sim::generic::CoreDebugInterface::HaltReason::kNone)) {
      std::cout << "[STEP DEBUG] Reached terminal/halt state: "
                << halt_reason_or.value() << std::endl;
      break;
    }
  }

  ASSERT_OK_AND_ASSIGN(uint64_t result, sim.ReadRegister("t0"));
  EXPECT_EQ(result, expected_val)
      << "Did not load from the correctly sandboxed modulo address!";

  std::remove(asm_path.c_str());
  std::remove(elf_path.c_str());
}

TEST(IsgEngineTest, PrngSeedTruncationFixed) {
  // Test that 64-bit seed doesn't get truncated
  uint64_t seed1 = 0x1111222233334444ULL;
  uint64_t seed2 = 0x5555666633334444ULL;  // Same lower 32 bits
  IsgEngine engine1(seed1);
  IsgEngine engine2(seed2);
  EXPECT_NE(engine1.prng()(), engine2.prng()());
}

TEST(IsgEngineTest, MemorySandboxEmitLoadImmediateOverflowFixed) {
  IsgEngine engine(12345);
  // An immediate of 4096 cannot be encoded in a 12-bit signed immediate.
  engine.BeginMemoryBlock().EmitLoad("t0", "t1", 4096).EndBlock();
  TestSequence seq = engine.Build();
  // Ensure it doesn't just use `addi t6, t1, 4096`
  EXPECT_FALSE(absl::StrContains(seq.assembly_text(), "addi t6, t1, 4096"))
      << "Found unsafe immediate overflow in EmitLoad";
  EXPECT_TRUE(absl::StrContains(seq.assembly_text(), "lui t6"));
}

TEST(IsgEngineTest, MemorySandboxEmitVectorLoadStridedNoMutation) {
  IsgEngine engine(12345);
  engine.BeginMemoryBlock().EmitVectorLoadStrided("v0", "t1", "t2").EndBlock();
  TestSequence seq = engine.Build();
  // Ensure we don't mutate user's rs2: `andi t2, t2, 0x7`
  EXPECT_FALSE(absl::StrContains(seq.assembly_text(), "andi t2, t2, 0x7"))
      << "EmitVectorLoadStrided mutates user register in place!";
  EXPECT_TRUE(absl::StrContains(seq.assembly_text(), "andi t4, t2, 0x7"))
      << "Should use scratch register t4";
  EXPECT_TRUE(absl::StrContains(seq.assembly_text(), "remu t6, t6, t2"))
      << "Should use dynamic mask for safety";
}

TEST(IsgEngineTest, MemorySandboxEmitVectorLoadIndexedNoCollision) {
  IsgEngine engine(12345);
  engine.EmitVsetvli("t0", "t1", VectorSew::e32, VectorLmul::m8);
  // User vs2 is v24, temp vs2 could default to v31 (aligned to v24) or v15
  // (aligned to v8). If it aligns to v24, it clobbers user's vs2.
  engine.BeginMemoryBlock().EmitVectorLoadIndexed("v0", "t1", "v24").EndBlock();
  TestSequence seq = engine.Build();
  // It should use v8 or v16 instead of v24 for temp index clamping.
  EXPECT_FALSE(absl::StrContains(seq.assembly_text(), "vand.vx v24, v24"))
      << "EmitVectorLoadIndexed clobbers index register!";
}

TEST(IsgEngineTest, EmitInstructionIgnoresVset) {
  IsgEngine engine(12345);
  engine.EmitInstruction("vsetvli t2, t3, e8, m1, ta, ma");
  TestSequence seq = engine.Build();
  EXPECT_FALSE(
      absl::StrContains(seq.assembly_text(), "vsetvli t2, t3, e8, m1, ta, ma"));
}

TEST(IsgEngineTest, EmitInstructionAllowsVsetvliZeroZero) {
  IsgEngine engine(12345);
  engine.EmitInstruction("vsetvli zero, zero");
  engine.EmitInstruction("vadd.vv v0, v1, v2");
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());
  EXPECT_TRUE(absl::StrContains(text, "vsetvli zero, zero"));
  EXPECT_FALSE(absl::StrContains(text, "addi t3, zero, "))
      << "Fallback vsetvli was emitted even though vsetvli zero, zero should "
         "mark context as valid";
}

TEST(IsgEngineTest, EmitLoadImmediateOverflowFixed) {
  IsgEngine engine(12345);
  engine.BeginMemoryBlock().EmitLoad("t0", "t1", -2049).EndBlock();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());
  EXPECT_FALSE(absl::StrContains(text, "addi t6, t1, -2049"))
      << "Found simplistic addi which overflows";
  EXPECT_TRUE(absl::StrContains(text, "lui t6, "));
  EXPECT_TRUE(absl::StrContains(text, "add t6, t1, t6"));
}

TEST(IsgEngineTest, EmitVectorLoadStridedClobberingFixed) {
  IsgEngine engine(12345);
  engine.BeginMemoryBlock().EmitVectorLoadStrided("v0", "t1", "t2").EndBlock();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());
  EXPECT_FALSE(absl::StrContains(text, "andi t2, t2, 0x7"))
      << "Found mutation of rs2";
  EXPECT_TRUE(absl::StrContains(text, "andi t4, t2, 0x7"));
  EXPECT_TRUE(absl::StrContains(text, "vlse32.v v0, (t6), t4"));
}

TEST(IsgEngineTest, MemorySandboxRegion2MaskCorrectlySized) {
  IsgEngine engine(12345);
  engine.BeginMemoryBlock().EmitLoad("t0", "t1", 0).EndBlock();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());
  EXPECT_TRUE(absl::StrContains(text, "lui t1, 0x400"))
      << "Expected 4MB Region 2 mask to align with memory_config.h";
}

TEST(IsgEngineTest, MemorySandboxRegion2MaskExactBoundary) {
  IsgEngine engine(12345);
  // EmitLoad uses offset_size = 4
  engine.BeginMemoryBlock().EmitLoad("t0", "t1", 0).EndBlock();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());
  // r2_mask = 0x400000 - 4 + 1 = 0x3FFFFD
  // lui t1, 0x400
  // addi t1, t1, -3
  EXPECT_TRUE(absl::StrContains(text, "lui t1, 0x400"));
  EXPECT_TRUE(absl::StrContains(text, "addi t1, t1, -3"))
      << "Expected exact boundary constant for r2_mask (0x3FFFFD)";
}

TEST(IsgEngineTest, MemorySandboxEmitVectorLoadIndexedThreeWayCollision) {
  IsgEngine engine(12345);
  engine.EmitVsetvli("t0", "t1", VectorSew::e32, VectorLmul::m8);
  // vd = v8, vs2 = v24. For m8:
  // v31 aligns to v24 (collides with vs2)
  // v15 aligns to v8 (collides with vd)
  // We need to make sure the clamping temp register isn't v8 or v24!
  engine.BeginMemoryBlock().EmitVectorLoadIndexed("v8", "t1", "v24").EndBlock();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());
  EXPECT_FALSE(absl::StrContains(text, "vand.vx v8, ")) << "Clobbers vd!";
  EXPECT_FALSE(absl::StrContains(text, "vand.vx v24, ")) << "Clobbers vs2!";
  EXPECT_TRUE(absl::StrContains(text, "vand.vx v16, "))
      << "Should use v16 as third fallback!";
}

TEST(IsgEngineTest, CanonicalizeMnemonicVsetvliFixed) {
  EXPECT_EQ(CanonicalizeMnemonic("vsetvli"), "vsetvli")
      << "CanonicalizeMnemonic incorrectly appended .v to vsetvli";
  EXPECT_EQ(CanonicalizeMnemonic("vsetivli"), "vsetivli")
      << "CanonicalizeMnemonic incorrectly appended .v to vsetivli";
  EXPECT_EQ(CanonicalizeMnemonic("vse8"), "vse8.v");
  EXPECT_EQ(CanonicalizeMnemonic("vse8.v"), "vse8.v");
  EXPECT_EQ(CanonicalizeMnemonic("vaddvv"), "vadd.vv");
  EXPECT_EQ(CanonicalizeMnemonic("vadd.vv"), "vadd.vv");
  EXPECT_EQ(CanonicalizeMnemonic("vluxei8"), "vluxei8.v");
  EXPECT_EQ(CanonicalizeMnemonic("vsuxei8"), "vsuxei8.v");
  EXPECT_EQ(CanonicalizeMnemonic("vle8_vm1"), "vle8.v");
  EXPECT_EQ(CanonicalizeMnemonic("csrrs_nr"), "csrs");
  EXPECT_EQ(CanonicalizeMnemonic("csrrwnr"), "csrw");
  EXPECT_EQ(CanonicalizeMnemonic("csrrwi_nr"), "csrwi");
  EXPECT_EQ(CanonicalizeMnemonic("vsetvl"), "vsetvl");
  EXPECT_EQ(CanonicalizeMnemonic("vmadcvxm"), "vmadc.vxm");
}

TEST(IsgEngineTest, PreambleDoesNotClobberT2T3) {
  IsgEngine engine(12345);
  engine.EmitPreamble();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  // Verify that t2 and t3 are re-initialized after Vector setup.
  // The preamble re-initializes them to kDefaultRwRegionStart + offset.
  // We can just verify the text contains multiple initializations for t2/t3.
  size_t vsetvli_pos = text.find("vsetvli");
  EXPECT_NE(vsetvli_pos, std::string::npos) << "Preamble must contain vsetvli";
  EXPECT_TRUE(absl::StrContains(text.substr(vsetvli_pos), "t2"))
      << "t2 not re-initialized after vsetvli";
  EXPECT_TRUE(absl::StrContains(text.substr(vsetvli_pos), "t3"))
      << "t3 not re-initialized after vsetvli";
}

TEST(IsgEngineTest, VectorMaskAndTailPoliciesAreSupported) {
  IsgEngine engine(12345);
  engine.EmitVsetvli("t2", "t3", VectorSew::e8, VectorLmul::m1, false, false);
  engine.BeginVectorBlock().EmitVadd("v0", "v1", "v2", true).EndBlock();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());
  EXPECT_TRUE(absl::StrContains(text, "vsetvli t2, t3, e8, m1, tu, mu"));
  EXPECT_TRUE(absl::StrContains(text, "vadd.vv v0, v1, v2, v0.t"));
}

TEST(IsgEngineTest, VectorPreambleInitializesWithLegalLmul) {
  IsgEngine engine(12345);
  engine.EmitPreamble();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());
  EXPECT_TRUE(absl::StrContains(text, "vsetvli t2, t3, e8, m8"))
      << "Preamble must initialize vectors with m8 to be legal";
}

TEST(IsgEngineTest, MemorySandboxEmitVectorLoadStridedEmulM4) {
  IsgEngine engine(12345);
  // sew=e8, lmul=m1. eew=32. emul_ratio = (32/8) * 1 = 4.0 -> m4.
  // v2 aligned to m4 is v0.
  engine.EmitVsetvli("t0", "t1", VectorSew::e8, VectorLmul::m1);
  engine.BeginMemoryBlock().EmitVectorLoadStrided("v2", "t1", "t2").EndBlock();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());
  EXPECT_TRUE(absl::StrContains(text, "vlse32.v v0, "))
      << "Expected vd to be aligned to v0 for m4";
  EXPECT_FALSE(absl::StrContains(text, "vlse32.v v2, "))
      << "vd was not aligned properly for m4!";
}

TEST(IsgEngineTest, PreambleMemoryBoundaryInitialization) {
  IsgEngine engine(12345);
  engine.EmitPreamble();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  EXPECT_TRUE(absl::StrContains(text, "lui t2, 0x2"));
  EXPECT_FALSE(absl::StrContains(text, "lui t2, 0x2\naddi t2, t2, -1"))
      << "Memory boundary mutated! Expected exact 8192 words without addi "
         "decrement before loop.";
}

TEST(IsgEngineTest, PreambleDoesNotInitializeZeroRegister) {
  IsgEngine engine(12345);
  engine.EmitPreamble();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  std::vector<std::string> lines =
      absl::StrSplit(text, '\n', absl::SkipEmpty());
  for (absl::string_view line : lines) {
    if (line.find("addi x0, x0, 0") != std::string::npos ||
        line.find("addi zero, zero, 0") != std::string::npos) {
      // Padding is allowed.
      continue;
    }

    std::vector<std::string> tokens =
        absl::StrSplit(line, absl::ByAnyChar(" \t,"), absl::SkipWhitespace());
    if (tokens.size() > 1) {
      std::string mnemonic = tokens[0];
      std::string op1 = tokens[1];

      // Store and branch instructions do not write to the first operand.
      bool writes_to_op1 = true;
      if (mnemonic == "sw" || mnemonic == "sh" || mnemonic == "sb" ||
          mnemonic == "beq" || mnemonic == "bne" || mnemonic == "blt" ||
          mnemonic == "bge" || mnemonic == "bltu" || mnemonic == "bgeu" ||
          mnemonic == "j" || mnemonic == "jr" || mnemonic == "vse8.v" ||
          mnemonic == "vse16.v" || mnemonic == "vse32.v") {
        writes_to_op1 = false;
      }

      // jal, csrrw, and csrw can legitimately write to zero (discarding the
      // result).
      if (mnemonic == "jal" || mnemonic == "csrrw" || mnemonic == "csrw") {
        writes_to_op1 = false;
      }

      if (writes_to_op1 && (op1 == "x0" || op1 == "zero")) {
        ADD_FAILURE() << "Preamble initializes the zero register (" << op1
                      << "): " << line;
      }
    }
  }
}

TEST(IsgEngineTest, EmitDataHazardEmitsInstructions) {
  IsgEngine engine(12345);
  engine.EmitDataHazard(::coralnpu::sim::isa32_m3::OpcodeEnum::kAdd,
                        ::coralnpu::sim::isa32_m3::OpcodeEnum::kSub);
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  // Verify that a read-after-write hazard on register 't0' is generated.
  EXPECT_TRUE(absl::StrContains(text, "add t0, t1, t2"));
  EXPECT_TRUE(absl::StrContains(text, "sub t3, t0, t4"));
}

TEST(IsgEngineTest, EmitVtypeEmitsVsetvli) {
  IsgEngine engine(12345);
  engine.EmitVtype(32, 4);
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  EXPECT_TRUE(absl::StrContains(text, "vsetvli zero, zero, e32, m4, ta, ma"));
  EXPECT_EQ(engine.vector_state().sew, VectorSew::e32);
  EXPECT_EQ(engine.vector_state().lmul, VectorLmul::m4);
}

TEST(IsgEngineTest, EmitVtypeTracksVl) {
  IsgEngine engine(12345);
  engine.EmitVtype(32, 4, 16);
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  EXPECT_TRUE(absl::StrContains(text, "addi t3, t3, 16"));
  EXPECT_TRUE(absl::StrContains(text, "vsetvli zero, t3, e32, m4, ta, ma"));
  EXPECT_EQ(engine.vector_state().vl, 16);
}

TEST(IsgEngineTest, FinalizeAddsTestSequence) {
  IsgEngine engine(12345);
  engine.EmitInstruction("add t0, t1, t2");
  ::coralnpu::sim::proto::Database db;
  engine.Finalize(&db);
  EXPECT_EQ(db.test_sequences_size(), 1);
  EXPECT_EQ(db.test_sequences(0).prng_seed(), 12345);
  EXPECT_TRUE(absl::StrContains(db.test_sequences(0).assembly_text(),
                                "add t0, t1, t2"));
}

TEST(IsgEngineTest, WatchdogCatchesInfiniteLoops) {
  EXPECT_DEATH(
      {
        IsgEngine engine(12345);
        for (int i = 0; i <= 1000000; ++i) {
          engine.EmitInstruction("add t0, t1, t2");
        }
      },
      "IsgWatchdog exceeded maximum iteration limit");
}

TEST(IsgEngineTest, VectorRegisterModulo32Safety) {
  // Test that register numbers are always modulo 32, even when alignment bypass
  // is triggered.
  for (int i = 0; i < 200; ++i) {
    IsgEngine inner_engine(i);
    inner_engine.EmitVsetvli("t2", "t3", VectorSew::e8, VectorLmul::m1);
    inner_engine.BeginVectorBlock().EmitVadd("v35", "v1", "v2").EndBlock();
    TestSequence seq = inner_engine.Build();
    std::string text = std::string(seq.assembly_text());
    EXPECT_FALSE(absl::StrContains(text, "v35"))
        << "Found unmapped register v35 on iteration " << i;
    EXPECT_TRUE(absl::StrContains(text, "v0") || absl::StrContains(text, "v3"))
        << "Expected v35 to be mapped to v0 or v3 on iteration " << i;
  }
}

TEST(IsgEngineTest, VectorRegisterModulo32Align) {
  IsgEngine engine(12345);
  engine.EmitVsetvli("t2", "t3", VectorSew::e8, VectorLmul::m8);
  // v35 with m8 align -> 35 & ~7 = 32. Modulo 32 -> 0. Should be v0.
  engine.BeginVectorBlock().EmitVadd("v35", "v8", "v16").EndBlock();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());
  EXPECT_TRUE(absl::StrContains(text, "vadd.vv v0, v8, v16"))
      << "Expected v35 to be mapped to v0 via align and modulo 32";
  EXPECT_FALSE(absl::StrContains(text, "v35"));
  EXPECT_FALSE(absl::StrContains(text, "v32"));
}

TEST(IsgEngineTest, VectorContextSetupFallthrough) {
  IsgEngine engine(12345);
  engine.EmitVtype(16, 2);
  engine.EmitInstruction("vadd.vv v0, v1, v2");
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());
  // It should NOT emit fallback because context was set by EmitVtype
  EXPECT_FALSE(absl::StrContains(text, "addi t3, zero, "))
      << "EmitVtype should establish valid context and prevent fallback";
}

TEST(IsgEngineTest, PadsItcmToExactly8kBWithMpause) {
  IsgEngine engine(12345);
  engine.EmitPreamble();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  EXPECT_TRUE(absl::EndsWith(std::string(absl::StripAsciiWhitespace(text)),
                             ".word 0x08000073\n; END PROTECTED"));

  uint32_t pc = 0;
  std::vector<std::string> lines =
      absl::StrSplit(text, '\n', absl::SkipEmpty());
  for (absl::string_view line : lines) {
    std::string trimmed = std::string(absl::StripAsciiWhitespace(line));
    if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';' ||
        trimmed.back() == ':')
      continue;
    if (trimmed[0] == '.') {
      if (absl::StartsWith(trimmed, ".word")) {
        pc += 4;
      }
      continue;
    }
    pc += 4;
  }
  EXPECT_EQ(pc, 8192) << "ITCM not padded to exactly 8192 bytes";
}

TEST(IsgEngineTest, TrapHandlerEmittedInPreamble) {
  IsgEngine engine(12345);
  engine.EmitPreamble();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  // Verify mtvec is set to point to the trap handler
  EXPECT_TRUE(absl::StrContains(text, "csrw mtvec, "));

  // Verify trap handler logs mcause and mepc and saves registers
  EXPECT_TRUE(absl::StrContains(text, "csrw mscratch, t0"));
  EXPECT_TRUE(absl::StrContains(text, "csrrs t1, mcause, zero"));
  EXPECT_TRUE(absl::StrContains(text, "csrrs t1, mepc, zero"));
  EXPECT_TRUE(absl::StrContains(text, "sw t1, 12(t0)"));
  EXPECT_TRUE(absl::StrContains(text, "sw t1, 16(t0)"));

  // Verify trap handler returns
  EXPECT_TRUE(absl::StrContains(text, "mret"));
}

TEST(IsgEngineTest, IntentionalTrapGeneration) {
  IsgEngine engine(12345);
  engine.EmitPreamble();
  engine.EmitEbreak();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  // Verify an intentional trap instruction is emitted
  EXPECT_TRUE(absl::StrContains(text, "ebreak"));
}

TEST(IsgEngineTest, IllegalInstructionEmission) {
  IsgEngine engine(12345);
  engine.EmitPreamble();
  engine.EmitIllegalInstruction();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());
  EXPECT_TRUE(absl::StrContains(text, ".word 0x00000000"));
}

TEST(IsgEngineTest, TrapHandlerPreservesMscratch) {
  IsgEngine engine(12345);
  engine.EmitPreamble();
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  // Verify trap handler saves (csrrw t0, mscratch, t0) and restores mscratch
  // (e.g., csrw mscratch, t1)
  EXPECT_TRUE(absl::StrContains(text, "csrrw t0, mscratch, t0"));
  EXPECT_TRUE(absl::StrContains(text, "csrw mscratch, t1"))
      << "Trap handler does not restore mscratch!";
}

TEST(IsgEngineTest, DatabaseSerializationIsDeterministic) {
  ::coralnpu::sim::proto::Database db;
  db.set_master_seed(42);
  auto* seq = db.add_test_sequences();
  (*seq->mutable_terminal_state()->mutable_registers())["x1"] = 1;
  (*seq->mutable_terminal_state()->mutable_registers())["x2"] = 2;
  (*seq->mutable_terminal_state()->mutable_registers())["x3"] = 3;
  (*seq->mutable_terminal_state()->mutable_registers())["x4"] = 4;
  (*seq->mutable_terminal_state()->mutable_registers())["x5"] = 5;

  std::string out1;
  {
    ::google::protobuf::io::StringOutputStream raw_out(&out1);
    ::google::protobuf::io::CodedOutputStream coded_out(&raw_out);
    coded_out.SetSerializationDeterministic(true);
    db.SerializeToCodedStream(&coded_out);
  }

  std::string out2;
  {
    // Rebuild db to potentially change map iteration order
    ::coralnpu::sim::proto::Database db2;
    db2.set_master_seed(42);
    auto* seq2 = db2.add_test_sequences();
    (*seq2->mutable_terminal_state()->mutable_registers())["x5"] = 5;
    (*seq2->mutable_terminal_state()->mutable_registers())["x4"] = 4;
    (*seq2->mutable_terminal_state()->mutable_registers())["x3"] = 3;
    (*seq2->mutable_terminal_state()->mutable_registers())["x2"] = 2;
    (*seq2->mutable_terminal_state()->mutable_registers())["x1"] = 1;

    ::google::protobuf::io::StringOutputStream raw_out(&out2);
    ::google::protobuf::io::CodedOutputStream coded_out(&raw_out);
    coded_out.SetSerializationDeterministic(true);
    db2.SerializeToCodedStream(&coded_out);
  }

  EXPECT_EQ(out1, out2);
}

TEST(IsgEngineTest, EmitFpPoisonPillsOutputsSpacedDisassembly) {
  IsgEngine engine(123);
  engine.EmitFpPoisonPills();
  std::string text;
  ::coralnpu::sim::proto::Database db;
  engine.Finalize(&db, &text);
  EXPECT_TRUE(absl::StrContains(text, "fadd.s f7, f1, f2"));
  EXPECT_TRUE(absl::StrContains(text, "fsub.s f8, f2, f3"));
  EXPECT_TRUE(absl::StrContains(text, "fmul.s f9, f1, f3"));
  EXPECT_TRUE(absl::StrContains(text, "fdiv.s f10, f1, f1"));
}

}  // namespace
}  // namespace fuzzer
}  // namespace coralnpu
