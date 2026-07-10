// Copyright 2026 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SIM_CORALNPU_M3_ZFBFMIN_OVERRIDES_H_
#define SIM_CORALNPU_M3_ZFBFMIN_OVERRIDES_H_

#include "mpact/sim/generic/instruction.h"

namespace coralnpu::sim {

// RISC-V Zfbfmin extension (Scalar BFloat16) custom semantic overrides for
// the coralnpu_m3 simulator baseline.

// fcvt.bf16.s rd, rs1 (Floating-point convert BF16 from S)
void CoralNPUM3ZfbfminFcvtBf16S(
    const mpact::sim::generic::Instruction* instruction);

// fcvt.s.bf16 rd, rs1 (Floating-point convert S from BF16)
void CoralNPUM3ZfbfminFcvtSBf16(
    const mpact::sim::generic::Instruction* instruction);

}  // namespace coralnpu::sim

#endif  // SIM_CORALNPU_M3_ZFBFMIN_OVERRIDES_H_
