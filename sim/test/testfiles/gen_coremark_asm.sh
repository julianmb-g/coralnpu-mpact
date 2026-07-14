#!/bin/bash
set -e

# Arguments:
# $1: location of core_portme.h
# $2: location of core_portme.c
# $3: location of build_unified_asm.sh
# $4: location of output coremark_unified.S
# $5: location of common_cflags.sh
# $6: location of format_asm.sh
# $7: location of crt0.S
# $8: location of linker.ld

CORE_PORTME_H=$1
CORE_PORTME_C=$2
BUILD_UNIFIED_ASM=$3
OUT_FILE=$4
COMMON_CFLAGS=$5
FORMAT_ASM=$6
CRT0_S=$7
LINKER_LD=$8

trap 'rm -f local_core_portme.h local_core_portme.c local_build_unified_asm.sh local_common_cflags.sh out_coremark_unified.S formatted_unified.S local_format_asm.sh local_crt0.S local_linker.ld' EXIT

cp -L "$CORE_PORTME_H" local_core_portme.h
cp -L "$CORE_PORTME_C" local_core_portme.c
cp -L "$BUILD_UNIFIED_ASM" local_build_unified_asm.sh
cp -L "$COMMON_CFLAGS" local_common_cflags.sh
cp -L "$FORMAT_ASM" local_format_asm.sh
cp -L "$CRT0_S" local_crt0.S
cp -L "$LINKER_LD" local_linker.ld

podman run --userns=keep-id:uid=1000,gid=1000 --rm -v $(pwd):/workspace coremark-builder:latest sh /workspace/local_build_unified_asm.sh

sh local_format_asm.sh out_coremark_unified.S formatted_unified.S

mv formatted_unified.S "$OUT_FILE"
