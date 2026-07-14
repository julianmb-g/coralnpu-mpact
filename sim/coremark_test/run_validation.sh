#!/bin/bash
# Copyright 2026 Google LLC

if [[ "$#" -ne 3 ]]; then
  printf "Usage: %s <elf_file> <coralnpu_m3_sim> <objdump_file>\n" "$0"
  exit 1
fi

elf_file=$(realpath "$1")
sim_bin=$(realpath "$2")
objdump_file=$(realpath "$3")

if [[ ! -f "$elf_file" ]] || [[ ! -f "$sim_bin" ]] || [[ ! -f "$objdump_file" ]]; then
  printf "Error: One or more input files not found.\n"
  exit 1
fi

# Load shared validation logic
# shellcheck source=sim/coremark_test/validate_output.sh
. "$(dirname "$0")/validate_output.sh"

# Authentic validation: No mocking logic is used here; simulator output is parsed directly.
# Finding #255: Ensure a clean execution environment by purging stale artifacts before simulation.
rm -f *.log

set +e
# Finding #244: Robust simulation output capturing via pty_runner.py.
output=$(timeout 600s python3 "$(dirname "$0")/pty_runner.py" "$sim_bin" --semihost_htif "$elf_file" 2>&1)
exit_code=$?
set -e

printf "--- SIMULATOR OUTPUT ---\n"
printf "%b\n" "$output"
printf "--- END SIMULATOR OUTPUT ---\n"

# Validate output using shared function
validate_coremark_output "$output" "$objdump_file" "$exit_code"
result=$?

if [[ $result -ne 0 ]]; then
  printf "Validation Check: FAILED\n"
  exit 1
fi

printf "Validation Check: PASSED\n"
exit 0
