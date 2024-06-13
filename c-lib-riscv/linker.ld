MEMORY {
  ram (rwx) : org = 0x80000000, len = 128M
}

PROVIDE(__stack_top$ = ORIGIN(ram) + LENGTH(ram) - (8 * 16K));

SECTIONS {
    . = ORIGIN(ram);

    .text : {
        KEEP(*(.text.crt0));
        *(.text*);
    }

    .rodata :  {
        . = ALIGN(0x10);
        *(.rodata .rodata*);
    }

    .data : {
        . = ALIGN(0x1000);
        __DATA_BEGIN__ = .;
        *(.data .data*);
    }

    .sdata : {
        . = ALIGN(0x10);
        __SDATA_BEGIN__ = .;
        *(.srodata .srodata*);
        *(.sdata .sdata*);
    }

    .sbss : {
        . = ALIGN(0x100);
        *(.sbss .sbss*);
    }

    .bss : {
        . = ALIGN(0x100);
        *(.bss .bss.*)
    }
    __BSS_END__ = .;

    __global_pointer$ = MIN(__SDATA_BEGIN__ + 0x800,
                        MAX(__DATA_BEGIN__ + 0x800,
                            __BSS_END__ - 0x800));
}
