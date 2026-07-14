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
cp -L "$(dirname "$0")/portable_malloc_test.c" "$tmp_dir/portable_malloc_test.c"
cp "$(dirname "$0")/common_cflags.sh" "$tmp_dir/common_cflags.sh"

# Mock coremark_authentic.h for portable_malloc test
printf 'typedef float secs_ret;\n#define MATDAT_INT 1\n#define MATDAT_FLOAT 0\ntypedef float ee_f32;\ntypedef float ee_f16;\n' > "$tmp_dir/coremark_authentic.h"
printf '#include "coremark_authentic.h"\n' > "$tmp_dir/coremark.h"

cd "$tmp_dir"

podman run --userns=keep-id:uid=1000,gid=1000 --rm -v "$tmp_dir":/workspace -w /workspace coremark-builder:latest sh -c '
. /workspace/common_cflags.sh
riscv-none-elf-gcc -c $COMMON_CFLAGS crt0.S -o crt0.o && 
riscv-none-elf-gcc $COMMON_CFLAGS -c core_portme.c -o core_portme.o && 
riscv-none-elf-gcc $COMMON_CFLAGS -c portable_malloc_test.c -o portable_malloc_test.o && 
riscv-none-elf-gcc -nostartfiles -Tlinker.ld -Wl,-Map,portable_malloc_test.map $COMMON_CFLAGS crt0.o core_portme.o portable_malloc_test.o -o portable_malloc_test.elf -lm
'

if [[ ! -f "$tmp_dir/portable_malloc_test.elf" ]]; then
  printf "Compilation failed.\n"
  exit 1
fi

output=$("$sim_bin" --semihost_htif "$tmp_dir/portable_malloc_test.elf" 2>&1)
exit_code=$?

printf "--- SIMULATOR OUTPUT ---\n"
printf "%b\n" "$output"
printf "--- END SIMULATOR OUTPUT ---\n"

if [[ $exit_code -ne 0 ]]; then
  printf "Simulator exit code non-zero: %s\n" "$exit_code"
  exit 1
fi

if printf "%b" "$output" | grep -q "portable_malloc tests passed"; then
  printf "portable_malloc_test passed.\n"
  exit 0
else
  printf "portable_malloc_test failed.\n"
  exit 1
fi
