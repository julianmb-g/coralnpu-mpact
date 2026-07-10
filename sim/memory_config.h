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

#ifndef KELVIN_SIM_MEMORY_CONFIG_H_
#define KELVIN_SIM_MEMORY_CONFIG_H_

#include <cstdint>
#include <string>

#include "sim/coralnpu_v2_state.h"

namespace coralnpu {
namespace sim {

constexpr char kDefaultRxRegionStr[] = "0x0:0x2000:rx";
constexpr char kDefaultRwRegionStr[] = "0x10000:0x8000:rw";
constexpr char kDefaultRwRegion2Str[] = "0x20000000:0x400000:rw";

constexpr uint32_t kDefaultRxRegionStart = 0x0;
constexpr uint32_t kDefaultRxRegionLength = 0x2000;
constexpr MemoryPermission kDefaultRxRegionPermission =
    MemoryPermission::kReadExecute;
constexpr uint32_t kDefaultRwRegionStart = 0x10000;
constexpr uint32_t kDefaultRwRegionLength = 0x8000;
constexpr MemoryPermission kDefaultRwRegionPermission =
    MemoryPermission::kReadWrite;
constexpr uint32_t kDefaultRwRegion2Start = 0x20000000;
constexpr uint32_t kDefaultRwRegion2Length = 0x400000;
constexpr MemoryPermission kDefaultRwRegion2Permission =
    MemoryPermission::kReadWrite;

}  // namespace sim
}  // namespace coralnpu

#endif  // KELVIN_SIM_MEMORY_CONFIG_H_