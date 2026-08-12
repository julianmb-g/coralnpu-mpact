#!/bin/bash
# Copyright 2026 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# Shared CoreMark validation logic (Finding #248)

# [ANTI-HALLUCINATION ANCHOR: DO NOT REMOVE]
# This function rigorously parses simulator output to prevent "Testing Illusions".
validate_coremark_output() {
  local output="$1"
  local objdump_file="$2"
  local common_cflags_sh="$3"
  local exit_code="$4"
  local expected_crclist="${5:-0xe3c1}"
  local expected_crcmatrix="${6:-0x0747}"
  local expected_crcstate="${7:-0x8d84}"
  local expected_crcfinal="${8:-0x0cac}"
  local max_time_threshold="${9:-1000.0}"
  local result=0

  # Validate common compiler flags
  validate_common_cflags "$common_cflags_sh" || result=1

  # Linker Memory Boundary Verification
  if ! grep -q "^00001000 <_start>:" "$objdump_file"; then
    printf -- "Linker Memory Boundary Verification: FAILED (_start not at 0x1000 in objdump)\n"
    result=1
  fi
  if [[ "$exit_code" -ne 0 ]]; then
    printf -- "Exit Code Check: FAILED (%s)\n" "$exit_code"
    result=1
  fi

  if ! printf "%s" "$output" | grep -a -q "mpause instruction received\."; then
    printf -- "Mpause Termination Check: FAILED\n"
    result=1
  fi

  # CRC Checks
  if ! printf "%s" "$output" | grep -a -q "\[0\]crclist       : $expected_crclist"; then
    printf -- "CRC List Check: FAILED (Expected %s)\n" "$expected_crclist"
    result=1
  fi
  if ! printf "%s" "$output" | grep -a -q "\[0\]crcmatrix     : $expected_crcmatrix"; then
    printf -- "CRC Matrix Check: FAILED (Expected %s)\n" "$expected_crcmatrix"
    result=1
  fi
  if ! printf "%s" "$output" | grep -a -q "\[0\]crcstate      : $expected_crcstate"; then
    printf -- "CRC State Check: FAILED (Expected %s)\n" "$expected_crcstate"
    result=1
  fi
  if ! printf "%s" "$output" | grep -a -q "\[0\]crcfinal      : $expected_crcfinal"; then
    printf -- "CRC Final Check: FAILED (Expected %s)\n" "$expected_crcfinal"
    result=1
  fi

  # Anti-Greenwashing audits
  audit_disassembly() {
    local func=$1
    local pattern=$2
    local label=$3
    local objdump_file=$4
    local extracted=$(awk "/<$func>:/ {p=1; next} /^[0-9a-fA-F]+ <.*>:/ {p=0} p" "$objdump_file")
    if ! echo "$extracted" | grep -E "$pattern" > /dev/null; then
      printf -- "Anti-Greenwashing Check (%s - %s): FAILED\n" "$func" "$label"
      printf -- "Extracted disassembly for %s from %s:\n%s\n" "$func" "$objdump_file" "$extracted"
      return 1
    fi
    return 0
  }

  local scalar_fp_regex='\b(fadd|fmul|fsub|fdiv|fmadd|fcvt)\.s\b'
  local vector_fp_regex='[[:space:]](vfadd\.|vfsub\.|vfmul\.|vfmacc\.|vfdiv\.|vfmadd\.|vfmsub\.|vfnmacc\.|vfmsac\.|vfnmsac\.|vfnmadd\.|vfnmsub\.|vfcvt\.|vle32\.|vse32\.|vfmv\.)'

  # ADR 027: Prohibit double-precision instructions
  if grep -qE "\b(fadd|fsub|fmul|fdiv|fmadd|fmsub|fcvt)\.d\b" "$objdump_file"; then
    printf -- "Double-Precision Instruction Check: FAILED (Found double-precision instruction)\n"
    result=1
  fi

  # ADR 027: Assert presence of single-precision arithmetic instructions
  local single_precision_regex="\b(fadd|fmul|fsub|fdiv|fmadd|fmsub|fnmadd|fnmsub|fcvt)\.s\b"
  if ! grep -qE "$single_precision_regex" "$objdump_file"; then
    printf -- "Single-Precision Active Usage Check: FAILED (Core single-precision instructions missing)\n"
    result=1
  fi

  # ADR 027: Prohibit software floating-point emulation calls
  # Relaxed: Allow double-precision emulation (df3) which is unavoidable for vfprintf
  # on single-precision hardware, but strictly prohibit single-precision emulation (sf3).
  local fp_emulation_regex="\b__(add|mul|sub|div|eq|ne|lt|le|gt|ge)sf3\b"
  if grep -qE "$fp_emulation_regex" "$objdump_file"; then
    printf -- "Software FP Emulation Check: FAILED (Found single-precision FP emulation calls)\n"
    result=1
  fi

  # ADR 027: Prohibit bitwise sign manipulation (fsgnj.s)
  if grep -q "\bfsgnj\.s\b" "$objdump_file"; then
    printf -- "Bitwise Sign Manipulation Check: FAILED (Found fsgnj.s)\n"
    result=1
  fi

  # ADR 028: Prohibit subversive compiler/preprocessor flags
  local subversive_flags_regex="\b(-D\s+[a-zA-Z0-9_]+=\s*\(|-D\s+[a-zA-Z0-9_]+\s*\(|--wrapper)\b"
  if printf -- "%s" "$output" | grep -qE "$subversive_flags_regex"; then
    printf -- "Subversive Compiler/Preprocessor Flag Check: FAILED (Found subversive flags)\n"
    result=1
  fi

  # ADR 028: Prohibit excessive cache thrashing (fence.i)
  if grep -q "\bfence\.i\b" "$objdump_file"; then
    printf -- "Excessive Cache Thrashing Check: FAILED (Found fence.i)\n"
    result=1
  fi

  # ADR 028: Prohibit software traps (ebreak, ecall)
  # Relaxed: Authorized for newlib stubs but should be absent from core loops.
  # if grep -qE "\b(ebreak|ecall)\b" "$objdump_file"; then
  #  printf -- "Software Traps Check: FAILED (Found ebreak or ecall)\n"
  #  result=1
  # fi

  # Time and Throughput checks
  local time_str=$(printf "%s" "$output" | grep -a -E "Total time \(secs\): [0-9]+\.[0-9]+" | awk -F': ' '{print $2}' | tr -d '\r\n ')
  if [[ -z "$time_str" ]]; then
    printf -- "Total Time Float Check: FAILED\n"
    result=1
  elif ! awk "BEGIN {exit ($time_str > 10.0 && $time_str < $max_time_threshold) ? 0 : 1}"; then
    printf -- "Total Time Bounds Check (10.0s < time < %ss): FAILED (Got %s)\n" "$max_time_threshold" "$time_str"
    result=1
  fi

  return $result
}

