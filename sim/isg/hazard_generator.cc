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

#include "sim/isg/hazard_generator.h"

#include <memory>

#include "sim/coralnpu_m3_enums.h"
#include "sim/coralnpu_m3_user_decoder.h"
#include "sim/coralnpu_v2_state.h"
#include "sim/memory_config.h"
#include "absl/memory/memory.h"
#include "absl/strings/str_cat.h"
#include "mpact/sim/generic/instruction.h"
#include "mpact/sim/util/memory/flat_demand_memory.h"

namespace coralnpu {
namespace fuzzer {

void GenerateDataHazard(IsgEngine& engine) {
  uint32_t reg_write = (engine.prng()() % 31) + 1;
  uint32_t reg_read1 = (engine.prng()() % 31) + 1;
  uint32_t reg_read2 = (engine.prng()() % 31) + 1;
  uint32_t dest = (engine.prng()() % 31) + 1;
  uint32_t dist = engine.prng()() % 5;
  bool ta = engine.prng()() % 2 == 0;
  bool ma = engine.prng()() % 2 == 0;
  bool masked = engine.prng()() % 2 == 0;

  engine.EmitVsetvli("t2", "zero", VectorSew::e8, VectorLmul::m1, ta, ma);
  engine.BeginVectorBlock()
      .EmitVadd(absl::StrCat("v", reg_write), absl::StrCat("v", reg_read1),
                absl::StrCat("v", reg_read2), masked)
      .EndBlock();

  for (uint32_t i = 0; i < dist; ++i) {
    engine.EmitInstruction("addi x0, x0, 0");
  }

  engine.BeginVectorBlock()
      .EmitVsub(absl::StrCat("v", dest), absl::StrCat("v", reg_write),
                absl::StrCat("v", reg_read1), masked)
      .EndBlock();
}

void GenerateControlHazard(IsgEngine& engine) {
  uint32_t dist = (engine.prng()() % 4) + 1;

  engine.EmitInstruction("addi t0, zero, 2");
  engine.EmitInstruction("addi t1, zero, 0");
  uint32_t beq_pc = engine.CurrentPc();
  // We standardize on absolute addresses for branch targets. The MPACT
  // assembler parses literal integer strings as absolute addresses, computing
  // the relative offset internally. Mixing absolute and relative literals (like
  // '-') is inconsistent and relies on undocumented parser fallback behavior.
  engine.EmitInstructionFormat("beq t0, t1, 0x%x", beq_pc + dist * 4 + 12);
  for (uint32_t i = 0; i < dist; ++i) {
    engine.EmitInstruction("addi x0, x0, 0");
  }
  engine.EmitInstruction("addi t0, t0, -1");
  engine.EmitInstructionFormat("bne t0, t1, 0x%x", beq_pc);
}

void GenerateStructuralHazard(IsgEngine& engine) {
  uint32_t iters = (engine.prng()() % 3) + 2;

  engine.EmitVsetvli("t2", "zero", VectorSew::e32, VectorLmul::m8);
  VectorBlockBuilder block = engine.BeginVectorBlock();

  uint32_t base_reg = (engine.prng()() % 4) * 8;
  uint32_t vd = base_reg;
  uint32_t vs2 = (base_reg + 8) % 32;
  uint32_t vs1 = (base_reg + 16) % 32;
  uint32_t vs2_div = (base_reg + 24) % 32;

  for (uint32_t i = 0; i < iters; ++i) {
    block.EmitVmul(absl::StrCat("v", vd), absl::StrCat("v", vs2),
                   absl::StrCat("v", vs1));
  }
  block
      .EmitVdiv(absl::StrCat("v", vs2), absl::StrCat("v", vs2_div),
                absl::StrCat("v", vd))
      .EndBlock();
}

void GenerateEdgeCaseOperands(IsgEngine& engine) {
  // Arithmetic Edge Case: INT_MIN / -1 (Signed Overflow)
  engine.EmitInstruction("addi t0, zero, -1");
  engine.EmitInstruction(
      "lui t2, 0x80000");  // 0x80000 << 12 = 0x80000000 (INT_MIN)
  engine.EmitInstruction("div t4, t2, t0");

  // Memory Edge Case: Unaligned Load
  // Explicitly emit a load to an unaligned address in Region 1
  engine.EmitInstructionFormat("lui t5, 0x%x",
                               ::coralnpu::sim::kDefaultRwRegionStart >> 12);
  engine.BeginMemoryBlock().EmitLoad("t6", "t5", 25).EndBlock();

  // Division by zero edge case
  engine.EmitInstruction("addi t1, zero, 0");
  engine.EmitInstruction("div t4, t2, t1");
}

void GenerateRandomInstructions(IsgEngine& engine) {
  auto memory = std::make_unique<::mpact::sim::util::FlatDemandMemory>();
  auto state = std::make_unique<::coralnpu::sim::CoralNPUV2State>(
      "CoralNPUM3", ::mpact::sim::riscv::RiscVXlen::RV32, memory.get());
  ::coralnpu::sim::CoralNPUM3UserDecoder decoder(state.get(), memory.get());

  int generated = 0;
  int attempts = 0;
  while (generated < 1000 && attempts < 1000000) {
    attempts++;
    uint32_t random_word = engine.prng()();
    ::mpact::sim::generic::DataBuffer* db =
        state->db_factory()->Allocate<uint32_t>(1);
    db->Set<uint32_t>(0, random_word);
    memory->Store(0x0, db);

    ::mpact::sim::generic::Instruction* inst = decoder.DecodeInstruction(0x0);
    db->DecRef();

    if (inst->opcode() !=
        static_cast<int>(::coralnpu::sim::isa32_m3::OpcodeEnum::kNone)) {
      std::string disasm = inst->AsString();
      while (!disasm.empty() &&
             (disasm.back() == ' ' || disasm.back() == ',')) {
        disasm.pop_back();
      }
      engine.EmitInstruction(disasm);
      generated++;
    }
    inst->DecRef();
  }
}

}  // namespace fuzzer
}  // namespace coralnpu