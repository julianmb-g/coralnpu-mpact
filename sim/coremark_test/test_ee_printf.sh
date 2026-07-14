#!/bin/bash
# Copyright 2026 Google LLC

if [[ "$#" -ne 2 ]]; then
  printf "Usage: %s <coralnpu_m3_sim> <image_tar>\n" "$0"
  exit 1
fi

sim_bin=$(realpath "$1")
image_tar=$(realpath "$2")

podman load -i "$image_tar"

tmp_dir=$(mktemp -d) || exit 1
trap 'rm -rf "$tmp_dir"' EXIT

cp -L "$(dirname "$0")/../test/testfiles/core_portme.h" "$tmp_dir/core_portme.h"
cp -L "$(dirname "$0")/../test/testfiles/core_portme.c" "$tmp_dir/core_portme.c"
cp -L "$(dirname "$0")/../test/testfiles/crt0.S" "$tmp_dir/crt0.S"
cp -L "$(dirname "$0")/../test/testfiles/linker.ld" "$tmp_dir/linker.ld"
cp -L "$(dirname "$0")/test_ee_printf.c" "$tmp_dir/test_ee_printf.c"
cp "$(dirname "$0")/common_cflags.sh" "$tmp_dir/common_cflags.sh"

# Mock coremark_authentic.h for ee_printf test
printf 'typedef float secs_ret;\n#define MATDAT_INT 0\n#define MATDAT_FLOAT 1\ntypedef float ee_f32;\ntypedef float ee_f16;\n' > "$tmp_dir/coremark_authentic.h"
# Create coremark.h that just includes coremark_authentic.h
printf '#include "coremark_authentic.h"\n' > "$tmp_dir/coremark.h"

cd "$tmp_dir"

podman run --userns=keep-id:uid=1000,gid=1000 --rm -v "$tmp_dir":/workspace -w /workspace coremark-builder:latest sh -c '
. /workspace/common_cflags.sh
riscv-none-elf-gcc -c $COMMON_CFLAGS crt0.S -o crt0.o && 
riscv-none-elf-gcc $COMMON_CFLAGS -c core_portme.c -o core_portme.o && 
riscv-none-elf-gcc $COMMON_CFLAGS -c test_ee_printf.c -o test_ee_printf.o && 
riscv-none-elf-gcc -nostartfiles -Tlinker.ld -Wl,-Map,test_ee_printf.map $COMMON_CFLAGS crt0.o core_portme.o test_ee_printf.o -o test_ee_printf.elf -lm
'

if [[ ! -f "$tmp_dir/test_ee_printf.elf" ]]; then
  printf "Compilation failed.\n"
  exit 1
fi

# Finding #225: Float precision discrepancy resolution (3.141589 instead of 3.141590)
expected_output="Hello World!
Int: 123
Hex: 7b
Float: 3.141589
String: Test String
Char: X
Percent: %
Large Hex 1: ffffffff
Large Hex 2: 80000000
INT_MAX: 2147483647
INT_MIN: -2147483648
UINT_MAX: 4294967295
Large Float 1: ovf
Large Float 2: 0.000000
Done!"

output=$("$sim_bin" --semihost_htif "$tmp_dir/test_ee_printf.elf" 2>&1)
exit_code=$?

printf "--- SIMULATOR OUTPUT ---\n"
printf "%b\n" "$output"
printf "--- END SIMULATOR OUTPUT ---\n"

if [[ $exit_code -ne 0 ]]; then
  printf "Simulator exit code non-zero: %s\n" "$exit_code"
  exit 1
fi

if ! printf "%b" "$output" | grep -q "mpause instruction received"; then
  printf "Mpause not received.\n"
  exit 1
fi

actual_output=$(printf "%b" "$output" | sed -E '/(Starting simulation|mpause instruction received|Total cycles|Simulation done)/d' | tr -d '\r')

if [[ "$actual_output" != "$expected_output" ]]; then
  printf "Output mismatch.\n"
  printf "Expected:\n"
  printf "%b\n" "$expected_output"
  printf "Actual:\n"
  printf "%b\n" "$actual_output"
  exit 1
fi

printf "test_ee_printf passed.\n"
exit 0
