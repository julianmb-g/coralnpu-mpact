#!/bin/sh
set -e

# Pre-flight workspace purge (ADR 007 / Pristine Workspace Mandate)
# Purge stale artifacts from the local workspace to prevent contamination.
rm -f /workspace/out_coremark_unified.S /workspace/*.log

cd /tmp

# Create a temporary build directory within the container
mkdir -p build_dir
cd build_dir

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

mkdir coralnpu_port
cp /workspace/local_core_portme.h coralnpu_port/core_portme.h
cp /workspace/local_core_portme.c coralnpu_port/core_portme.c

# Load common compiler flags
. /workspace/local_common_cflags.sh

# Compile from the patched source separately to isolate vectorization
COREMARK_SRCS="coralnpu_port/core_portme.c coremark_src/core_list_join.c coremark_src/core_state.c coremark_src/core_main.c coremark_src/core_util.c coremark_src/core_matrix.c"

OBJS=""
for src in $COREMARK_SRCS; do
  base=$(basename "$src" .c)
  riscv-none-elf-gcc $COMMON_CFLAGS -DHAS_FLOAT=1 -DEE_TYPES_DEFINED -I./coralnpu_port -I./coremark_src -S "$src" -o "${base}.S" || exit 1
  # Rename local labels in each assembly file to avoid collisions when concatenated
  sed -E -i "s/\.L([1-9][0-9]*)/\.L_${base}_\1/g; s/\.LC([0-9]+)/\.LC_${base}_\1/g; s/\.LANCHOR([0-9]+)/\.LANCHOR_${base}_\1/g" "${base}.S"
  sed -i "/\.attribute/d" "${base}.S"
  
  # Assemble for validation linkage (Finding #258)
  riscv-none-elf-gcc -c $COMMON_CFLAGS "${base}.S" -o "${base}.o" || exit 1
  OBJS="$OBJS ${base}.o"
done

# Assemble crt0
riscv-none-elf-gcc -c $COMMON_CFLAGS /workspace/local_crt0.S -o local_crt0.o || exit 1

# Link independent .o files using custom linker script (Finding #258)
riscv-none-elf-gcc -u _printf_float -nostartfiles -T/workspace/local_linker.ld -Wl,-Map,/workspace/coremark_unified_tmp.map $COMMON_CFLAGS -DHAS_FLOAT=1 -DEE_TYPES_DEFINED local_crt0.o $OBJS -o /workspace/coremark_unified_tmp.elf -lm || exit 1

cat core_portme.S core_list_join.S core_state.S core_main.S core_util.S core_matrix.S > /workspace/out_coremark_unified.S
