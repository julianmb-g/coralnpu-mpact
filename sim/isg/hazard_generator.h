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

#ifndef KELVIN_SIM_ISG_HAZARD_GENERATOR_H_
#define KELVIN_SIM_ISG_HAZARD_GENERATOR_H_

#include "sim/isg/isg_engine.h"

namespace coralnpu {
namespace fuzzer {

// Generator for structured Data Hazards (Read-After-Write)
void GenerateDataHazard(IsgEngine& engine);

// Generator for structured Control Hazards (Branches)
void GenerateControlHazard(IsgEngine& engine);

// Generator for structured Structural Hazards (Resource conflicts)
void GenerateStructuralHazard(IsgEngine& engine);

// Generator for Edge-case operands
void GenerateEdgeCaseOperands(IsgEngine& engine);

// Generator for random instructions
void GenerateRandomInstructions(IsgEngine& engine);

}  // namespace fuzzer
}  // namespace coralnpu

#endif  // KELVIN_SIM_ISG_HAZARD_GENERATOR_H_