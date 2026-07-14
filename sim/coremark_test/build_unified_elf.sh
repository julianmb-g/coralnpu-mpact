#!/bin/bash
set -e

# Arguments:
# $1: location of common_cflags.sh

common_cflags_file=$1
# shellcheck source=sim/coremark_test/common_cflags.sh
. "$common_cflags_file"

riscv-none-elf-gcc -c $COMMON_CFLAGS local_crt0.S -o local_crt0.o
riscv-none-elf-gcc -c $COMMON_CFLAGS local_coremark_unified.S -o local_coremark_unified.o
riscv-none-elf-gcc -u _printf_float -nostartfiles -Tlocal_linker.ld -Wl,-Map,coremark_unified_tmp.map $COMMON_CFLAGS local_crt0.o local_coremark_unified.o -o coremark_unified_tmp.elf -lm
riscv-none-elf-objdump -d coremark_unified_tmp.elf > coremark_unified_tmp.objdump
