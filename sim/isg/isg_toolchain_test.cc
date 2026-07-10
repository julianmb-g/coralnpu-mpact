#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "sim/isg/isg_engine.h"
#include "googletest/include/gtest/gtest.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"

namespace coralnpu {
namespace fuzzer {
namespace {

std::string FindRiscvAs() {
  // 1. Check if riscv64g_as is on the system PATH
  if (std::system("command -v riscv64g_as > /dev/null 2>&1") == 0) {
    return "riscv64g_as";
  }

  // 2. Check if TEST_SRCDIR is defined (Bazel/Blaze)
  const char* srcdir = std::getenv("TEST_SRCDIR");
  if (srcdir != nullptr) {
    std::string base_dir(srcdir);
    try {
      if (std::filesystem::exists(base_dir)) {
        for (const auto& entry :
             std::filesystem::recursive_directory_iterator(base_dir)) {
          if (entry.is_regular_file() &&
              entry.path().filename() == "riscv64g_as") {
            return entry.path().string();
          }
        }
      }
    } catch (...) {
      // Ignore filesystem errors and fallback.
    }
  }

  // 3. Fallback to current directory or relative paths
  std::vector<std::string> relative_paths = {
      "./third_party/mpact_riscv/riscv64g_as",
      "../mpact-riscv/riscv/riscv64g_as",
      "external/mpact-riscv/riscv/riscv64g_as",
      "external/com_google_mpact-riscv/riscv/riscv64g_as",
  };
  for (const auto& path : relative_paths) {
    if (std::ifstream(path).good()) {
      return path;
    }
  }

  return "";
}

TEST(IsgToolchainTest, AssemblyCompatibleWithGcc) {
  IsgEngine engine(12345);
  engine.EmitInstruction("addw t0, t1, t2");
  engine.EmitInstruction("subw t3, t4, t5");
  engine.EmitInstruction("addw s0, s1, s2");
  TestSequence seq = engine.Build();

  const char* tmpdir = std::getenv("TEST_TMPDIR");
  std::string dir = tmpdir ? tmpdir : "/tmp";
  std::string asm_path = dir + "/toolchain_test.s";
  std::string obj_path = dir + "/toolchain_test.o";

  std::string asm_text(seq.assembly_text());
  size_t end_pos = asm_text.find("_mutable_end:");
  if (end_pos != std::string::npos) {
    asm_text = asm_text.substr(0, end_pos);
  }

  std::ofstream out(asm_path);
  out << ".text\n" << asm_text;
  out.close();

  std::string riscv_as = FindRiscvAs();
  ASSERT_FALSE(riscv_as.empty())
      << "riscv64g_as is not available in the current environment path "
         "or Bazel runfiles.";

  // Invoke standard GCC toolchain (either from path or from Bazel runfiles)
  std::string cmd;
  if (riscv_as.find("riscv64g_as") != std::string::npos) {
    cmd = absl::StrCat(riscv_as, " -c -o ", obj_path, " ", asm_path);
  } else {
    cmd = absl::StrCat(riscv_as, " -march=rv32imf_zve32f_zicsr_zifencei_zbb ",
                       asm_path, " -o ", obj_path);
  }

  int ret = std::system(cmd.c_str());
  EXPECT_EQ(ret, 0)
      << "Standard GCC toolchain failed to assemble ISG output. Command: "
      << cmd;
}

}  // namespace
}  // namespace fuzzer
}  // namespace coralnpu
