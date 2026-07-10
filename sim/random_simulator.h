#ifndef SIM_RANDOM_SIMULATOR_H_
#define SIM_RANDOM_SIMULATOR_H_

#include <cstdint>
#include <string>

#include "sim/fuzzer_types.h"
#include "sim/isg/isg_engine.h"
#include "sim/proto/isg_database.pb.h"
#include "absl/status/status.h"

namespace coralnpu {
namespace fuzzer {

// Normalizes a disassembly string (e.g. by removing trailing commas and
// spaces).
std::string NormalizeDisassembly(std::string disasm);

struct RandomSimulatorResult {
  bool success;
  uint64_t executed_steps;
  CoverageSummary coverage_summary;
  ::coralnpu::sim::proto::TerminalState terminal_state;
  ::coralnpu::sim::proto::MemoryDump memory_dump;
};

// Generates a random payload, assembles it using the provided assembler_path,
// and runs it in the simulator.
RandomSimulatorResult RunRandomSimulation(
    const ::coralnpu::fuzzer::TestSequence& seq, uint32_t random_start_pc,
    uint64_t step_limit = 1000, bool log_pc_discontinuity = false,
    bool debug_trace = false);

// Helper to generate a completely random sequence
::coralnpu::fuzzer::TestSequence GenerateRandomSequence(
    ::coralnpu::fuzzer::IsgEngine& engine, int num_instructions = 1000);

absl::Status DisassembleSequence(
    ::coralnpu::fuzzer::TestSequence& seq,
    const ::coralnpu::fuzzer::TestSequence& parent);

absl::Status CompileInMemory(::coralnpu::fuzzer::TestSequence& seq);

}  // namespace fuzzer
}  // namespace coralnpu

#endif  // SIM_RANDOM_SIMULATOR_H_