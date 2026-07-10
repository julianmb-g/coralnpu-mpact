#include "sim/isg/hazard_generator.h"

#include <cstdio>
#include <cstring>
#include <iostream>

#include "sim/isg/isg_engine.h"
#include "googletest/include/gtest/gtest.h"
#include "absl/strings/match.h"
#include "absl/strings/str_format.h"

namespace coralnpu {
namespace fuzzer {
namespace {

TEST(HazardGeneratorTest, GeneratesDataHazard) {
  IsgEngine engine(42);
  GenerateDataHazard(engine);
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  int reg_write, reg_read1, reg_read2;
  const char* c_str = text.c_str();

  const char* vadd_ptr = strstr(c_str, "vadd.vv ");
  ASSERT_NE(vadd_ptr, nullptr);
  EXPECT_EQ(sscanf(vadd_ptr, "vadd.vv v%d, v%d, v%d", &reg_write, &reg_read1,
                   &reg_read2),
            3);

  int sub_dest, sub_src1, sub_src2;
  const char* vsub_ptr = strstr(c_str, "vsub.vv ");
  ASSERT_NE(vsub_ptr, nullptr);
  EXPECT_EQ(sscanf(vsub_ptr, "vsub.vv v%d, v%d, v%d", &sub_dest, &sub_src1,
                   &sub_src2),
            3);

  EXPECT_EQ(sub_src1, reg_write)
      << "Data hazard requires vsub to read the register written by vadd";
  EXPECT_EQ(sub_src2, reg_read1);

  std::string in_between(vadd_ptr, vsub_ptr - vadd_ptr);
  uint32_t nop_count = 0;
  size_t pos = 0;
  while ((pos = in_between.find("addi x0, x0, 0", pos)) != std::string::npos) {
    nop_count++;
    pos += 14;
  }
  EXPECT_LT(nop_count, 5)
      << "Data hazard padding distance exceeds configured limits";
}

TEST(HazardGeneratorTest, GeneratesControlHazard) {
  IsgEngine engine(42);
  GenerateControlHazard(engine);
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  size_t beq_pos = text.find("beq t0, t1");
  size_t bne_pos = text.find("bne t0, t1");
  ASSERT_NE(beq_pos, std::string::npos);
  ASSERT_NE(bne_pos, std::string::npos);

  std::string in_between = text.substr(beq_pos, bne_pos - beq_pos);
  uint32_t nop_count = 0;
  size_t pos = 0;
  while ((pos = in_between.find("addi x0, x0, 0", pos)) != std::string::npos) {
    nop_count++;
    pos += 14;
  }

  EXPECT_GT(nop_count, 0)
      << "Expected at least one NOP in control hazard loop body";

  int beq_pc = 8;
  int expected_fwd = nop_count * 4 + 12;
  int expected_fwd_abs = beq_pc + expected_fwd;

  std::string expected_beq =
      absl::StrFormat("beq t0, t1, 0x%x", expected_fwd_abs);
  std::string expected_bne = absl::StrFormat("bne t0, t1, 0x%x", beq_pc);

  EXPECT_TRUE(absl::StrContains(text, expected_beq))
      << "Expected forward branch offset to be " << expected_fwd;
  EXPECT_TRUE(absl::StrContains(text, expected_bne))
      << "Expected backward branch target to be " << beq_pc;
}

TEST(HazardGeneratorTest, ControlHazardNoInfiniteLoop) {
  IsgEngine engine(12345);
  GenerateControlHazard(engine);
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());
  EXPECT_TRUE(absl::StrContains(text, "bne t0, t1"))
      << "Control hazard must use bne to break out of infinite loop";
  EXPECT_TRUE(absl::StrContains(text, "addi t0, t0, -1"))
      << "Control hazard must decrement loop counter to avoid infinite loop";
  EXPECT_TRUE(absl::StrContains(text, "beq t0, t1"))
      << "Control hazard must evaluate loop exit";
}

TEST(HazardGeneratorTest, GeneratesStructuralHazard) {
  IsgEngine engine(42);
  GenerateStructuralHazard(engine);
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  int vmul_vd, vmul_vs2, vmul_vs1;
  const char* vmul_ptr = strstr(text.c_str(), "vmul.vv ");
  ASSERT_NE(vmul_ptr, nullptr);
  ASSERT_EQ(
      sscanf(vmul_ptr, "vmul.vv v%d, v%d, v%d", &vmul_vd, &vmul_vs2, &vmul_vs1),
      3);

  int vdiv_vd, vdiv_vs2, vdiv_vs1;
  const char* vdiv_ptr = strstr(text.c_str(), "vdiv.vv ");
  ASSERT_NE(vdiv_ptr, nullptr);
  // Note: vdiv.vv has vd, vs2, vs1 format. In GenerateStructuralHazard it is:
  // vdiv.vv v[vs2], v[vs2_div], v[vd]
  // So vs2 is dest, vs2_div is src2, vd is src1.
  // Wait, AsString() format for vdiv.vv might be different.
  // Let's check how AsString() formatting works.
  // Usually it is: mnemonic dest, src1, src2 (or dest, src2, src1 depending on
  // ISA). In riscv spec: vdiv.vv vd, vs2, vs1 (vd = vs2 / vs1). So AsString()
  // should be "vdiv.vv vd, vs2, vs1". In GenerateStructuralHazard:
  // block.EmitVdiv(vs2, vs2_div, vd)
  // If EmitVdiv follows (dest, src2, src1) or (dest, src1, src2)?
  // Let's check block.EmitVdiv signature or implementation.
  // Actually, we can check the previous expectation:
  // "vdiv.vv v16, v0, v8"
  // Here vd=16 (vs2), vs2=0 (vs2_div), vs1=8 (vd).
  // So it is: vdiv.vv vd, vs2, vs1.
  // Thus sscanf should match this format.
  ASSERT_EQ(
      sscanf(vdiv_ptr, "vdiv.vv v%d, v%d, v%d", &vdiv_vd, &vdiv_vs2, &vdiv_vs1),
      3);

  // Verify alignment to 8
  EXPECT_EQ(vmul_vd % 8, 0);
  EXPECT_EQ(vmul_vs2 % 8, 0);
  EXPECT_EQ(vmul_vs1 % 8, 0);
  EXPECT_EQ(vdiv_vd % 8, 0);
  EXPECT_EQ(vdiv_vs2 % 8, 0);
  EXPECT_EQ(vdiv_vs1 % 8, 0);

  // Verify structural hazard relation
  EXPECT_EQ(vdiv_vd, vmul_vs2);
  EXPECT_EQ(vdiv_vs1, vmul_vd);
}

TEST(HazardGeneratorTest, GeneratesEdgeCaseOperands) {
  IsgEngine engine(42);
  GenerateEdgeCaseOperands(engine);
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  EXPECT_TRUE(absl::StrContains(text, "addi t0, zero, -1"));
  EXPECT_TRUE(absl::StrContains(text, "lui t2, 0x80000"));
  EXPECT_TRUE(absl::StrContains(text, "div t4, t2, t0"));
  EXPECT_TRUE(absl::StrContains(text, "lw t6, 0(t6)"));
  EXPECT_TRUE(absl::StrContains(text, "div t4, t2, t1"));
}

TEST(HazardGeneratorTest, GeneratesRandomInstructions) {
  IsgEngine engine(42);
  GenerateRandomInstructions(engine);
  TestSequence seq = engine.Build();
  std::string text = std::string(seq.assembly_text());

  // Count the number of non-empty, non-comment lines
  int valid_lines = 0;
  size_t pos = 0;
  while (pos < text.length()) {
    size_t next_pos = text.find('\n', pos);
    std::string line;
    if (next_pos == std::string::npos) {
      line = text.substr(pos);
      pos = text.length();
    } else {
      line = text.substr(pos, next_pos - pos);
      pos = next_pos + 1;
    }

    std::string trimmed = std::string(absl::StripAsciiWhitespace(line));
    if (!trimmed.empty() && trimmed[0] != '#' && trimmed[0] != ';' &&
        trimmed.back() != ':' && trimmed[0] != '.') {
      valid_lines++;
    }
  }

  EXPECT_GT(valid_lines, 1000)
      << "Expected more than 1000 generated instructions";
}

}  // namespace
}  // namespace fuzzer
}  // namespace coralnpu
