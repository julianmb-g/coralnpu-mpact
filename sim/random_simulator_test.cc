#include "sim/random_simulator.h"

#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"

namespace coralnpu {
namespace fuzzer {
namespace {

TEST(RandomSimulatorTest, NormalizeDisassemblyStripsTrailingCommasAndSpaces) {
  // Finding #127: Verify robust trailing comma and space stripping
  EXPECT_EQ(NormalizeDisassembly("vluxseg8 ei2.v v29, (s6), v13, v0, "),
            "vluxseg8 ei2.v v29, (s6), v13, v0");
  EXPECT_EQ(NormalizeDisassembly("vluxseg8 ei2.v v29, (s6), v13, v0, , ,,   "),
            "vluxseg8 ei2.v v29, (s6), v13, v0");
  EXPECT_EQ(NormalizeDisassembly("vadd.vv v1, v2, v3"), "vadd.vv v1, v2, v3");
}

}  // namespace
}  // namespace fuzzer
}  // namespace coralnpu