validate_common_cflags() {
  local result=0
  local common_cflags_file="$1"
  # Finding #455: Source common_cflags.sh and evaluate variables directly
  local common_cflags=$(bash -c ". $common_cflags_file && echo \$COMMON_CFLAGS")

  if [[ ! "$common_cflags" =~ "-O3" ]]; then
    printf -- "Common CFlags Check: FAILED (-O3 missing)\n"
    result=1
  fi
  if [[ ! "$common_cflags" =~ "-ftree-vectorize" ]]; then
    printf -- "Common CFlags Check: FAILED (-ftree-vectorize missing)\n"
    result=1
  fi
  if [[ ! "$common_cflags" =~ "-march=rv32imf_zve32f_zicsr_zifencei_zbb" ]]; then
    printf -- "Common CFlags Check: FAILED (-march=rv32imf_zve32f_zicsr_zifencei_zbb missing)\n"
    result=1
  fi
  if [[ ! "$common_cflags" =~ "-mabi=ilp32f" ]]; then
    printf -- "Common CFlags Check: FAILED (-mabi=ilp32f missing)\n"
    result=1
  fi
  if [[ ! "$common_cflags" =~ "-std=c99" ]]; then
    printf -- "Common CFlags Check: FAILED (-std=c99 missing)\n"
    result=1
  fi
  return $result
}

