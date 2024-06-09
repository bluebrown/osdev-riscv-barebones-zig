SECTIONS {
    . = 0x80000000;

    .text ALIGN(16) : {
        *(.text .text*);
    }

    .rodata ALIGN(16) :  {
        *(.rodata .rodata*);
    }

    .data ALIGN(16) : {
        __DATA_BEGIN__ = .;
        *(.data .data*);
    }

    .sdata ALIGN(16) : {
        __SDATA_BEGIN__ = .;
        *(.srodata .srodata*);
        *(.sdata .sdata*);
    }

    .sbss ALIGN(16) : {
        *(.sbss .sbss*);
    }

    .bss ALIGN(64) : {
        *(.bss .bss.*)
    }
    __BSS_END__ = .;

    __global_pointer$ = MIN(__SDATA_BEGIN__ + 0x800,
                        MAX(__DATA_BEGIN__ + 0x800,
                            __BSS_END__ - 0x800));

    __stack_top$ = __global_pointer$ + 1K;
}
