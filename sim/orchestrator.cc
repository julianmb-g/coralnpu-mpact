#include "sim/orchestrator.h"

#include <functional>
#include <utility>

#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"

namespace coralnpu {
namespace fuzzer {

void Orchestrator::PerformPhase2Finalize(
    TestSequence& seq,
    const ::coralnpu::sim::proto::TerminalState& terminal_state) {
  std::string binary_str(seq.itcm_binary());
  std::string* binary = &binary_str;
  uint32_t nop_start = binary->size();
  while (nop_start >= 4) {
    uint32_t inst = 0;
    inst |= static_cast<unsigned char>((*binary)[nop_start - 4]);
    inst |= static_cast<unsigned char>((*binary)[nop_start - 3]) << 8;
    inst |= static_cast<unsigned char>((*binary)[nop_start - 2]) << 16;
    inst |= static_cast<unsigned char>((*binary)[nop_start - 1]) << 24;
    if (inst == 0x08000073 || inst == 0x00000013) {
      nop_start -= 4;
    } else {
      break;
    }
  }

  uint32_t current_pc = nop_start;
  const std::function<void(uint32_t)> emit_inst = [&](uint32_t inst) {
    if (current_pc + 4 <= binary->size()) {
      (*binary)[current_pc] = inst & 0xFF;
      (*binary)[current_pc + 1] = (inst >> 8) & 0xFF;
      (*binary)[current_pc + 2] = (inst >> 16) & 0xFF;
      (*binary)[current_pc + 3] = (inst >> 24) & 0xFF;
      current_pc += 4;
    }
  };

  // Assert X registers (1 to 29)
  for (int i = 1; i < 30; ++i) {
    std::string reg_name = absl::StrCat("x", i);
    decltype(terminal_state.registers().begin()) it =
        terminal_state.registers().find(reg_name);
    if (it != terminal_state.registers().end()) {
      uint32_t golden = it->second;
      uint32_t scratch = 31;  // x31 (t6)

      uint32_t upper20 = (golden + 0x800) >> 12;
      int32_t lower12 = static_cast<int32_t>(golden) - (upper20 << 12);
      uint32_t u_lower12 = static_cast<uint32_t>(lower12) & 0xFFF;

      if (current_pc + 16 > binary->size()) {
        break;  // Out of space
      }

      // 1. lui scratch, upper20
      emit_inst((upper20 << 12) | (scratch << 7) | 0x37);
      // 2. addi scratch, scratch, lower12
      emit_inst((u_lower12 << 20) | (scratch << 15) | (0 << 12) |
                (scratch << 7) | 0x13);
      // 3. beq r, scratch, +8
      uint32_t offset = 8;
      uint32_t bit12 = (offset >> 12) & 0x1;
      uint32_t bit11 = (offset >> 11) & 0x1;
      uint32_t bits10_5 = (offset >> 5) & 0x3F;
      uint32_t bits4_1 = (offset >> 1) & 0xF;
      emit_inst((bit12 << 31) | (bits10_5 << 25) | (scratch << 20) | (i << 15) |
                (0 << 12) | (bits4_1 << 8) | (bit11 << 7) | 0x63);
      // 4. ebreak
      emit_inst(0x00100073);
    }
  }

  // Assert F registers
  for (int i = 0; i < 32; ++i) {
    std::string reg_name = absl::StrCat("f", i);
    decltype(terminal_state.registers().begin()) it =
        terminal_state.registers().find(reg_name);
    if (it != terminal_state.registers().end()) {
      uint32_t golden = it->second;
      uint32_t scratch_golden = 30;   // x30 (t5)
      uint32_t scratch_extract = 31;  // x31 (t6)

      if (current_pc + 20 > binary->size()) break;

      // fmv.x.w scratch_extract, f_i
      emit_inst((0b1110000 << 25) | (0 << 20) | (i << 15) | (0 << 12) |
                (scratch_extract << 7) | 0x53);

      uint32_t upper20 = (golden + 0x800) >> 12;
      int32_t lower12 = static_cast<int32_t>(golden) - (upper20 << 12);
      uint32_t u_lower12 = static_cast<uint32_t>(lower12) & 0xFFF;

      // lui scratch_golden, upper20
      emit_inst((upper20 << 12) | (scratch_golden << 7) | 0x37);
      // addi scratch_golden, scratch_golden, lower12
      emit_inst((u_lower12 << 20) | (scratch_golden << 15) | (0 << 12) |
                (scratch_golden << 7) | 0x13);

      // beq scratch_extract, scratch_golden, +8
      uint32_t offset = 8;
      uint32_t bit12 = (offset >> 12) & 0x1;
      uint32_t bit11 = (offset >> 11) & 0x1;
      uint32_t bits10_5 = (offset >> 5) & 0x3F;
      uint32_t bits4_1 = (offset >> 1) & 0xF;
      emit_inst((bit12 << 31) | (bits10_5 << 25) | (scratch_golden << 20) |
                (scratch_extract << 15) | (0 << 12) | (bits4_1 << 8) |
                (bit11 << 7) | 0x63);

      // ebreak
      emit_inst(0x00100073);
    }
  }

  // Assert V registers (element 0 only)
  for (int i = 0; i < 32; ++i) {
    std::string reg_name = absl::StrCat("v", i);
    decltype(terminal_state.vector_registers().begin()) it =
        terminal_state.vector_registers().find(reg_name);
    if (it != terminal_state.vector_registers().end()) {
      if (it->second.size() >= 4) {
        uint32_t golden = 0;
        golden |= static_cast<unsigned char>(it->second[0]);
        golden |= static_cast<unsigned char>(it->second[1]) << 8;
        golden |= static_cast<unsigned char>(it->second[2]) << 16;
        golden |= static_cast<unsigned char>(it->second[3]) << 24;

        uint32_t scratch_golden = 30;   // x30 (t5)
        uint32_t scratch_extract = 31;  // x31 (t6)

        if (current_pc + 20 > binary->size()) break;

        // vmv.x.s scratch_extract, v_i
        emit_inst((0b010000 << 26) | (1 << 25) | (i << 20) | (0 << 15) |
                  (2 << 12) | (scratch_extract << 7) | 0x57);

        uint32_t upper20 = (golden + 0x800) >> 12;
        int32_t lower12 = static_cast<int32_t>(golden) - (upper20 << 12);
        uint32_t u_lower12 = static_cast<uint32_t>(lower12) & 0xFFF;

        // lui scratch_golden, upper20
        emit_inst((upper20 << 12) | (scratch_golden << 7) | 0x37);
        // addi scratch_golden, scratch_golden, lower12
        emit_inst((u_lower12 << 20) | (scratch_golden << 15) | (0 << 12) |
                  (scratch_golden << 7) | 0x13);

        // beq scratch_extract, scratch_golden, +8
        uint32_t offset = 8;
        uint32_t bit12 = (offset >> 12) & 0x1;
        uint32_t bit11 = (offset >> 11) & 0x1;
        uint32_t bits10_5 = (offset >> 5) & 0x3F;
        uint32_t bits4_1 = (offset >> 1) & 0xF;
        emit_inst((bit12 << 31) | (bits10_5 << 25) | (scratch_golden << 20) |
                  (scratch_extract << 15) | (0 << 12) | (bits4_1 << 8) |
                  (bit11 << 7) | 0x63);

        // ebreak
        emit_inst(0x00100073);
      }
    }
  }

  if (current_pc + 4 <= binary->size()) {
    emit_inst(0x08000073);  // mpause
  }

  seq.set_itcm_binary(binary_str);
}

Orchestrator::Orchestrator(IsgEngine& engine, uint32_t random_start_pc,
                           uint64_t step_limit, uint64_t seed)
    : prng_(seed),
      engine_(engine),
      random_start_pc_(random_start_pc),
      step_limit_(step_limit) {}

SearchResult Orchestrator::PerformPhase1Search() {
  if (archive_.Size() == 0 || (prng_() % 100 < 10)) {
    uint64_t next_seed = prng_();
    engine_.SetSeed(next_seed);
    return {GenerateRandomSequence(engine_, 100), {}, false};
  } else {
    TestSequence parent_seq = archive_.GetRandomParent(prng_);
    return {MutateSequence(parent_seq), parent_seq, true};
  }
}

void Orchestrator::RunEvolutionLoop(int iterations) {
  LOG(INFO) << "Starting Evolution Loop for " << iterations << " iterations.";
  for (int i = 0; i < iterations; ++i) {
    SearchResult search_result = PerformPhase1Search();
    TestSequence& current_seq = search_result.seq;

    RandomSimulatorResult result =
        RunRandomSimulation(current_seq, random_start_pc_, step_limit_);

    current_seq.mutable_terminal_state()->CopyFrom(result.terminal_state);
    current_seq.mutable_memory_dump()->CopyFrom(result.memory_dump);
    if (result.success) {
      current_seq.set_expected_terminal_state(
          absl::StrCat("Cycles: ", result.terminal_state.cycles()));
    } else {
      current_seq.set_expected_terminal_state(absl::StrCat(
          "Cycles: ", result.terminal_state.cycles(), " (Failed/Trapped)"));
    }

    BehavioralDescriptor bd =
        archive_.ExtractDescriptor(result.coverage_summary);

    const std::function<void(TestSequence&)> populate_fn =
        [&](TestSequence& seq) {
          if (bd.detector_name != "Trap Handler Corrupted") {
            PerformPhase2Finalize(seq, result.terminal_state);
          }
          absl::Status status = DisassembleSequence(
              seq, search_result.was_mutated ? search_result.parent : seq);
          if (!status.ok()) {
            LOG(ERROR) << "Failed to disassemble sequence: "
                       << status.message();
          }
        };

    if (archive_.AddIfNovel(bd, current_seq, result.coverage_summary,
                            populate_fn)) {
      LOG(INFO)
          << "Iteration " << i
          << ": Found novel coverage or better local elite! Adding to DB.";

      // Update global coverage
      for (const std::pair<const std::string, uint64_t>& kv :
           result.coverage_summary) {
        if (kv.second > 0) {
          global_coverage_[kv.first] = 1;  // Just track presence for now
        }
      }
    }
  }
}

TestSequence Orchestrator::MutateSequence(const TestSequence& parent) {
  if (parent.itcm_binary().empty()) {
    TestSequence mutable_parent = parent;
    absl::Status compile_status = CompileInMemory(mutable_parent);
    if (!compile_status.ok()) {
      LOG(ERROR) << "Failed to compile parent in MutateSequence: "
                 << compile_status.message();
      return parent;
    }
    return MutateSequence(mutable_parent);
  }

  uint32_t mutable_start = parent.mutable_start();
  uint32_t mutable_end = parent.mutable_end();

  std::string_view preamble_bytes =
      parent.itcm_binary().substr(0, mutable_start);
  std::string_view mutable_bytes =
      parent.itcm_binary().substr(mutable_start, mutable_end - mutable_start);

  std::vector<uint32_t> mutable_insts;
  const char* ptr = mutable_bytes.data();
  for (size_t i = 0; i < mutable_bytes.size(); i += 4) {
    uint32_t word = *reinterpret_cast<const uint32_t*>(ptr + i);
    mutable_insts.push_back(word);
  }

  std::vector<uint32_t> mutated_insts;
  for (uint32_t word : mutable_insts) {
    // 5% chance to drop an instruction
    if (prng_() % 100 < 5) {
      continue;
    }
    mutated_insts.push_back(word);
  }

  // Generate a few random new instructions to inject
  uint64_t next_seed = prng_();
  engine_.SetSeed(next_seed);
  TestSequence injection_seq = GenerateRandomSequence(engine_, 10);

  std::vector<uint32_t> inject_insts;
  const char* inject_ptr = injection_seq.itcm_binary().data();
  for (uint32_t i = injection_seq.mutable_start();
       i < injection_seq.mutable_end(); i += 4) {
    uint32_t word = *reinterpret_cast<const uint32_t*>(inject_ptr + i);
    inject_insts.push_back(word);
  }

  for (uint32_t word : inject_insts) {
    // 10% chance to insert a new instruction at a random point
    if (prng_() % 100 < 10) {
      int insert_idx = 0;
      if (!mutated_insts.empty()) {
        insert_idx = prng_() % mutated_insts.size();
      }
      mutated_insts.insert(mutated_insts.begin() + insert_idx, word);
    }
  }

  // Reconstruct final binary
  std::string new_mutable_bytes;
  new_mutable_bytes.reserve(mutated_insts.size() * 4);
  for (uint32_t word : mutated_insts) {
    new_mutable_bytes.append(reinterpret_cast<const char*>(&word), 4);
  }

  uint32_t total_pc = mutable_start + new_mutable_bytes.size();
  if (total_pc > 8188) {
    LOG(WARNING) << "Mutated sequence exceeds 8kB limit: " << total_pc;
    return parent;
  }

  std::string postamble_bytes;
  postamble_bytes.reserve(8192 - total_pc);
  uint32_t nop = 0x00000013;  // addi x0, x0, 0
  while (total_pc < 8188) {
    postamble_bytes.append(reinterpret_cast<const char*>(&nop), 4);
    total_pc += 4;
  }
  uint32_t mpause = 0x08000073;
  postamble_bytes.append(reinterpret_cast<const char*>(&mpause), 4);

  TestSequence child = parent;
  child.set_itcm_binary(
      absl::StrCat(preamble_bytes, new_mutable_bytes, postamble_bytes));
  child.set_mutable_start(mutable_start);
  child.set_mutable_end(mutable_start + new_mutable_bytes.size());
  child.clear_assembly_text();  // Clear to force disassembly when needed

  return child;
}

}  // namespace fuzzer
}  // namespace coralnpu