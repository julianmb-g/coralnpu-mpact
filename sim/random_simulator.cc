#include "sim/random_simulator.h"
// Phase 31: Reverted uncommitted changes to restore clean slate (Recovery 2).

#include <sys/wait.h>
#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_set>

#include "sim/coralnpu_architecture.h"
#include "sim/coralnpu_m3_assembler.h"
#include "sim/coralnpu_m3_bin_encoder_interface.h"
#include "sim/coralnpu_m3_encoder.h"
#include "sim/coralnpu_m3_enums.h"
#include "sim/coralnpu_simulator.h"
#include "sim/execution_tracker.h"
#include "sim/isg/coverage_event_router.h"
#include "sim/memory_config.h"
#include "absl/log/log.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "elfio/elfio.hpp"
#include "riscv/riscv_csr.h"
#include "mpact/sim/util/asm/simple_assembler.h"

namespace coralnpu {
namespace fuzzer {

using ::coralnpu::sim::Architecture;
using ::coralnpu::sim::CoralNPUSimulator;
using ::coralnpu::sim::CoralNPUSimulatorOptions;

std::string NormalizeDisassembly(std::string disasm) {
  while (!disasm.empty() && (disasm.back() == ' ' || disasm.back() == ',')) {
    disasm.pop_back();
  }

  // Find first part before comma.
  size_t first_comma = disasm.find(',');
  std::string first_part = (first_comma == std::string::npos)
                               ? disasm
                               : disasm.substr(0, first_comma);

  // If first_part does not have a space, and is not empty, find end of mnemonic
  // and insert space.
  if (first_part.find(' ') == std::string::npos && !first_part.empty()) {
    size_t split_pos = std::string::npos;
    size_t n = first_part.length();

    // 1. Check zero suffix.
    if (n > 4 && first_part.substr(n - 4) == "zero") {
      split_pos = n - 4;
    }
    // 2. Check X0Dest suffix.
    else if (n > 6 && first_part.substr(n - 6) == "X0Dest") {
      split_pos = n - 6;
    }
    // 3. Check ra, sp, gp, tp suffix.
    else if (n > 2 && (first_part.substr(n - 2) == "ra" ||
                       first_part.substr(n - 2) == "sp" ||
                       first_part.substr(n - 2) == "gp" ||
                       first_part.substr(n - 2) == "tp")) {
      split_pos = n - 2;
    }
    // 3. Check trailing digits + register prefix suffix.
    else {
      size_t digit_start = n;
      while (digit_start > 0 && first_part[digit_start - 1] >= '0' &&
             first_part[digit_start - 1] <= '9') {
        digit_start--;
      }
      if (digit_start < n && digit_start > 0 && n - digit_start <= 2) {
        char reg_char = first_part[digit_start - 1];
        int reg_num = 0;
        for (size_t i = digit_start; i < n; ++i) {
          reg_num = reg_num * 10 + (first_part[i] - '0');
        }
        bool is_reg = false;
        size_t reg_start = digit_start - 1;

        // Check if there is an 'f' prefix for two-letter float registers (e.g.
        // ft7, fa5, fs11).
        if (digit_start > 1 && first_part[digit_start - 2] == 'f' &&
            (reg_char == 't' || reg_char == 'a' || reg_char == 's')) {
          if (reg_char == 't' && reg_num <= 11)
            is_reg = true;
          else if (reg_char == 'a' && reg_num <= 7)
            is_reg = true;
          else if (reg_char == 's' && reg_num <= 11)
            is_reg = true;

          if (is_reg) {
            reg_start = digit_start - 2;
          }
        } else {
          // Standard one-letter prefix registers.
          if (reg_char == 'v' && reg_num <= 31)
            is_reg = true;
          else if (reg_char == 'f' && reg_num <= 31)
            is_reg = true;
          else if (reg_char == 'x' && reg_num <= 31)
            is_reg = true;
          else if (reg_char == 'a' && reg_num <= 7)
            is_reg = true;
          else if (reg_char == 't' && reg_num <= 6)
            is_reg = true;
          else if (reg_char == 's' && reg_num <= 11)
            is_reg = true;
        }

        if (is_reg) {
          split_pos = reg_start;
        }
      }
    }

    if (split_pos != std::string::npos && split_pos > 0 &&
        split_pos < disasm.length()) {
      disasm.insert(split_pos, " ");
    }
  }

  if (absl::StartsWith(disasm, "lui") && disasm.size() > 3 &&
      disasm[3] != ' ' && disasm[3] != '.') {
    disasm.insert(3, " ");
  }
  size_t pos;
  while ((pos = disasm.find("0x0x")) != std::string::npos) {
    disasm.replace(pos, 4, "0x");
  }
  return disasm;
}

namespace {

void AddCustomHandlers(CoralNPUSimulator& simulator, bool exit_on_ebreak) {
  simulator.state()->AddMpauseHandler(
      [&simulator](const ::mpact::sim::generic::Instruction* inst) {
        LOG(INFO) << "mpause instruction received.";
        simulator.top()->RequestHalt(
            ::mpact::sim::generic::CoreDebugInterface::HaltReason::kUserRequest,
            inst);
        return true;
      });
  simulator.state()->AddEbreakHandler([&simulator, exit_on_ebreak](
                                          const ::mpact::sim::generic::
                                              Instruction* inst) {
    LOG(INFO) << "ebreak instruction received. Instruction address: "
              << absl::StrFormat("0x%08x", inst->address());
    if (exit_on_ebreak) {
      simulator.top()->RequestHalt(
          ::mpact::sim::generic::CoreDebugInterface::HaltReason::kUserRequest,
          inst);
      return true;
    }
    return false;
  });
}

}  // namespace

absl::Status CompileInMemory(::coralnpu::fuzzer::TestSequence& seq);

TestSequence GenerateRandomSequence(::coralnpu::fuzzer::IsgEngine& engine,
                                    int num_instructions) {
  engine.Reset();
  engine.EmitPreamble();

  ::coralnpu::sim::CoralNPUSimulatorOptions options;
  options.architecture = ::coralnpu::sim::Architecture::kM3;
  options.exit_on_ebreak = true;
  options.skip_default_handlers = true;
  options.memory_regions = {
      {.start_address = 0x0,
       .length = 0x2000,
       .permissions = ::coralnpu::sim::MemoryPermission::kRead |
                      ::coralnpu::sim::MemoryPermission::kExecute},
      {.start_address = ::coralnpu::sim::kDefaultRwRegionStart,
       .length = ::coralnpu::sim::kDefaultRwRegionLength,
       .permissions = ::coralnpu::sim::MemoryPermission::kRead |
                      ::coralnpu::sim::MemoryPermission::kWrite}};

  ::coralnpu::sim::CoralNPUSimulator gen_sim(options);
  AddCustomHandlers(gen_sim, options.exit_on_ebreak);
  mpact::sim::generic::DecoderInterface* decoder = gen_sim.decoder();
  mpact::sim::util::MemoryInterface* memory = gen_sim.memory();
  coralnpu::sim::CoralNPUV2State* state = gen_sim.state();

  int generated = 0;
  int attempts = 0;
  int decoded_count = 0;
  int passed_whitelist = 0;
  while (generated < num_instructions && attempts < num_instructions * 1000) {
    attempts++;
    uint32_t random_word = engine.prng()();
    ::mpact::sim::generic::DataBuffer* db =
        state->db_factory()->Allocate<uint32_t>(1);
    db->Set<uint32_t>(0, random_word);
    memory->Store(0x0, db);

    ::mpact::sim::generic::Instruction* inst = decoder->DecodeInstruction(0x0);
    db->DecRef();

    if (inst) {
      if (inst->opcode() !=
              static_cast<int>(::coralnpu::sim::isa32_m3::OpcodeEnum::kNone) &&
          inst->opcode() !=
              static_cast<int>(
                  ::coralnpu::sim::isa32_m3::OpcodeEnum::kMpause) &&
          inst->opcode() !=
              static_cast<int>(
                  ::coralnpu::sim::isa32_m3::OpcodeEnum::kEbreak)) {
        decoded_count++;
        std::string disasm = NormalizeDisassembly(inst->AsString());

        if (disasm.find("inv") == std::string::npos) {
          passed_whitelist++;
          engine.EmitInstruction(disasm);
          generated++;
        }
      }
      inst->DecRef();
    }
  }
  LOG(INFO) << "Generation stats: " << attempts << " attempts, "
            << decoded_count << " decoded, " << passed_whitelist
            << " whitelisted, " << generated << " generated.";
  TestSequence seq = engine.Build();
  absl::Status compile_status = CompileInMemory(seq);
  if (!compile_status.ok()) {
    LOG(FATAL) << "Failed to compile generated sequence in-memory: "
               << compile_status.message();
  }
  return seq;
}

void PrintTerminalState(CoralNPUSimulator& simulator) {
  LOG(INFO) << "\n========================================\n"
            << "  TERMINAL REGISTER STATE\n"
            << "========================================";

  // X Registers
  LOG(INFO) << "--- X Registers ---";
  for (int i = 0; i < 32; ++i) {
    std::string name = absl::StrCat("x", i);
    absl::StatusOr<uint64_t> res = simulator.top()->ReadRegister(name);
    if (res.ok()) {
      LOG(INFO) << name << ": 0x" << std::hex << res.value() << std::dec;
    }
  }

  // F Registers
  LOG(INFO) << "--- F Registers ---";
  for (int i = 0; i < 32; ++i) {
    std::string name = absl::StrCat("f", i);
    absl::StatusOr<uint64_t> res = simulator.top()->ReadRegister(name);
    if (res.ok()) {
      LOG(INFO) << name << ": 0x" << std::hex << res.value() << std::dec;
    }
  }

  // V Registers
  LOG(INFO) << "--- V Registers ---";
  for (int i = 0; i < 32; ++i) {
    std::string name = absl::StrCat("v", i);
    std::pair<::mpact::sim::riscv::RVVectorRegister*, bool> res_pair =
        simulator.state()->GetRegister<::mpact::sim::riscv::RVVectorRegister>(
            name);
    if (res_pair.first != nullptr) {
      ::mpact::sim::generic::DataBuffer* db = res_pair.first->data_buffer();
      if (db) {
        std::string val_str = "";
        for (size_t j = 0; j < db->size<uint32_t>(); ++j) {
          absl::StrAppend(
              &val_str, absl::Hex(db->Get<uint32_t>(j), absl::kZeroPad8), " ");
        }
        LOG(INFO) << name << ": " << val_str;
      }
    }
  }

  // CSRs (Common list)
  LOG(INFO) << "--- CSRs ---";
  std::vector<std::string> csrs = {
      "mstatus", "mie",    "mtvec", "mscratch", "mepc",   "mcause", "mtval",
      "mip",     "fflags", "frm",   "fcsr",     "vstart", "vxsat",  "vxrm",
      "vcsr",    "vl",     "vtype", "vlenb",    "cycle",  "time",   "instret"};

  for (const std::string& name : csrs) {
    absl::StatusOr<uint64_t> res = simulator.top()->ReadRegister(name);
    if (res.ok()) {
      LOG(INFO) << name << ": 0x" << std::hex << res.value() << std::dec;
    }
  }
}

RandomSimulatorResult RunRandomSimulation(
    const ::coralnpu::fuzzer::TestSequence& seq, uint32_t random_start_pc,
    uint64_t step_limit, bool log_pc_discontinuity, bool debug_trace) {
  RandomSimulatorResult result;
  result.success = false;
  result.executed_steps = 0;

  ::coralnpu::fuzzer::TestSequence run_seq = seq;
  if (run_seq.itcm_binary().empty()) {
    absl::Status compile_status = CompileInMemory(run_seq);
    if (!compile_status.ok()) {
      LOG(ERROR) << "In-memory compilation failed: "
                 << compile_status.message();
      LOG(ERROR) << "Failed Assembly Text:\n" << seq.assembly_text();
      return result;
    }
  }

  if (run_seq.mutable_start() != 0) {
    random_start_pc = run_seq.mutable_start();
  }

  CoralNPUSimulatorOptions options;
  options.architecture = Architecture::kM3;
  options.exit_on_ebreak = true;
  options.skip_default_handlers = true;
  options.memory_regions = {
      {.start_address = ::coralnpu::sim::kDefaultRxRegionStart,
       .length = ::coralnpu::sim::kDefaultRxRegionLength,
       .permissions = ::coralnpu::sim::MemoryPermission::kRead |
                      ::coralnpu::sim::MemoryPermission::kExecute},
      {.start_address = ::coralnpu::sim::kDefaultRwRegionStart,
       .length = ::coralnpu::sim::kDefaultRwRegionLength,
       .permissions = ::coralnpu::sim::MemoryPermission::kRead |
                      ::coralnpu::sim::MemoryPermission::kWrite}};

  CoralNPUSimulator simulator(options);
  AddCustomHandlers(simulator, options.exit_on_ebreak);

  ::coralnpu::fuzzer::CoverageEventRouter router;
  std::unique_ptr<::coralnpu::fuzzer::ExecutionTracker> execution_tracker =
      std::make_unique<::coralnpu::fuzzer::ExecutionTracker>();
  ::coralnpu::fuzzer::ExecutionTracker* tracker_ptr = execution_tracker.get();
  router.RegisterDetector(std::move(execution_tracker));

  absl::StatusOr<size_t> write_res = simulator.WriteMemory(
      0x0, run_seq.itcm_binary().data(), run_seq.itcm_binary().size());
  if (!write_res.ok()) {
    LOG(ERROR) << "Failed to write ITCM binary to memory: "
               << write_res.status().message();
    return result;
  }

  absl::Status pc_status = simulator.top()->WriteRegister("pc", 0x0);
  if (!pc_status.ok()) {
    LOG(ERROR) << "Failed to set PC: " << pc_status.message();
    return result;
  }

  LOG(INFO) << "Running simulator...";
  absl::Status run_status;
  uint64_t executed = 0;
  std::unordered_set<uint64_t> visited_pcs;

  uint64_t last_pc = 0;
  bool first_inst = true;

  uint64_t captured_trap_pc = 0;
  uint64_t captured_trap_inst_addr = 0;
  simulator.state()->set_on_trap(
      [&](bool is_interrupt, uint64_t trap_value, uint64_t exception_code,
          uint64_t epc, const ::mpact::sim::generic::Instruction* inst) {
        if (!is_interrupt && captured_trap_pc == 0) {
          captured_trap_pc = epc;
          if (inst != nullptr) {
            captured_trap_inst_addr = inst->address();
          }
        }
        return false;
      });

  ::mpact::sim::generic::RegisterBase* pc_reg = nullptr;
  decltype(simulator.state()->registers()->find("pc")) pc_reg_iter =
      simulator.state()->registers()->find("pc");
  if (pc_reg_iter != simulator.state()->registers()->end()) {
    pc_reg = pc_reg_iter->second;
  } else {
    LOG(ERROR) << "PC register not found in simulator state.";
    return result;
  }

  ::mpact::sim::generic::RegisterBase* vtype_reg = nullptr;
  decltype(simulator.state()->registers()->find("vtype")) vtype_reg_iter =
      simulator.state()->registers()->find("vtype");
  if (vtype_reg_iter != simulator.state()->registers()->end()) {
    vtype_reg = vtype_reg_iter->second;
  }

  while (executed < step_limit) {
    if (captured_trap_pc != 0) {
      LOG(INFO) << "Trap captured. epc: 0x" << std::hex << captured_trap_pc
                << ", inst_addr: 0x" << captured_trap_inst_addr << std::dec;
      break;
    }
    uint64_t current_pc = pc_reg->data_buffer()->Get<uint32_t>(0);

    if (!first_inst && (current_pc != last_pc) && (current_pc - last_pc != 4)) {
      if (log_pc_discontinuity) {
        LOG(INFO) << "PC discontinuity: 0x" << std::hex << last_pc << " -> 0x"
                  << current_pc << std::dec;
      }
    }

    if (visited_pcs.find(current_pc) == visited_pcs.end()) {
      visited_pcs.insert(current_pc);

      if (current_pc == random_start_pc) {
        LOG(INFO) << "\n========================================\n"
                  << "  START OF RANDOMIZED INSTRUCTIONS (PC: 0x" << std::hex
                  << random_start_pc << std::dec << ")\n"
                  << "========================================";
      }
    }

    ::mpact::sim::generic::Instruction* inst =
        simulator.top()->GetInstruction(current_pc).value_or(nullptr);

    if (debug_trace) {
      uint32_t raw_inst = 0;
      absl::StatusOr<size_t> mem_res =
          simulator.ReadMemory(current_pc, &raw_inst, sizeof(uint32_t));
      (void)mem_res;
      if (inst) {
        LOG(INFO) << "TRACE: PC=0x" << std::hex << current_pc << " Raw=0x"
                  << raw_inst << " Disasm=" << inst->AsString() << std::dec;
      } else {
        LOG(INFO) << "TRACE: PC=0x" << std::hex << current_pc << " Raw=0x"
                  << raw_inst << " Disasm=INVALID" << std::dec;
      }
    }

    first_inst = false;
    last_pc = current_pc;

    if (inst != nullptr) {
      router.RouteInstruction(inst);
      inst->DecRef();
    }
    run_status = simulator.Step(1).status();
    executed++;

    if (!run_status.ok()) {
      LOG(ERROR) << "Run status error: " << run_status.message();
      break;
    }

    absl::StatusOr<uint32_t> halt_reason_or =
        simulator.top()->GetLastHaltReason();
    if (halt_reason_or.ok() &&
        halt_reason_or.value() !=
            static_cast<uint32_t>(
                ::mpact::sim::generic::CoreDebugInterface::HaltReason::kNone)) {
      // LOG(INFO) << "Halted with reason: " << halt_reason_or.value();
      break;
    }
  }

  result.executed_steps = executed;
  result.coverage_summary = tracker_ptr->coverage_summary();

  // Populate terminal state (best-effort, captures state even if program
  // trapped/timed out)
  {
    ::coralnpu::sim::proto::TerminalState* terminal_state =
        &result.terminal_state;
    terminal_state->set_cycles(simulator.GetCycleCount());
    absl::StatusOr<uint64_t> pc_val = simulator.top()->ReadRegister("pc");
    if (pc_val.ok()) terminal_state->set_pc(*pc_val);
    for (int r = 0; r < 32; ++r) {
      absl::StatusOr<uint64_t> x_val =
          simulator.top()->ReadRegister(absl::StrCat("x", r));
      if (x_val.ok())
        (*terminal_state->mutable_registers())[absl::StrCat("x", r)] = *x_val;
      absl::StatusOr<uint64_t> f_val =
          simulator.top()->ReadRegister(absl::StrCat("f", r));
      if (f_val.ok())
        (*terminal_state->mutable_registers())[absl::StrCat("f", r)] = *f_val;
      absl::StatusOr<mpact::sim::generic::DataBuffer*> v_db =
          simulator.GetRegisterDataBuffer(absl::StrCat("v", r));
      if (v_db.ok()) {
        int size = (*v_db)->size<uint8_t>();
        std::string bytes_str(size, '\0');
        (*v_db)->CopyTo(reinterpret_cast<uint8_t*>(bytes_str.data()));
        (*terminal_state->mutable_vector_registers())[absl::StrCat("v", r)] =
            bytes_str;
      }
    }
    absl::StatusOr<uint64_t> fcsr_val = simulator.top()->ReadRegister("fcsr");
    if (fcsr_val.ok())
      (*terminal_state->mutable_registers())["fcsr"] = *fcsr_val;
    absl::StatusOr<uint64_t> mstatus_val =
        simulator.top()->ReadRegister("mstatus");
    if (mstatus_val.ok())
      (*terminal_state->mutable_registers())["mstatus"] = *mstatus_val;
    absl::StatusOr<uint64_t> vxrm_val = simulator.top()->ReadRegister("vxrm");
    if (vxrm_val.ok())
      (*terminal_state->mutable_registers())["vxrm"] = *vxrm_val;
    absl::StatusOr<uint64_t> vxsat_val = simulator.top()->ReadRegister("vxsat");
    if (vxsat_val.ok())
      (*terminal_state->mutable_registers())["vxsat"] = *vxsat_val;
    absl::StatusOr<uint64_t> vl_val = simulator.top()->ReadRegister("vl");
    if (vl_val.ok()) (*terminal_state->mutable_registers())["vl"] = *vl_val;
    absl::StatusOr<uint64_t> vtype_val = simulator.top()->ReadRegister("vtype");
    if (vtype_val.ok())
      (*terminal_state->mutable_registers())["vtype"] = *vtype_val;

    for (const char* csr :
         {"satp", "mscratch", "mepc", "mcause", "mtvec", "vstart"}) {
      absl::StatusOr<uint64_t> val = simulator.top()->ReadRegister(csr);
      if (val.ok()) (*terminal_state->mutable_registers())[csr] = *val;
    }

    for (const ::coralnpu::sim::CoralNPUV2MemoryRegion& region :
         options.memory_regions) {
      if (region.permissions != ::coralnpu::sim::MemoryPermission::kReadWrite)
        continue;
      uint32_t capture_length = region.length;
      std::string bytes_str(capture_length, '\0');
      absl::StatusOr<size_t> res = simulator.ReadMemory(
          region.start_address, bytes_str.data(), capture_length);
      if (res.ok() && *res == capture_length) {
        (*result.memory_dump.mutable_memory_blobs())[region.start_address] =
            bytes_str;
      }
    }
  }

  if (!run_status.ok()) {
    LOG(ERROR) << "Simulation error: " << run_status.message();
    return result;
  }

  absl::StatusOr<uint32_t> halt_reason_or =
      simulator.top()->GetLastHaltReason();
  if (executed >= step_limit && halt_reason_or.ok() &&
      halt_reason_or.value() ==
          static_cast<uint32_t>(
              ::mpact::sim::generic::CoreDebugInterface::HaltReason::kNone)) {
    LOG(ERROR) << "Simulation timed out.";
    return result;
  }

  uint64_t pc = 0;
  absl::StatusOr<uint64_t> res = simulator.top()->ReadRegister("pc");
  if (res.ok()) {
    pc = res.value();
    // LOG(INFO) << "Terminated at PC: 0x" << std::hex << pc << std::dec;

    bool found_mpause = false;
    std::string disasm_pc = "Failed to decode";
    std::string disasm_pc_minus_4 = "Failed to decode";

    ::mpact::sim::generic::Instruction* final_inst =
        simulator.decoder()->DecodeInstruction(pc);
    if (final_inst) {
      disasm_pc = final_inst->AsString();
      final_inst->DecRef();
      if (absl::StartsWith(disasm_pc, "mpause")) found_mpause = true;
    }

    if (!found_mpause && pc >= 4) {
      ::mpact::sim::generic::Instruction* prev_inst =
          simulator.decoder()->DecodeInstruction(pc - 4);
      if (prev_inst) {
        disasm_pc_minus_4 = prev_inst->AsString();
        prev_inst->DecRef();
        if (absl::StartsWith(disasm_pc_minus_4, "mpause")) found_mpause = true;
      }
    }

    if (!found_mpause) {
      /*
      LOG(ERROR) << "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
                 << "LOUD ERROR: Final PC does not point to mpause!\n"
                 << "Disassembly at final PC (0x" << std::hex << pc
                 << "): " << disasm_pc << "\n"
                 << "Disassembly at PC - 4 (0x" << (pc >= 4 ? pc - 4 : 0)
                 << "): " << disasm_pc_minus_4 << "\n"
                 << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
      PrintTerminalState(simulator);
      */
      return result;
    }
  } else {
    LOG(ERROR) << "\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
               << "LOUD ERROR: Failed to read final PC register!\n"
               << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
    // PrintTerminalState(simulator);
    return result;
  }

  // LOG(INFO) << "Success! Executed " << executed << " steps.";

  result.success = true;
  return result;
}

absl::Status CompileInMemory(::coralnpu::fuzzer::TestSequence& seq) {
  ::coralnpu::sim::isa32_m3::CoralNPUM3BinEncoderInterface
      bin_encoder_interface;
  ::coralnpu::sim::isa32_m3::CoralnpuM3SlotMatcher matcher(
      &bin_encoder_interface);
  ::coralnpu::sim::isa32_m3::CoralNPUM3Assembler coralnpu_assembler(&matcher);
  absl::Status status = matcher.Initialize();
  if (!status.ok()) return status;

  ::mpact::sim::util::assembler::SimpleAssembler assembler(
      ";", ELFIO::ELFCLASS32, &coralnpu_assembler);
  assembler.writer().set_os_abi(ELFIO::ELFOSABI_NONE);
  assembler.writer().set_machine(ELFIO::EM_RISCV);
  enum class RiscVElfFlags : uint32_t {
    kNone = 0,
    kRiscvTso = 0x0001,
    kRiscvRvc = 0x0010,
  };
  assembler.writer().set_flags(static_cast<uint32_t>(RiscVElfFlags::kRiscvTso) |
                               static_cast<uint32_t>(RiscVElfFlags::kRiscvRvc));

  std::stringstream ss(std::string(seq.assembly_text()));
  status = assembler.Parse(ss);
  if (!status.ok()) return status;

  status = assembler.CreateExecutable(0x0, "_start");
  if (!status.ok()) return status;

  uint64_t mutable_start = 0;
  uint64_t mutable_end = 0;

  decltype(assembler.symtab()) symtab = assembler.symtab();
  if (symtab != nullptr) {
    ELFIO::symbol_section_accessor symbols(assembler.writer(), symtab);
    for (ELFIO::Elf_Xword i = 0; i < symbols.get_symbols_num(); ++i) {
      std::string name;
      ELFIO::Elf64_Addr value;
      ELFIO::Elf_Xword size;
      unsigned char bind;
      unsigned char type;
      ELFIO::Elf_Half shndx;
      unsigned char other;
      if (symbols.get_symbol(i, name, value, size, bind, type, shndx, other)) {
        if (name == "_mutable_start") {
          mutable_start = value;
        } else if (name == "_mutable_end") {
          mutable_end = value;
        }
      }
    }
  }

  decltype(assembler.writer().sections[".text"]) text_sec =
      assembler.writer().sections[".text"];
  if (text_sec == nullptr) {
    return absl::NotFoundError("No .text section found in compiled ELF");
  }

  const char* data = text_sec->get_data();
  if (data == nullptr) {
    return absl::NotFoundError(".text section has no data");
  }

  size_t size = text_sec->get_size();
  seq.set_itcm_binary(std::string(data, size));
  seq.set_mutable_start(mutable_start);
  seq.set_mutable_end(mutable_end);

  return absl::OkStatus();
}

absl::Status DisassembleSequence(
    ::coralnpu::fuzzer::TestSequence& seq,
    const ::coralnpu::fuzzer::TestSequence& parent) {
  if (seq.itcm_binary().empty()) {
    return absl::FailedPreconditionError("Cannot disassemble empty binary");
  }

  std::vector<std::string> parent_lines =
      absl::StrSplit(parent.assembly_text(), '\n');
  std::string preamble_text = "";
  for (const std::string& line : parent_lines) {
    absl::StrAppend(&preamble_text, line, "\n");
    if (line == "; BEGIN MUTABLE") {
      break;
    }
  }
  if (preamble_text.find("_mutable_start:") == std::string::npos) {
    absl::StrAppend(&preamble_text, "_mutable_start:\n");
  }

  CoralNPUSimulatorOptions options;
  options.architecture = Architecture::kM3;
  options.skip_default_handlers = true;
  options.memory_regions = {
      {.start_address = ::coralnpu::sim::kDefaultRxRegionStart,
       .length = ::coralnpu::sim::kDefaultRxRegionLength,
       .permissions = ::coralnpu::sim::MemoryPermission::kRead |
                      ::coralnpu::sim::MemoryPermission::kExecute}};

  CoralNPUSimulator simulator(options);
  absl::StatusOr<size_t> write_res = simulator.WriteMemory(
      0x0, seq.itcm_binary().data(), seq.itcm_binary().size());
  if (!write_res.ok()) return write_res.status();

  std::string mutable_text = "";
  for (uint32_t addr = seq.mutable_start(); addr < seq.mutable_end();
       addr += 4) {
    ::mpact::sim::generic::Instruction* inst =
        simulator.decoder()->DecodeInstruction(addr);
    if (inst) {
      std::string disasm = NormalizeDisassembly(inst->AsString());
      absl::StrAppend(&mutable_text, disasm, "\n");
      inst->DecRef();
    } else {
      absl::StrAppend(&mutable_text, ".word 0x00000013\n");
    }
  }

  std::string postamble_text =
      "_mutable_end:\n; END MUTABLE\n; BEGIN PROTECTED\n";
  uint32_t pc = seq.mutable_end();
  while (pc < seq.itcm_binary().size()) {
    uint32_t inst_word = 0;
    if (pc + 4 <= seq.itcm_binary().size()) {
      inst_word =
          *reinterpret_cast<const uint32_t*>(seq.itcm_binary().data() + pc);
    }
    if (inst_word == 0x08000073) {
      absl::StrAppend(&postamble_text, ".word 0x08000073\n");
    } else {
      ::mpact::sim::generic::Instruction* inst =
          simulator.decoder()->DecodeInstruction(pc);
      if (inst) {
        std::string disasm = NormalizeDisassembly(inst->AsString());
        absl::StrAppend(&postamble_text, disasm, "\n");
        inst->DecRef();
      } else {
        absl::StrAppend(&postamble_text, ".word 0x00000013\n");
      }
    }
    pc += 4;
  }
  absl::StrAppend(&postamble_text, "; END PROTECTED\n");

  seq.set_assembly_text(
      absl::StrCat(preamble_text, mutable_text, postamble_text));
  return absl::OkStatus();
}

}  // namespace fuzzer
}  // namespace coralnpu