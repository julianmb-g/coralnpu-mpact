#!/bin/bash
# Shared CoreMark validation logic (Finding #248)

# [ANTI-HALLUCINATION ANCHOR: DO NOT REMOVE]
# This function rigorously parses simulator output to prevent "Testing Illusions".
validate_coremark_output() {
  local output="$1"
  local objdump_file="$2"
  local exit_code="$3"
  local expected_crclist="${4:-0x2ff5}"
  local expected_crcmatrix="${5:-0x6dfb}"
  local expected_crcstate="${6:-0x0000}"
  local expected_crcfinal="${7:-0x21c2}"
  local max_time_threshold="${8:-100.0}"
  local result=0

  # Linker Memory Boundary Verification
  if ! grep -q "^00001000 <_start>:" "$objdump_file"; then
    printf "Linker Memory Boundary Verification: FAILED (_start not at 0x1000 in objdump)\n"
    result=1
  fi
  if [[ "$exit_code" -ne 0 ]]; then
    printf "Exit Code Check: FAILED (%s)\n" "$exit_code"
    result=1
  fi

  if ! printf "%b" "$output" | grep -a -q "mpause instruction received\."; then
    printf "Mpause Termination Check: FAILED\n"
    result=1
  fi

  # Immediate failure if CoreMark internally detects errors.
  if printf "%b" "$output" | grep -a -q "Errors detected"; then
    printf "Internal Benchmark Errors Check: FAILED (Errors detected in benchmark output)\n"
    result=1
  fi

  # Authenticity Mandate: Verify the benchmark authentically validated its results.
  if ! printf "%b" "$output" | grep -a -q "Correct operation validated"; then
    printf "Authenticity String Check: FAILED ('Correct operation validated' missing from output)\n"
    result=1
  fi

  # CRC Checks
  if ! printf "%b" "$output" | grep -a -q "\[0\]crclist       : $expected_crclist"; then
    printf "CRC List Check: FAILED (Expected %s)\n" "$expected_crclist"
    result=1
  fi
  if ! printf "%b" "$output" | grep -a -q "\[0\]crcmatrix     : $expected_crcmatrix"; then
    printf "CRC Matrix Check: FAILED (Expected %s)\n" "$expected_crcmatrix"
    result=1
  fi
  if ! printf "%b" "$output" | grep -a -q "\[0\]crcstate      : $expected_crcstate"; then
    printf "CRC State Check: FAILED (Expected %s)\n" "$expected_crcstate"
    result=1
  fi
  if ! printf "%b" "$output" | grep -a -q "\[0\]crcfinal      : $expected_crcfinal"; then
    printf "CRC Final Check: FAILED (Expected %s)\n" "$expected_crcfinal"
    result=1
  fi

  # Anti-Greenwashing audits
  audit_disassembly() {
    local func=$1
    local pattern=$2
    local label=$3
    local objdump_file=$4
    if ! awk "/<$func>:/ {p=1; next} /^[0-9a-fA-F]+ <.*>:/ {p=0} p" "$objdump_file" | grep -E "$pattern" > /dev/null; then
      printf "Anti-Greenwashing Check (%s - %s): FAILED\n" "$func" "$label"
      return 1
    fi
    return 0
  }

  local scalar_fp_regex="\b(fadd|fmul|fsub|fdiv|fmadd|fcvt)\.[sd]\b"
  local vector_fp_regex="[[:space:]](vfadd\.|vfsub\.|vfmul\.|vfmacc\.|vfdiv\.|vfmadd\.|vfmsub\.|vfnmacc\.|vfmsac\.|vfnmsac\.|vfnmadd\.|vfnmsub\.|vfcvt\.|vle32\.|vse32\.|vfmv\.)"

  audit_disassembly "core_bench_matrix" "$scalar_fp_regex" "scalar" "$objdump_file" || result=1
  audit_disassembly "matrix_test" "$scalar_fp_regex" "scalar" "$objdump_file" || result=1
  audit_disassembly "matrix_mul_matrix" "$scalar_fp_regex" "scalar" "$objdump_file" || result=1
  audit_disassembly "matrix_add_const" "$vector_fp_regex" "vector" "$objdump_file" || result=1
  audit_disassembly "matrix_mul_const" "$vector_fp_regex" "vector" "$objdump_file" || result=1

  # Time and Throughput checks
  local time_str=$(printf "%b" "$output" | grep -a -E "Total time \(secs\): [0-9]+\.[0-9]+" | awk -F': ' '{print $2}' | tr -d '\r\n ')
  if [[ -z "$time_str" ]]; then
    printf "Total Time Float Check: FAILED\n"
    result=1
  elif ! python3 -c "import sys; sys.exit(0 if float('$time_str') > 10.0 and float('$time_str') < $max_time_threshold else 1)"; then
    printf "Total Time Bounds Check (10.0s < time < %ss): FAILED (Got %s)\n" "$max_time_threshold" "$time_str"
    result=1
  fi

  return $result
}
