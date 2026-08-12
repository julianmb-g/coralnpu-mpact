# Copyright 2026 Google LLC
OUTPUT_ARCH("riscv")
ENTRY(_start)

MEMORY {
    text_region (rx) : ORIGIN = 0x1000, LENGTH = 0x4000000
    data_region (rw) : ORIGIN = 0x4001000, LENGTH = 0x4000000
}

SECTIONS {
    . = 0x1000;
    
    .text : {
        *(.text.init)
        *(.text .text.*)
    } > text_region

    .tohost : {
        . = ALIGN(64);
        *(.tohost)
        . = ALIGN(64);
    } > data_region

    .rodata : {
        *(.rodata .rodata.*)
    } > text_region

    .data : {
        *(.data .data.*)
        . = ALIGN(8);
        PROVIDE( __global_pointer$ = . + 0x800 );
        *(.sdata .sdata.*)
    } > data_region

    .bss : {
        . = ALIGN(8);
        PROVIDE(__BSS_START__ = .); __bss_start = .;
        *(.sbss .sbss.*)
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(8);
        PROVIDE(__BSS_END__ = .); _end = .;
    } > data_region

    .heap : {
        . = ALIGN(16);
        __heap_start = .;
        . += 0x100000;
        __heap_end = .;
    } > data_region

    .stack : {
        . = ALIGN(16);
        . += 0x100000;
        _stack_top = .;
    } > data_region
}
