#include "sim/execution_tracker.h"

#include <algorithm>
#include <any>
#include <array>
#include <vector>

#include "absl/strings/str_cat.h"
#include "riscv/riscv_state.h"
#include "riscv/riscv_vector_state.h"
#include "mpact/sim/generic/instruction.h"

namespace coralnpu {
namespace fuzzer {

namespace {
static constexpr absl::string_view kVectorRegNames[32] = {
    "v0",  "v1",  "v2",  "v3",  "v4",  "v5",  "v6",  "v7",  "v8",  "v9",  "v10",
    "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19", "v20", "v21",
    "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31"};
}  // namespace

void ExecutionTracker::OnInstruction(
    const ::mpact::sim::generic::Instruction* inst) {
  if (inst == nullptr) return;
  LogOpcode(static_cast<::coralnpu::sim::isa32_m3::OpcodeEnum>(inst->opcode()));
  CalculateHazardDistance(inst);
  VerifyFpPoisonPills(inst);
}

void ExecutionTracker::OnTrap(uint64_t trap_code) {
  // Trap handling logic, if needed
}

void ExecutionTracker::OnRegisterWrite(absl::string_view reg_name,
                                       uint64_t value) {
  // Register write logic, if needed
}

void ExecutionTracker::LogOpcode(::coralnpu::sim::isa32_m3::OpcodeEnum opcode) {
  instruction_count_++;
  int index = static_cast<int>(opcode);
  if (index >= 0 &&
      index < static_cast<int>(
                  ::coralnpu::sim::isa32_m3::OpcodeEnum::kPastMaxValue)) {
    coverage_summary_[absl::StrCat(
        "k", ::coralnpu::sim::isa32_m3::kOpcodeNames[index])]++;
  } else {
    coverage_summary_["kUnknown"]++;
  }
}

void ExecutionTracker::CalculateHazardDistance(
    const ::mpact::sim::generic::Instruction* inst) {
  if (inst == nullptr) return;

  uint32_t vl = 32;
  int vlmul_scaled = 8;
  auto* state = inst->state();
  if (state != nullptr) {
    auto* rv_state = dynamic_cast<::mpact::sim::riscv::RiscVState*>(state);
    if (rv_state != nullptr) {
      auto* rv_vector = rv_state->rv_vector();
      if (rv_vector != nullptr) {
        vl = rv_vector->vector_length();
        vlmul_scaled = rv_vector->vector_length_multiplier();
      }
    }
  }

  // Use fixed-size stack array to avoid dynamic allocations.
  auto expand_vector_reg = [vl, vlmul_scaled](
                               absl::string_view reg_name,
                               std::array<absl::string_view, 8>& expanded,
                               int& count) {
    if (!reg_name.empty() && reg_name[0] == 'v') {
      if (vl == 0) {
        count = 0;
        return;
      }
      int reg_num = 0;
      bool is_digits = true;
      for (size_t idx = 1; idx < reg_name.size(); ++idx) {
        if (reg_name[idx] < '0' || reg_name[idx] > '9') {
          is_digits = false;
          break;
        }
        reg_num = reg_num * 10 + (reg_name[idx] - '0');
      }
      if (is_digits && reg_name.size() > 1) {
        int group_size = 1;
        if (vlmul_scaled == 16)
          group_size = 2;
        else if (vlmul_scaled == 32)
          group_size = 4;
        else if (vlmul_scaled == 64)
          group_size = 8;

        count = group_size;
        for (int i = 0; i < group_size; ++i) {
          expanded[i] = kVectorRegNames[(reg_num + i) % 32];
        }
        return;
      }
    }
    expanded[0] = reg_name;
    count = 1;
  };

  // Validate implicit CSR side-effects on fflags and vstart.
  auto check_csr = [this](const std::string& name) {
    if (name == "fflags") {
      coverage_summary_["csr_side_effect_fflags"]++;
    } else if (name == "vstart") {
      coverage_summary_["csr_side_effect_vstart"]++;
    } else if (name == "mtvec") {
      coverage_summary_["Trap Handler Corrupted"] = 1;
    }
  };

  // Track Read-After-Write (RAW) hazards.
  for (int i = 0; i < inst->SourcesSize(); ++i) {
    auto* src = inst->Source(i);
    if (src == nullptr) continue;

    std::any obj = src->GetObject();
    // Only track hazards for operands that have an underlying object (e.g.,
    // registers).
    if (obj.has_value()) {
      std::string reg_name = src->AsString();
      std::array<absl::string_view, 8> expanded;
      int count = 0;
      expand_vector_reg(reg_name, expanded, count);
      for (int j = 0; j < count; ++j) {
        auto expanded_reg = expanded[j];
        auto it = last_written_cycle_.find(expanded_reg);
        if (it != last_written_cycle_.end()) {
          uint64_t last_cycle = it->second;
          // Instruction count represents the current instruction/cycle index.
          uint64_t distance = instruction_count_ - last_cycle;

          // Track distances 1, 2, and 3.
          if (distance > 0 && distance <= 3) {
            std::string hazard_name = absl::StrCat("RAW_HAZARD_", distance);
            coverage_summary_[hazard_name]++;
            uint32_t current_address = inst->address();
            hazard_to_address_[hazard_name] = current_address;
            auto gen_it = address_to_generator_.find(current_address);
            if (gen_it != address_to_generator_.end()) {
              hazard_to_generator_[hazard_name] = gen_it->second;
            }
          }
        }
      }
    }
    check_csr(src->AsString());
  }

  // Update last written cycle for destinations.
  for (int i = 0; i < inst->DestinationsSize(); ++i) {
    auto* dest = inst->Destination(i);
    if (dest == nullptr) continue;

    std::any obj = dest->GetObject();
    if (obj.has_value()) {
      std::string reg_name = dest->AsString();
      std::array<absl::string_view, 8> expanded;
      int count = 0;
      expand_vector_reg(reg_name, expanded, count);
      for (int j = 0; j < count; ++j) {
        auto expanded_reg = expanded[j];
        last_written_cycle_[expanded_reg] = instruction_count_;
      }
    }
    check_csr(dest->AsString());
  }
}

void ExecutionTracker::VerifyFpPoisonPills(
    const ::mpact::sim::generic::Instruction* inst) {
  for (int i = 0; i < inst->SourcesSize(); ++i) {
    ::mpact::sim::generic::SourceOperandInterface* src = inst->Source(i);
    if (!src) continue;

    std::string name = src->AsString();
    if (name.empty() || name[0] != 'f') {
      continue;
    }

    int elements = 1;
    for (int dim : src->shape()) {
      elements *= dim;
    }

    for (int j = 0; j < elements; ++j) {
      uint64_t val = src->AsUint64(j);

      bool is_nan_boxed = false;
      if ((val >> 32) == 0xFFFFFFFFULL) {
        coverage_summary_["fp_poison_pill_NaN-boxed_scalar"]++;
        is_nan_boxed = true;
      }

      bool is_zero_extended = ((val >> 32) == 0ULL);

      if (is_nan_boxed || is_zero_extended) {
        uint32_t val32 = static_cast<uint32_t>(val);
        uint32_t exp32 = (val32 >> 23) & 0xFF;
        uint32_t frac32 = val32 & 0x7FFFFF;
        if (exp32 == 0xFF && frac32 != 0 && (frac32 & 0x400000) == 0) {
          coverage_summary_["fp_poison_pill_Signaling_NaN"]++;
        } else if (exp32 == 0 && frac32 != 0) {
          coverage_summary_["fp_poison_pill_Denormal"]++;
        }
      } else {
        uint64_t exp64 = (val >> 52) & 0x7FF;
        uint64_t frac64 = val & 0xFFFFFFFFFFFFFULL;
        if (exp64 == 0x7FF && frac64 != 0 &&
            (frac64 & 0x8000000000000ULL) == 0) {
          coverage_summary_["fp_poison_pill_Signaling_NaN"]++;
        } else if (exp64 == 0 && frac64 != 0) {
          coverage_summary_["fp_poison_pill_Denormal"]++;
        }
      }
    }
  }
}

void ExecutionTracker::RegisterGeneratorMapping(
    uint32_t address, const std::string& generator_function) {
  address_to_generator_[address] = generator_function;
}

absl::StatusOr<std::string> ExecutionTracker::GetGeneratorForHazard(
    const std::string& hazard_name) const {
  auto it = hazard_to_generator_.find(hazard_name);
  if (it != hazard_to_generator_.end()) {
    return it->second;
  }
  return absl::NotFoundError(
      absl::StrCat("Generator not found for hazard: ", hazard_name));
}

absl::StatusOr<uint32_t> ExecutionTracker::GetTraceAddressForHazard(
    const std::string& hazard_name) const {
  auto it = hazard_to_address_.find(hazard_name);
  if (it != hazard_to_address_.end()) {
    return it->second;
  }
  return absl::NotFoundError(
      absl::StrCat("Address not found for hazard: ", hazard_name));
}

void ExecutionTracker::RegisterExpectedHazard(const std::string& hazard_name) {
  expected_hazards_.insert(hazard_name);
}

std::vector<std::string> ExecutionTracker::GetCoverageGaps() const {
  std::vector<std::string> gaps;
  for (const auto& expected : expected_hazards_) {
    if (!coverage_summary_.contains(expected) ||
        coverage_summary_.at(expected) == 0) {
      gaps.push_back(expected);
    }
  }
  return gaps;
}

std::string ExecutionTracker::GetCoverageGapSummary() const {
  std::vector<std::string> gaps = GetCoverageGaps();
  if (gaps.empty()) {
    return "No coverage gaps found.\n";
  }
  std::string summary = "Coverage Gaps Summary:\n";
  summary += "----------------------------------------\n";
  for (const auto& gap : gaps) {
    summary += absl::StrCat(gap, "\n");
  }
  summary += "----------------------------------------\n";
  return summary;
}

std::string ExecutionTracker::GetCoverageJson() const {
  std::string json = "{";
  bool first = true;
  for (const auto& [key, value] : coverage_summary_) {
    if (!first) json += ", ";
    json += absl::StrCat("\"", key, "\": ", value);
    first = false;
  }
  json += "}";
  return json;
}

std::string ExecutionTracker::GetCoverageLcov() const {
  std::string lcov = "TN:coralnpu_m3_coverage\nSF:coralnpu_m3.isa\n";
  std::vector<std::pair<std::string, uint64_t>> sorted_entries;
  sorted_entries.reserve(coverage_summary_.size());
  for (const auto& pair : coverage_summary_) {
    sorted_entries.push_back({pair.first, pair.second});
  }
  std::sort(sorted_entries.begin(), sorted_entries.end(),
            [](const std::pair<std::string, uint64_t>& a,
               const std::pair<std::string, uint64_t>& b) {
              return a.first < b.first;
            });

  uint64_t total = sorted_entries.size();
  uint64_t hit_count = 0;
  for (size_t i = 0; i < total; ++i) {
    const auto& pair = sorted_entries[i];
    int line_num = i + 1;
    lcov += absl::StrCat("FN:", line_num, ",", pair.first, "\n");
    lcov += absl::StrCat("FNDA:", pair.second, ",", pair.first, "\n");
    lcov += absl::StrCat("DA:", line_num, ",", pair.second, "\n");
    if (pair.second > 0) {
      hit_count++;
    }
  }
  lcov += absl::StrCat("FNF:", total, "\n");
  lcov += absl::StrCat("FNH:", hit_count, "\n");
  lcov += absl::StrCat("LF:", total, "\n");
  lcov += absl::StrCat("LH:", hit_count, "\n");
  lcov += "end_of_record\n";
  return lcov;
}

}  // namespace fuzzer
}  // namespace coralnpu
