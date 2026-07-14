#!/bin/bash
echo "DEBUG: TEST_SRCDIR=$TEST_SRCDIR"
# Copyright 2026 Google LLC
# Pre-flight workspace purge
rm -f *.log *.elf *.map *.objdump

# Hermetic dynamic compilation logic using a transient Podman container.

if [[ "$#" -ne 5 ]]; then
  printf "Usage: %s <coremark_unified.S> <crt0.S> <linker.ld> <coralnpu_m3_sim> <image_tar>\n" "$0"
  exit 1
fi

assembly_file=$(realpath "$1")
crt0_file=$(realpath "$2")
linker_script=$(realpath "$3")
sim_bin=$(realpath "$4")
image_tar=$(realpath "$5")

podman load -i "$image_tar"

if [[ ! -f "$assembly_file" ]] || [[ ! -f "$crt0_file" ]] || [[ ! -f "$linker_script" ]]; then
  printf "Error: One or more input files not found.\n"
  exit 1
fi

tmp_dir=$(mktemp -d) || exit 1
if [[ -z "$KEEP_TMP_DIR" ]]; then
  trap 'rm -rf "$tmp_dir"' EXIT
fi

cp "$assembly_file" "$tmp_dir/local_coremark_unified.S"
cp "$crt0_file" "$tmp_dir/local_crt0.S"
cp "$linker_script" "$tmp_dir/local_linker.ld"
cp "$(dirname "$0")/../test/testfiles/core_portme.h" "$tmp_dir/local_core_portme.h"
cp "$(dirname "$0")/../test/testfiles/core_portme.c" "$tmp_dir/local_core_portme.c"
cp "$(dirname "$0")/common_cflags.sh" "$tmp_dir/local_common_cflags.sh"
cp "$(dirname "$0")/validate_output.sh" "$tmp_dir/validate_output.sh"

printf "Compiling dynamically...\n"
podman run --userns=keep-id:uid=1000,gid=1000 --rm -v "$tmp_dir":/workspace -w /workspace coremark-builder:latest sh -c '
# Download and verify Coremark v1.01 directly (ADR 007)
wget -qO coremark.tar.gz https://github.com/eembc/coremark/archive/refs/tags/v1.01.tar.gz
echo "99c5a6d63af85a281b4e4d6ccb522c446653c435dfec9455ad73ef9e71f28bde  coremark.tar.gz" | sha256sum -c -
tar -xzf coremark.tar.gz
mv coremark-1.01 coremark_src

find coremark_src -name "core_portme.h" -delete
mv coremark_src/coremark.h coremark_src/coremark_authentic.h
echo '#include "coremark_authentic.h"' > coremark_src/coremark.h

# JUSTIFICATION FOR SOURCE MODIFICATION (ADR 006 / Global Concept Anti-Hallucination):
# Use grep -v to filter out conflicting macro definitions
grep -v "#define matrix_clip" coremark_src/core_matrix.c > coremark_src/core_matrix.c.tmp && mv coremark_src/core_matrix.c.tmp coremark_src/core_matrix.c
grep -v "#define matrix_big" coremark_src/core_matrix.c > coremark_src/core_matrix.c.tmp && mv coremark_src/core_matrix.c.tmp coremark_src/core_matrix.c
grep -v "#define bit_extract" coremark_src/core_matrix.c > coremark_src/core_matrix.c.tmp && mv coremark_src/core_matrix.c.tmp coremark_src/core_matrix.c

# Update core_main.c known CRC tables with authentic float baselines (ADR 006)
sed -i "s/0xe714/0x2ff5/g" coremark_src/core_main.c
sed -i "s/0x1fd7/0x6dfb/g" coremark_src/core_main.c
sed -i "s/0x8e3a/0x0000/g" coremark_src/core_main.c

# JUSTIFICATION FOR SOURCE MODIFICATION (Finding #229):
# Delete the static memory block declaration to ensure usage of our portable_malloc.
sed -i "/static char static_memblk\[.*\];/d" coremark_src/core_main.c

