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

#include "sim/isg/coverage_event_router.h"

#include "googlemock/include/gmock/gmock.h"
#include "googletest/include/gtest/gtest.h"

namespace coralnpu {
namespace fuzzer {

class MockDetector : public CoverageDetector {
 public:
  MOCK_METHOD(void, OnInstruction,
              (const ::mpact::sim::generic::Instruction* inst), (override));
  MOCK_METHOD(void, OnTrap, (uint64_t trap_code), (override));
  MOCK_METHOD(void, OnRegisterWrite,
              (absl::string_view reg_name, uint64_t value), (override));
  MOCK_METHOD(std::string, GetName, (), (const, override));
};

TEST(CoverageEventRouterTest, RoutesEvents) {
  CoverageEventRouter router;
  auto mock_detector = std::make_unique<MockDetector>();
  auto* mock_ptr = mock_detector.get();
  EXPECT_CALL(*mock_ptr, GetName()).WillRepeatedly(testing::Return("mock"));
  router.RegisterDetector(std::move(mock_detector));

  EXPECT_CALL(*mock_ptr, OnTrap(0x123));
  router.RouteTrap(0x123);

  EXPECT_CALL(*mock_ptr, OnRegisterWrite("x1", 0x456));
  router.RouteRegisterWrite("x1", 0x456);

  ::mpact::sim::generic::Instruction inst(0x123, nullptr);
  EXPECT_CALL(*mock_ptr, OnInstruction(&inst));
  router.RouteInstruction(&inst);
}

}  // namespace fuzzer
}  // namespace coralnpu
