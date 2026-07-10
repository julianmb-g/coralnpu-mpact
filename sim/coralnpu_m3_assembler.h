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

#ifndef SIM_CORALNPU_M3_ASSEMBLER_H_
#define SIM_CORALNPU_M3_ASSEMBLER_H_

#include <cstddef>
#include <cstdint>
#include <vector>

#include "sim/coralnpu_m3_encoder.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "mpact/sim/util/asm/opcode_assembler_interface.h"
#include "mpact/sim/util/asm/resolver_interface.h"

namespace coralnpu {
namespace sim {
namespace isa32_m3 {

using ::mpact::sim::util::assembler::RelocationInfo;
using ::mpact::sim::util::assembler::ResolverInterface;

class CoralnpuM3SlotMatcher;

class CoralNPUM3Assembler
    : public mpact::sim::util::assembler::OpcodeAssemblerInterface {
 public:
  explicit CoralNPUM3Assembler(CoralnpuM3SlotMatcher* matcher);
  ~CoralNPUM3Assembler() override = default;

  absl::StatusOr<size_t> Encode(
      uint64_t address, absl::string_view text,
      AddSymbolCallback add_symbol_callback,
      mpact::sim::util::assembler::ResolverInterface* resolver,
      std::vector<uint8_t>& bytes,
      std::vector<mpact::sim::util::assembler::RelocationInfo>& relocations)
      override;

 private:
  CoralnpuM3SlotMatcher* matcher_;
};

}  // namespace isa32_m3
}  // namespace sim
}  // namespace coralnpu

#endif  // SIM_CORALNPU_M3_ASSEMBLER_H_
