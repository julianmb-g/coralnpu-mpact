OUTPUT_ARCH("riscv")
ENTRY(_start)

MEMORY {
    RAM (rwx) : ORIGIN = 0x1000, LENGTH = 0x8000000
}

SECTIONS {
    . = 0x1000;
    
    .text : {
        *(.text.init)
        *(.text .text.*)
    } > RAM

    .tohost : {
        . = ALIGN(64);
        *(.tohost)
        . = ALIGN(64);
    } > RAM

    .rodata : {
        *(.rodata .rodata.*)
    } > RAM

    .data : {
        *(.data .data.*)
        . = ALIGN(8);
        PROVIDE( __global_pointer$ = . + 0x800 );
        *(.sdata .sdata.*)
    } > RAM

    .bss : {
        . = ALIGN(8);
        PROVIDE(__BSS_START__ = .); __bss_start = .;
        *(.sbss .sbss.*)
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(8);
        PROVIDE(__BSS_END__ = .); _end = .;
    } > RAM

    .heap : {
        . = ALIGN(16);
        __heap_start = .;
        . += 0x100000;
        __heap_end = .;
    } > RAM

    .stack : {
        . = ALIGN(16);
        . += 0x100000;
        _stack_top = .;
    } > RAM
}
