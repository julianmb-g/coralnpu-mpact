// Copyright 2026 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SIM_CORALNPU_M3_ZFHMIN_INSTRUCTIONS_H_
#define SIM_CORALNPU_M3_ZFHMIN_INSTRUCTIONS_H_

#include "absl/base/nullability.h"
#include "mpact/sim/generic/instruction.h"

namespace coralnpu::sim {

// Zfhmin extension instructions for CoralNPU M3.
void CoralNPUM3ZfhminFlh(
    const ::mpact::sim::generic::Instruction* /*absl_nonnull*/ inst);
void CoralNPUM3ZfhminFsh(
    const ::mpact::sim::generic::Instruction* /*absl_nonnull*/ inst);
void CoralNPUM3ZfhminFMvxh(
    const ::mpact::sim::generic::Instruction* /*absl_nonnull*/ inst);

}  // namespace coralnpu::sim

#endif  // SIM_CORALNPU_M3_ZFHMIN_INSTRUCTIONS_H_
