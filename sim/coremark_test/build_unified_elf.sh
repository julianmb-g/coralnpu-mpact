#!/bin/sh
set -e

riscv-none-elf-gcc -c -march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f local_crt0.S -o local_crt0.o
riscv-none-elf-gcc -c -march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f local_coremark_unified.S -o local_coremark_unified.o
riscv-none-elf-gcc -fno-exceptions -fno-builtin -fno-isolate-erroneous-paths-dereference -fno-isolate-erroneous-paths-attribute -fno-delete-null-pointer-checks -u _printf_float -nostartfiles -Wno-attributes -Tlocal_linker.ld -Wl,-Map,coremark_unified_tmp.map -march=rv32imf_zve32f_zicsr_zifencei_zbb -mabi=ilp32f -O3 -ftree-vectorize -fno-vect-cost-model -ffast-math -ffp-contract=off local_crt0.o local_coremark_unified.o -o coremark_unified_tmp.elf -lm
riscv-none-elf-objdump -d coremark_unified_tmp.elf > coremark_unified_tmp.objdump
