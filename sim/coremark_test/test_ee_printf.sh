#!/bin/bash
# Copyright 2026 Google LLC

if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <coralnpu_m3_sim> <image_tar>"
  exit 1
fi

SIM_BIN=$(realpath "$1")
IMAGE_TAR=$(realpath "$2")

podman load -i "$IMAGE_TAR"

TMP_DIR=$(mktemp -d) || exit 1
trap 'rm -rf "$TMP_DIR"' EXIT

cp -L "$(dirname "$0")/../test/testfiles/core_portme.h" "$TMP_DIR/core_portme.h"
cp -L "$(dirname "$0")/../test/testfiles/core_portme.c" "$TMP_DIR/core_portme.c"
cat << 'EOF' > "$TMP_DIR/coremark.h"
#ifndef CORALNPU_COREMARK_H
#define CORALNPU_COREMARK_H

#if defined(__riscv) || defined(__riscv__)
#include "coremark_authentic.h"
#else
typedef float secs_ret;
#endif

#if HAS_FLOAT
#undef MATDAT
#define MATDAT ee_f32
#undef MATRES
#define MATRES ee_f32
#endif

#endif
EOF
cp -L "$(dirname "$0")/../test/testfiles/crt0.S" "$TMP_DIR/crt0.S"
cp -L "$(dirname "$0")/../test/testfiles/linker.ld" "$TMP_DIR/linker.ld"
cp -L "$(dirname "$0")/test_ee_printf.c" "$TMP_DIR/test_ee_printf.c"
echo 'typedef float secs_ret;' > "$TMP_DIR/coremark_authentic.h"

cd "$TMP_DIR"

podman run --userns=keep-id:uid=1000,gid=1000 --rm -v "$TMP_DIR":/workspace -w /workspace coremark-builder:latest sh -c '
riscv-none-elf-gcc -c -march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f crt0.S -o crt0.o && 
riscv-none-elf-gcc -fno-exceptions -fno-builtin -Wno-attributes -march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f -O3 -c core_portme.c -o core_portme.o && 
riscv-none-elf-gcc -fno-exceptions -fno-builtin -Wno-attributes -march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f -O3 -c test_ee_printf.c -o test_ee_printf.o && 
riscv-none-elf-gcc -nostartfiles -Tlinker.ld -Wl,-Map,test_ee_printf.map -march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f crt0.o core_portme.o test_ee_printf.o -o test_ee_printf.elf -lm
'

if [ ! -f "$TMP_DIR/test_ee_printf.elf" ]; then
  echo "Compilation failed."
  exit 1
fi

EXPECTED_OUTPUT="Hello World!
Int: 123
Hex: 7b
Float: 3.141590
String: Test String
Char: X
Percent: %
Done!"

OUTPUT=$("$SIM_BIN" --semihost_htif "$TMP_DIR/test_ee_printf.elf" 2>&1)
EXIT_CODE=$?

echo "--- SIMULATOR OUTPUT ---"
echo -e "$OUTPUT"
echo "--- END SIMULATOR OUTPUT ---"

if [ $EXIT_CODE -ne 0 ]; then
  echo "Simulator exit code non-zero: $EXIT_CODE"
  exit 1
fi

if ! echo -e "$OUTPUT" | grep -q "mpause instruction received"; then
  echo "Mpause not received."
  exit 1
fi

ACTUAL_OUTPUT=$(echo -e "$OUTPUT" | sed -E '/(Starting simulation|mpause instruction received|Total cycles|Simulation done)/d' | tr -d '\r')

if [ "$ACTUAL_OUTPUT" != "$EXPECTED_OUTPUT" ]; then
  echo "Output mismatch."
  echo "Expected:"
  echo -e "$EXPECTED_OUTPUT"
  echo "Actual:"
  echo -e "$ACTUAL_OUTPUT"
  exit 1
fi

echo "test_ee_printf passed."
exit 0
