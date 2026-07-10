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

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "sim/coralnpu_m3_assembler.h"
#include "sim/coralnpu_m3_bin_encoder_interface.h"
#include "sim/coralnpu_m3_encoder.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/check.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "elfio/elf_types.hpp"
#include "elfio/elfio.hpp"
#include "elfio/elfio_dump.hpp"
#include "mpact/sim/util/asm/simple_assembler.h"

ABSL_FLAG(bool, dump_elf, false, "Dump the ELF file");
ABSL_FLAG(bool, compile, false, "Produce a relocatable file");
ABSL_FLAG(std::optional<std::string>, output, std::nullopt, "Output file name");

enum class RiscVElfFlags : uint32_t {
  kNone = 0,
  kRiscvTso = 0x0001,
  kRiscvRvc = 0x0010,
};

int main(int argc, char* argv[]) {
  using ::mpact::sim::util::assembler::SimpleAssembler;
  absl::InitializeLog();
  std::vector<char*> arg_vec = absl::ParseCommandLine(argc, argv);

  if (arg_vec.size() > 2) {
    LOG(ERROR) << "Too many arguments";
    return 1;
  }

  std::optional<std::ifstream> file_stream;
  std::istream* is = nullptr;
  if (arg_vec.size() == 1) {
    is = &std::cin;
  } else {
    file_stream.emplace(arg_vec[1], std::ios::in);
    if (!file_stream->is_open()) {
      LOG(ERROR) << "Failed to open input file: " << arg_vec[1];
      return 1;
    }
    is = &(*file_stream);
  }

  coralnpu::sim::isa32_m3::CoralNPUM3BinEncoderInterface bin_encoder_interface;
  coralnpu::sim::isa32_m3::CoralnpuM3SlotMatcher matcher(
      &bin_encoder_interface);
  coralnpu::sim::isa32_m3::CoralNPUM3Assembler coralnpu_assembler(&matcher);
  CHECK_OK(matcher.Initialize());

  // ELFCLASS32 for CoralNPU M3
  mpact::sim::util::assembler::SimpleAssembler assembler(";", ELFIO::ELFCLASS32,
                                                         &coralnpu_assembler);
  assembler.writer().set_os_abi(ELFIO::ELFOSABI_NONE);
  assembler.writer().set_machine(ELFIO::EM_RISCV);
  assembler.writer().set_flags(static_cast<uint32_t>(RiscVElfFlags::kRiscvTso) |
                               static_cast<uint32_t>(RiscVElfFlags::kRiscvRvc));

  absl::Status status = assembler.Parse(*is);
  if (!status.ok()) {
    LOG(ERROR) << "Failed to parse assembly: " << status.message();
    return 1;
  }

  std::string output_file_name;
  if (absl::GetFlag(FLAGS_compile)) {
    status = assembler.CreateRelocatable();
    if (arg_vec.size() == 1) {
      output_file_name = "stdin.o";
    } else {
      absl::string_view input_file_name = arg_vec[1];
      size_t slash_pos = input_file_name.find_last_of('/');
      size_t dot_pos = input_file_name.find_last_of('.');
      if (dot_pos == std::string::npos ||
          (slash_pos != std::string::npos && dot_pos < slash_pos)) {
        char buf[256];
        int len = absl::SNPrintF(buf, sizeof(buf), "%s.o", input_file_name);
        if (len >= 0) {
          output_file_name.assign(buf, std::min<size_t>(len, sizeof(buf) - 1));
        }
      } else {
        char buf[256];
        int len = absl::SNPrintF(buf, sizeof(buf), "%s.o",
                                 input_file_name.substr(0, dot_pos));
        if (len >= 0) {
          output_file_name.assign(buf, std::min<size_t>(len, sizeof(buf) - 1));
        }
      }
    }
  } else {
    status = assembler.CreateExecutable(0x0, "_start");
    output_file_name = "a.out";
  }
  if (!status.ok()) {
    LOG(ERROR) << "Assembly failure: " << status.message();
    return 1;
  }

  if (std::optional<std::string> o_flag = absl::GetFlag(FLAGS_output); o_flag) {
    output_file_name = *o_flag;
  }
  std::ofstream output_file(output_file_name);
  if (!output_file.is_open()) {
    LOG(ERROR) << "Failed to open output file: " << output_file_name;
    return 1;
  }
  status = assembler.Write(output_file);
  if (!status.ok()) {
    LOG(ERROR) << "Failed to write output file: " << status.message();
    return 1;
  }
  output_file.close();

  if (absl::GetFlag(FLAGS_dump_elf)) {
    ELFIO::elfio reader;
    if (!reader.load(output_file_name)) {
      LOG(ERROR) << "Failed to load output file: " << output_file_name;
      return 1;
    }

    ELFIO::dump::header(std::cout, reader);
    ELFIO::dump::section_headers(std::cout, reader);
    ELFIO::dump::segment_headers(std::cout, reader);
    ELFIO::dump::symbol_tables(std::cout, reader);
    ELFIO::dump::notes(std::cout, reader);
    ELFIO::dump::modinfo(std::cout, reader);
    ELFIO::dump::dynamic_tags(std::cout, reader);
    ELFIO::dump::section_datas(std::cout, reader);
    ELFIO::dump::segment_datas(std::cout, reader);
  }
  return 0;
}