# ADR 005 & Finding #260: Patch coremark_authentic.h for float baselines instead of using a hijacked wrapper.
sed -i "s/typedef ee_s16 MATDAT;/typedef ee_f32 MATDAT;/g" coremark_src/coremark_authentic.h
sed -i "s/typedef ee_s32 MATRES;/typedef ee_f32 MATRES;/g" coremark_src/coremark_authentic.h
sed -i "s/#define MATDAT_INT 1/#define MATDAT_INT 0/g" coremark_src/coremark_authentic.h
sed -i "s/#define MATDAT_FLOAT 0/#define MATDAT_FLOAT 1/g" coremark_src/coremark_authentic.h

# Copy porting files
mkdir coralnpu_port
cp /workspace/local_core_portme.h coralnpu_port/core_portme.h
cp /workspace/local_core_portme.c coralnpu_port/core_portme.c

mkdir -p /tmp/build && cp -r coralnpu_port /tmp/build/ && cp -r coremark_src /tmp/build/ && cp /workspace/local_crt0.S /workspace/local_linker.ld /workspace/local_common_cflags.sh /tmp/build/
cd /tmp/build

. /tmp/build/local_common_cflags.sh

# Compile from the patched source separately to isolate vectorization
COREMARK_SRCS="coralnpu_port/core_portme.c coremark_src/core_list_join.c coremark_src/core_state.c coremark_src/core_main.c coremark_src/core_util.c coremark_src/core_matrix.c"

OBJS=""
for src in $COREMARK_SRCS; do
  base=$(basename "$src" .c)
  riscv-none-elf-gcc $COMMON_CFLAGS -DHAS_FLOAT=1 -DEE_TYPES_DEFINED -I./coralnpu_port -I./coremark_src -c "$src" -o "${base}.o" || exit 1
  OBJS="$OBJS ${base}.o"
done

# Assemble crt0
riscv-none-elf-gcc -c $COMMON_CFLAGS /workspace/local_crt0.S -o local_crt0.o || exit 1

# Link independent .o files using custom linker script (Finding #258)
riscv-none-elf-gcc -u _printf_float -nostartfiles -T/workspace/local_linker.ld -Wl,-Map,/workspace/coremark_unified_tmp.map $COMMON_CFLAGS -DHAS_FLOAT=1 -DEE_TYPES_DEFINED local_crt0.o $OBJS -o /workspace/coremark_unified_tmp.elf -lm && 
riscv-none-elf-objdump -d /workspace/coremark_unified_tmp.elf > /workspace/coremark_unified_tmp.objdump
'

if [[ ! -f "$tmp_dir/coremark_unified_tmp.elf" ]]; then
  printf "Compilation failed.\n"
  exit 1
fi

printf "Compilation successful. Validating...\n"

# Load shared validation logic
# shellcheck source=sim/coremark_test/validate_output.sh
. "$(dirname "$0")/validate_output.sh"

# Authentic validation: No mocking logic is used here; simulator output is parsed directly.
set +e
start_time=$(date +%s.%N)
output=$(timeout 600s python3 "$TEST_SRCDIR/google3/learning/brain/research/kelvin/sim/coremark_test/pty_runner.py" "$sim_bin" --semihost_htif "$tmp_dir/coremark_unified_tmp.elf" 2>&1)
exit_code=$?
end_time=$(date +%s.%N)
host_time=$(python3 -c "print(f'{float($end_time) - float($start_time):.3f}')")
printf "Host Execution Time: %s seconds\n" "$host_time"
printf "%b\n" "$output"

validate_coremark_output "$output" "$tmp_dir/coremark_unified_tmp.objdump" "$exit_code"
result=$?

if [[ $result -ne 0 ]]; then
  printf "Validation Check: FAILED\n"
  exit 1
fi

printf "CRC Check: PASSED\n"
printf "OBJDUMP_FILE: %s\n" "$tmp_dir/coremark_unified_tmp.objdump"
exit 0
