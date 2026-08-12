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

linker_ld=$1
crt0_s=$2
coremark_unified_s=$3
build_unified_elf=$4
common_cflags=$5
out_elf=$6
out_map=$7
out_objdump=$8

trap 'rm -f local_linker.ld local_crt0.S local_coremark_unified.S local_build_unified_elf.sh local_common_cflags.sh' EXIT

cp -L "$linker_ld" local_linker.ld
cp -L "$crt0_s" local_crt0.S
cp -L "$coremark_unified_s" local_coremark_unified.S
cp -L "$build_unified_elf" local_build_unified_elf.sh
cp -L "$common_cflags" local_common_cflags.sh

podman run --userns=keep-id:uid=1000,gid=1000 --rm -v "$(pwd)":/workspace -w /workspace coremark-builder:latest sh local_build_unified_elf.sh local_common_cflags.sh

mv coremark_unified_tmp.elf "$out_elf"
mv coremark_unified_tmp.map "$out_map"
mv coremark_unified_tmp.objdump "$out_objdump"
