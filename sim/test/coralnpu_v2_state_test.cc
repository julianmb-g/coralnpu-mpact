#include "sim/coralnpu_v2_state.h"

#include <memory>

#include "googletest/include/gtest/gtest.h"
#include "googlemock/include/gmock/gmock.h"
#ifndef ABSL_EXPECT_OK
#define ABSL_EXPECT_OK(x) EXPECT_TRUE((x).ok())
#endif
#ifndef ABSL_ASSERT_OK
#define ABSL_ASSERT_OK(x) ASSERT_TRUE((x).ok())
#endif
#ifndef EXPECT_OK
#define EXPECT_OK(x) EXPECT_TRUE((x).ok())
#endif
#ifndef ASSERT_OK
#define ASSERT_OK(x) ASSERT_TRUE((x).ok())
#endif
#ifndef KELVIN_TEST_MATCHERS_DEFINED
#define KELVIN_TEST_MATCHERS_DEFINED
namespace absl_testing {
MATCHER(IsOk, "") { return arg.ok(); }
template <typename M>
inline auto IsOkAndHolds(M matcher) {
  return ::testing::AllOf(
      ::testing::ResultOf([](const auto& s) { return s.ok(); }, ::testing::IsTrue()),
      ::testing::ResultOf([](const auto& s) -> const auto& { return *s; }, matcher));
}
}  // namespace absl_testing
namespace testing::status {
using ::absl_testing::IsOk;
using ::absl_testing::IsOkAndHolds;
}  // namespace testing::status
#endif
#include "riscv/riscv_state.h"
#include "mpact/sim/util/memory/flat_demand_memory.h"

namespace coralnpu::sim {
namespace {

using ::mpact::sim::riscv::RiscVXlen;
using ::mpact::sim::util::FlatDemandMemory;

TEST(CoralNPUV2StateTest, HasPermission) {
  auto memory = std::make_unique<FlatDemandMemory>();
  auto state = CreateCoralNPUV2State("test", RiscVXlen::RV32, memory.get());

  // Add a read-only region.
  state->AddMemoryRegion(0x1000, 0x1000, MemoryPermission::kRead);
  // Add a write-only region.
  state->AddMemoryRegion(0x2000, 0x1000, MemoryPermission::kWrite);
  // Add an execute-only region.
  state->AddMemoryRegion(0x3000, 0x1000, MemoryPermission::kExecute);
  // Add a read-write region.
  state->AddMemoryRegion(0x4000, 0x1000,
                         MemoryPermission::kRead | MemoryPermission::kWrite);

  // Check read-only region.
  EXPECT_TRUE(state->HasPermission(0x1000, 4, MemoryPermission::kRead));
  EXPECT_FALSE(state->HasPermission(0x1000, 4, MemoryPermission::kWrite));
  EXPECT_FALSE(state->HasPermission(0x1000, 4, MemoryPermission::kExecute));

  // Check write-only region.
  EXPECT_FALSE(state->HasPermission(0x2000, 4, MemoryPermission::kRead));
  EXPECT_TRUE(state->HasPermission(0x2000, 4, MemoryPermission::kWrite));
  EXPECT_FALSE(state->HasPermission(0x2000, 4, MemoryPermission::kExecute));

  // Check execute-only region.
  EXPECT_FALSE(state->HasPermission(0x3000, 4, MemoryPermission::kRead));
  EXPECT_FALSE(state->HasPermission(0x3000, 4, MemoryPermission::kWrite));
  EXPECT_TRUE(state->HasPermission(0x3000, 4, MemoryPermission::kExecute));

  // Check read-write region.
  EXPECT_TRUE(state->HasPermission(0x4000, 4, MemoryPermission::kRead));
  EXPECT_TRUE(state->HasPermission(0x4000, 4, MemoryPermission::kWrite));
  EXPECT_FALSE(state->HasPermission(0x4000, 4, MemoryPermission::kExecute));
  EXPECT_TRUE(state->HasPermission(
      0x4000, 4, MemoryPermission::kRead | MemoryPermission::kWrite));

  // Check out of bounds.
  EXPECT_FALSE(state->HasPermission(0x5000, 4, MemoryPermission::kRead));

  // Add two adjacent regions with the same permissions.
  state->AddMemoryRegion(0x6000, 0x1000,
                         MemoryPermission::kRead | MemoryPermission::kWrite);
  state->AddMemoryRegion(0x7000, 0x1000,
                         MemoryPermission::kRead | MemoryPermission::kWrite);

  // Check spanning multiple regions (should fail even if both have
  // permissions).
  EXPECT_FALSE(state->HasPermission(0x6FFC, 8, MemoryPermission::kRead));
}

}  // namespace
}  // namespace coralnpu::sim
