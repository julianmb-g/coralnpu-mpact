#!/bin/bash
set -e

# Strict Workspace Boundary Mandate: This script runs *inside* the Podman container.
# It copies artifacts to /workspace, which is mapped to the host's sim/test/testfiles.

COREMARK_DIR="/opt/coremark"
BUILD_DIR="/tmp/build"
OUTPUT_DIR="/workspace"

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Copy CoreMark source to build directory
cp -r "${COREMARK_DIR}/"* "${BUILD_DIR}/"

# Apply patches for float support
# Sed commands from DESIGN.md ADR 006
sed -i 's/CRC_C   (0x5e46)/CRC_C   (0x69f8)/' core_main.c
sed -i 's/CRC_AL  (0xeffa)/CRC_AL  (0x546a)/' core_main.c
sed -i 's/CRC_P   (0x42f0)/CRC_P   (0xaf43)/' core_main.c
sed -i '/#define matrix_clip/d' core_matrix.c
sed -i '/#define matrix_big/d' core_matrix.c
sed -i '/#define bit_extract/d' core_matrix.c
sed -i 's/#define matrix_test(N, A, B, res) matrix_test_next(N, A, B, res)/#define matrix_test(N, A, B, res) matrix_test_int(N, A, B, res)/' core_matrix.c
grep -v "extern MATRES matrix_mul_const(MDAT *A, MDAT B, MATRES *RES);" core_matrix.c > core_matrix.c.tmp && mv core_matrix.c.tmp core_matrix.c
grep -v "extern MATRES matrix_add_const(MDAT *A, MDAT B, MATRES *RES);" core_matrix.c > core_matrix.c.tmp && mv core_matrix.c.tmp core_matrix.c

# Rename authentic coremark.h to allow overrides
mv coremark.h coremark_authentic.h

# Surgically modify types in authentic header to force 32-bit floats
sed -i 's/typedef ee_s16 MATDAT;/typedef ee_f32 MATDAT;/' coremark_authentic.h
sed -i 's/typedef ee_s32 MATRES;/typedef ee_f32 MATRES;/' coremark_authentic.h

# Create simple wrapper coremark.h
cat << 'EOF' > coremark.h
#ifndef CORALNPU_COREMARK_H
#define CORALNPU_COREMARK_H
#include "coremark_authentic.h"
#endif
EOF

# Set up build flags
MARCH="rv32imf_zve32f_zicsr_zifencei_zbb"
MABI="ilp32f"
CFLAGS="-O3 -ftree-vectorize -march=${MARCH} -mabi=${MABI} -I./ -DHAS_FLOAT=1 -DITERATIONS=20000"

# Copy port files to build directory for unified compilation
cp "${OUTPUT_DIR}/core_portme.c" "${BUILD_DIR}/"
cp "${OUTPUT_DIR}/core_portme.h" "${BUILD_DIR}/"

# Create unified wrapper
cat << 'EOF' > unified_wrapper.c
#include "core_main.c"
#include "core_matrix.c"
#include "core_list_join.c"
#include "core_state.c"
#include "core_util.c"
#include "core_portme.c"
EOF

# Unified Assembly Generation
riscv-none-elf-gcc ${CFLAGS} -S -o "${BUILD_DIR}/coremark_unified.S" unified_wrapper.c

# Individual Assembly Generation
riscv-none-elf-gcc ${CFLAGS} -c core_main.c -o core_main.o
riscv-none-elf-gcc ${CFLAGS} -S core_main.c -o core_main.S

riscv-none-elf-gcc ${CFLAGS} -c core_matrix.c -o core_matrix.o
riscv-none-elf-gcc ${CFLAGS} -S core_matrix.c -o core_matrix.S

riscv-none-elf-gcc ${CFLAGS} -c core_list_join.c -o core_list_join.o
riscv-none-elf-gcc ${CFLAGS} -S core_list_join.c -o core_list_join.S

riscv-none-elf-gcc ${CFLAGS} -c core_state.c -o core_state.o
riscv-none-elf-gcc ${CFLAGS} -S core_state.c -o core_state.S

riscv-none-elf-gcc ${CFLAGS} -c core_util.c -o core_util.o
riscv-none-elf-gcc ${CFLAGS} -S core_util.c -o core_util.S

riscv-none-elf-gcc ${CFLAGS} -c "${OUTPUT_DIR}/core_portme.c" -o core_portme.o
riscv-none-elf-gcc ${CFLAGS} -S "${OUTPUT_DIR}/core_portme.c" -o core_portme.S

# Apply assembly formatting
for asm_file in $(find . -name "*.S"); do
  sed -i -E 's/^[ 	]+//g' "${asm_file}" # Remove all leading whitespace
  sed -i -E '/^\s*[a-zA-Z0-9_]+:/! s/^/  /' "${asm_file}" # Add 2 spaces if not a label
  sed -i -E '/^\s*\./! s/^/  /' "${asm_file}" # Add 2 spaces if not a directive
  # Ensure labels and directives have no leading spaces
  sed -i -E 's/^  ([a-zA-Z0-9_]+:)/\1/' "${asm_file}"
  sed -i -E 's/^  (\..*)/\1/' "${asm_file}"
done

# Link transient ELF from unified assembly
riscv-none-elf-gcc ${CFLAGS} -nostartfiles -T "${OUTPUT_DIR}/linker.ld" "${OUTPUT_DIR}/crt0.S" "${BUILD_DIR}/coremark_unified.S" -o "${OUTPUT_DIR}/coremark_unified.elf"

# Link transient ELF from individual assembly (Validation Gate)
riscv-none-elf-gcc ${CFLAGS} -nostartfiles -T "${OUTPUT_DIR}/linker.ld" "${OUTPUT_DIR}/crt0.S" core_main.S core_matrix.S core_list_join.S core_state.S core_util.S core_portme.S -o "${OUTPUT_DIR}/coremark_individual.elf"

# Copy artifacts to output directory
cp "${BUILD_DIR}/coremark_unified.S" "${OUTPUT_DIR}/"

echo "Build complete. Artifacts in ${OUTPUT_DIR}"
