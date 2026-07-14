#!/bin/bash
# Copyright 2026 Google LLC

# Hermetic dynamic compilation and execution logic utilizing newlib's standard crt0

if [[ "$#" -ne 4 ]]; then
  printf "Usage: %s <coremark_unified.S> <linker.ld> <coralnpu_m3_sim> <image_tar>\n" "$0"
  exit 1
fi

assembly_file=$(realpath "$1")
linker_script=$(realpath "$2")
sim_bin=$(realpath "$3")
image_tar=$(realpath "$4")

podman load -i "$image_tar"

if [[ ! -f "$assembly_file" ]] || [[ ! -f "$linker_script" ]]; then
  printf "Error: One or more input files not found.\n"
  exit 1
fi

tmp_dir=$(mktemp -d) || exit 1
trap 'rm -rf "$tmp_dir"' EXIT

cp "$assembly_file" "$tmp_dir/local_coremark_unified.S"
cp "$linker_script" "$tmp_dir/local_linker.ld"
cp "$(dirname "$0")/common_cflags.sh" "$tmp_dir/local_common_cflags.sh"

printf "Compiling dynamically with newlib crt0...\n"
podman run --userns=keep-id:uid=1000,gid=1000 --rm -v "$tmp_dir":/workspace -w /workspace coremark-builder:latest sh -c '
. /workspace/local_common_cflags.sh
riscv-none-elf-gcc -c $COMMON_CFLAGS local_coremark_unified.S -o local_coremark_unified.o &&
riscv-none-elf-gcc -u _printf_float -Tlocal_linker.ld -Wl,-Map,/workspace/coremark_unified_tmp.map $COMMON_CFLAGS local_coremark_unified.o -o /workspace/coremark_unified_tmp.elf -lm
'

if [[ ! -f "$tmp_dir/coremark_unified_tmp.elf" ]]; then
  printf "Compilation failed.\n"
  exit 1
fi

printf "Compilation successful. Performing dynamic verification of newlib crt0 incompatibility...\n"

# Perform simulation to confirm that stock newlib crt0 causes InstructionAccessFault
# because it does not properly initialize stack or vector state for CoralNPU M3.
# Finding #261: Add timeout to prevent hanging.
output=$(timeout 60s "$sim_bin" --semihost_htif "$tmp_dir/coremark_unified_tmp.elf" 2>&1)
exit_code=$?

printf "--- SIMULATOR OUTPUT ---\n"
printf "%b\n" "$output"
printf "--- END SIMULATOR OUTPUT ---\n"

# Finding #243: Stock newlib crt0 SHOULD fail to execute correctly.
if [[ $exit_code -eq 0 ]] && printf "%b" "$output" | grep -q "Correct operation validated"; then
  printf "Security/Compatibility Violation: Stock newlib crt0 unexpectedly succeeded.\n"
  exit 1
fi

if printf "%b" "$output" | grep -E -q "InstructionAccessFault|IllegalInstruction|MemoryAccessFault"; then
  printf "newlib_crt0_test PASSED: Correctly identified incompatibility (Trap detected).\n"
  exit 0
else
  printf "newlib_crt0_test FAILED: Stock newlib failed but not with an expected trap.\n"
  exit 1
fi

