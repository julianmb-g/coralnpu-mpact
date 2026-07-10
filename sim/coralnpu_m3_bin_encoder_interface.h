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

#ifndef SIM_CORALNPU_M3_BIN_ENCODER_INTERFACE_H_
#define SIM_CORALNPU_M3_BIN_ENCODER_INTERFACE_H_

#include <cstdint>
#include <tuple>
#include <vector>

#include "sim/coralnpu_m3_encoder.h"
#include "sim/coralnpu_m3_enums.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "mpact/sim/util/asm/opcode_assembler_interface.h"
#include "mpact/sim/util/asm/resolver_interface.h"

namespace coralnpu {
namespace sim {
namespace isa32_m3 {

using ::mpact::sim::util::assembler::RelocationInfo;
using ::mpact::sim::util::assembler::ResolverInterface;

class CoralNPUM3BinEncoderInterface : public CoralNPUM3EncoderInterfaceBase {
 public:
  CoralNPUM3BinEncoderInterface();
  ~CoralNPUM3BinEncoderInterface() override = default;

  absl::StatusOr<std::tuple<uint64_t, int>> GetOpcodeEncoding(
      SlotEnum slot, int entry, OpcodeEnum opcode,
      ResolverInterface* resolver) override;

  absl::StatusOr<uint64_t> GetSrcOpEncoding(
      uint64_t address, absl::string_view text, SlotEnum slot, int entry,
      OpcodeEnum opcode, SourceOpEnum source_op, int source_num,
      ResolverInterface* resolver) override;

  absl::Status AppendSrcOpRelocation(
      uint64_t address, absl::string_view text, SlotEnum slot, int entry,
      OpcodeEnum opcode, SourceOpEnum source_op, int source_num,
      ResolverInterface* resolver,
      std::vector<RelocationInfo>& relocations) override;

  absl::StatusOr<uint64_t> GetDestOpEncoding(
      uint64_t address, absl::string_view text, SlotEnum slot, int entry,
      OpcodeEnum opcode, DestOpEnum dest_op, int dest_num,
      ResolverInterface* resolver) override;

  absl::Status AppendDestOpRelocation(
      uint64_t address, absl::string_view text, SlotEnum slot, int entry,
      OpcodeEnum opcode, DestOpEnum dest_op, int dest_num,
      ResolverInterface* resolver,
      std::vector<RelocationInfo>& relocations) override;

  absl::StatusOr<uint64_t> GetListSrcOpEncoding(
      uint64_t address, absl::string_view text, SlotEnum slot, int entry,
      OpcodeEnum opcode, ListSourceOpEnum source_op, int source_num,
      ResolverInterface* resolver) override;

  absl::StatusOr<uint64_t> GetListDestOpEncoding(
      uint64_t address, absl::string_view text, SlotEnum slot, int entry,
      OpcodeEnum opcode, ListDestOpEnum dest_op, int dest_num,
      ResolverInterface* resolver) override;

  absl::StatusOr<uint64_t> GetPredOpEncoding(
      uint64_t address, absl::string_view text, SlotEnum slot, int entry,
      OpcodeEnum opcode, PredOpEnum pred_op,
      ResolverInterface* resolver) override;

 private:
  absl::StatusOr<uint64_t> ParseReg(
      absl::string_view text, SourceOpEnum src_op = SourceOpEnum::kNone) const;
};

}  // namespace isa32_m3
}  // namespace sim
}  // namespace coralnpu

#endif  // SIM_CORALNPU_M3_BIN_ENCODER_INTERFACE_H_
