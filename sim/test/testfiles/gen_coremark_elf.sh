#!/bin/bash
set -e

# Arguments:
# $1: location of linker.ld
# $2: location of crt0.S
# $3: location of coremark_unified.S
# $4: location of build_unified_elf.sh
# $5: location of common_cflags.sh
# $6: location of output coremark_unified.elf
# $7: location of output coremark_unified.map
# $8: location of output coremark_unified.objdump

LINKER_LD=$1
CRT0_S=$2
COREMARK_UNIFIED_S=$3
BUILD_UNIFIED_ELF=$4
COMMON_CFLAGS=$5
OUT_ELF=$6
OUT_MAP=$7
OUT_OBJDUMP=$8

trap 'rm -f local_linker.ld local_crt0.S local_coremark_unified.S local_build_unified_elf.sh local_common_cflags.sh' EXIT

cp -L "$LINKER_LD" local_linker.ld
cp -L "$CRT0_S" local_crt0.S
cp -L "$COREMARK_UNIFIED_S" local_coremark_unified.S
cp -L "$BUILD_UNIFIED_ELF" local_build_unified_elf.sh
cp -L "$COMMON_CFLAGS" local_common_cflags.sh

podman run --userns=keep-id:uid=1000,gid=1000 --rm -v $(pwd):/workspace -w /workspace coremark-builder:latest sh local_build_unified_elf.sh local_common_cflags.sh

mv coremark_unified_tmp.elf "$OUT_ELF"
mv coremark_unified_tmp.map "$OUT_MAP"
mv coremark_unified_tmp.objdump "$OUT_OBJDUMP"
