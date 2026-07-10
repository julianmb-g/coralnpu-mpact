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
#include "sim/coralnpu_m3_assembler.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "sim/coralnpu_m3_bin_encoder_interface.h"
#include "sim/coralnpu_m3_encoder.h"
#include "absl/log/check.h"
#include "absl/status/statusor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "mpact/sim/util/asm/opcode_assembler_interface.h"
#include "mpact/sim/util/asm/resolver_interface.h"
#include "re2/re2.h"

namespace coralnpu {
namespace sim {
namespace isa32_m3 {

using ::mpact::sim::util::assembler::RelocationInfo;
using ::mpact::sim::util::assembler::ResolverInterface;

namespace {
// Formal pass to handle missing trailing commas for vector optional arguments.
// The mpact-sim tokenization logic often uses trailing formatting like `,
// %vmask?` which requires a literal comma in the format string. GCC objdump
// output omits this comma when the mask is absent. We normalize the assembly
// string to match the ISA definitions natively.
std::string NormalizeOptionalArguments(absl::string_view assembly) {
  bool is_kelvin_custom = false;
  if (assembly.find(".b.") != std::string::npos ||
      assembly.find(".h.") != std::string::npos ||
      assembly.find(".uw.") != std::string::npos ||
      assembly.find(".d.") != std::string::npos ||
      assembly.find(".s.") != std::string::npos ||
      (assembly.find(".w.") != std::string::npos &&
       !absl::StartsWith(assembly, "vfw"))) {
    is_kelvin_custom = true;
  }
  if (is_kelvin_custom) {
    return std::string(assembly);
  }

  char buf[256];
  if (assembly.size() >= sizeof(buf) - 2) {
    return std::string(assembly);
  }
  int len = absl::SNPrintF(buf, sizeof(buf), "%s", assembly);
  if (len < 0) return std::string(assembly);

  if (len > 0 && buf[len - 1] != ',' && buf[len - 1] != ' ' &&
      !absl::StartsWith(assembly, "csr") &&
      !absl::StartsWith(assembly, "vset") &&
      !absl::StartsWith(assembly, "vmv") &&
      !absl::StartsWith(assembly, "vfmv") && absl::StartsWith(assembly, "v")) {
    absl::string_view trimmed = absl::StripAsciiWhitespace(assembly);
    if (absl::EndsWith(trimmed, "v0") || absl::EndsWith(trimmed, "v0.t")) {
      int comma_count = 0;
      for (char c : assembly) {
        if (c == ',') comma_count++;
      }
      bool is_masked = false;
      if (comma_count >= 3) {
        is_masked = true;
      } else if (comma_count >= 2 && (absl::StartsWith(assembly, "vl") ||
                                      absl::StartsWith(assembly, "vs"))) {
        is_masked = true;
      }
      if (is_masked) {
        return std::string(assembly);
      }
    }
    buf[len] = ',';
    buf[len + 1] = ' ';
    buf[len + 2] = '\0';
    return std::string(buf);
  }
  return std::string(assembly);
}
}  // namespace

CoralNPUM3Assembler::CoralNPUM3Assembler(CoralnpuM3SlotMatcher* matcher)
    : matcher_(matcher) {}

absl::StatusOr<size_t> CoralNPUM3Assembler::Encode(
    uint64_t address, absl::string_view text,
    AddSymbolCallback add_symbol_callback, ResolverInterface* resolver,
    std::vector<uint8_t>& bytes, std::vector<RelocationInfo>& relocations) {
  static constexpr LazyRE2 kLabelRe = {R"(^(\S+)\s*:)"};
  std::string label;
  if (RE2::Consume(&text, *kLabelRe, &label)) {
    auto status = add_symbol_callback(label, address, 0, 0, 0, 0);
    if (!status.ok()) return status;
  }

  std::string assembly(absl::StripAsciiWhitespace(text));

  // Map register alias X0Dest back to zero for simple assembler matching.
  RE2::GlobalReplace(&assembly, "X0Dest", "zero");

  if (absl::StartsWith(assembly, "vl") || absl::StartsWith(assembly, "vs")) {
    static LazyRE2 kRe = {R"(v(l|s)(u|o|s)?(x)?seg([2-8])\s*)"};
    RE2::Replace(&assembly, *kRe, "v\\1\\2\\3seg\\4");
  }

  std::string asm_str = std::string(assembly);
  if (absl::StartsWith(asm_str, "vsetvli") &&
      asm_str.find(", e") != std::string::npos) {
    static constexpr LazyRE2 kVsetvliRe = {
        R"(vsetvli\s+([^,]+),\s*([^,]+),\s*e([0-9]+),\s*m([0-9f]+),\s*t([au]),\s*m([au]))"};
    std::string rd, rs1, sew_str, m_str, ta_str, ma_str;
    if (RE2::FullMatch(asm_str, *kVsetvliRe, &rd, &rs1, &sew_str, &m_str,
                       &ta_str, &ma_str)) {
      uint32_t sew = 0;
      if (sew_str == "8")
        sew = 0;
      else if (sew_str == "16")
        sew = 1;
      else if (sew_str == "32")
        sew = 2;
      else if (sew_str == "64")
        sew = 3;

      uint32_t lmul = 0;
      if (m_str == "1")
        lmul = 0;
      else if (m_str == "2")
        lmul = 1;
      else if (m_str == "4")
        lmul = 2;
      else if (m_str == "8")
        lmul = 3;
      else if (m_str == "f8")
        lmul = 5;
      else if (m_str == "f4")
        lmul = 6;
      else if (m_str == "f2")
        lmul = 7;

      uint32_t vta = (ta_str == "a") ? 1 : 0;
      uint32_t vma = (ma_str == "a") ? 1 : 0;

      uint32_t zimm11 = (vma << 7) | (vta << 6) | (sew << 3) | lmul;
      asm_str = absl::StrCat("vsetvli ", rd, ", ", rs1, ", ", zimm11);
      assembly = asm_str;
    }
  }

  if (absl::StartsWith(asm_str, "vsetivli") &&
      asm_str.find(", e") != std::string::npos) {
    static constexpr LazyRE2 kVsetivliRe = {
        R"(vsetivli\s+([^,]+),\s*([^,]+),\s*e([0-9]+),\s*m([0-9f]+),\s*t([au]),\s*m([au]))"};
    std::string rd, uimm_str, sew_str, m_str, ta_str, ma_str;
    if (RE2::FullMatch(asm_str, *kVsetivliRe, &rd, &uimm_str, &sew_str, &m_str,
                       &ta_str, &ma_str)) {
      uint32_t sew = 0;
      if (sew_str == "8")
        sew = 0;
      else if (sew_str == "16")
        sew = 1;
      else if (sew_str == "32")
        sew = 2;
      else if (sew_str == "64")
        sew = 3;

      uint32_t lmul = 0;
      if (m_str == "1")
        lmul = 0;
      else if (m_str == "2")
        lmul = 1;
      else if (m_str == "4")
        lmul = 2;
      else if (m_str == "8")
        lmul = 3;
      else if (m_str == "f8")
        lmul = 5;
      else if (m_str == "f4")
        lmul = 6;
      else if (m_str == "f2")
        lmul = 7;

      uint32_t vta = (ta_str == "a") ? 1 : 0;
      uint32_t vma = (ma_str == "a") ? 1 : 0;

      uint32_t zimm10 = (vma << 7) | (vta << 6) | (sew << 3) | lmul;
      asm_str = absl::StrCat("vsetivli ", uimm_str, ", ", zimm10);
      assembly = asm_str;
    }
  }

  // Map no-aliases CSR instructions back to pseudo-instructions for mpact.
  static constexpr LazyRE2 kCsrNrRe = {
      R"(^(csrr[wsc]i?)\s+(zero|x0)\s*,\s*(.*)$)"};
  absl::string_view op, reg, rest;
  if (RE2::FullMatch(assembly, *kCsrNrRe, &op, &reg, &rest)) {
    if (op == "csrrw")
      op = "csrw";
    else if (op == "csrrs")
      op = "csrs";
    else if (op == "csrrc")
      op = "csrc";
    else if (op == "csrrwi")
      op = "csrwi";
    else if (op == "csrrsi")
      op = "csrsi";
    else if (op == "csrrci")
      op = "csrci";

    char buf[256];
    int len = absl::SNPrintF(buf, sizeof(buf), "%s %s", op, rest);
    if (len >= 0) {
      assembly.assign(buf, std::min<size_t>(len, sizeof(buf) - 1));
    }
  } else {
    if (absl::StartsWith(assembly, "csrrw ")) {
      RE2::Replace(&assembly, "^csrrw ", "csrw ");
    } else if (absl::StartsWith(assembly, "csrrs ")) {
      RE2::Replace(&assembly, "^csrrs ", "csrs ");
    } else if (absl::StartsWith(assembly, "csrrc ")) {
      RE2::Replace(&assembly, "^csrrc ", "csrc ");
    } else if (absl::StartsWith(assembly, "csrrwi ")) {
      RE2::Replace(&assembly, "^csrrwi ", "csrwi ");
    } else if (absl::StartsWith(assembly, "csrrsi ")) {
      RE2::Replace(&assembly, "^csrrsi ", "csrsi ");
    } else if (absl::StartsWith(assembly, "csrrci ")) {
      RE2::Replace(&assembly, "^csrrci ", "csrci ");
    }
  }

  // Normalize assembly by enforcing exactly one space after commas to match ISA
  // formatting.
  static constexpr LazyRE2 kCommaSpaceRe = {R"(,\s*)"};
  RE2::GlobalReplace(&assembly, *kCommaSpaceRe, ", ");

  // Strip trailing commas and spaces.
  while (!assembly.empty() &&
         (assembly.back() == ' ' || assembly.back() == ',')) {
    assembly.pop_back();
  }

  auto res = matcher_->Encode(address, assembly, 0, resolver, relocations);

  if (!res.ok()) {
    std::string normalized = NormalizeOptionalArguments(assembly);
    if (normalized != assembly) {
      res = matcher_->Encode(address, normalized, 0, resolver, relocations);
    }
  }

  if (!res.ok()) return res.status();

  auto [value, size] = *res;
  int num_bytes = size / 8;
  if (num_bytes > 8) num_bytes = 8;
  for (int i = 0; i < num_bytes; ++i) {
    bytes.push_back((value >> (i * 8)) & 0xFF);
  }
  return bytes.size();
}

}  // namespace isa32_m3
}  // namespace sim
}  // namespace coralnpu
