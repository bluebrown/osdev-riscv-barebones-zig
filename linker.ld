/* [NOTE]
 * the general layout of the section is based on
 * the default script of the riscv64-unknown-elf-ld.
 *
 * .Get the defaul script
 * [source,bash]
 * riscv64-unknown-elf-ld --verbose
*/
OUTPUT_FORMAT(elf64-littleriscv)
OUTPUT_ARCH(riscv:rv64g)
SECTIONS {
    /* qemu virt machine dram base */
    . = 0x80000000;

    .text ALIGN(8) : {
        *(.text .text*);
    }

    .rodata ALIGN(8) :  {
        *(.rodata .rodata*);
    }

    .data ALIGN(8) : {
        __DATA_BEGIN__ = .;
        *(.data .data*);
    }

    /* pack small data for more effient access */
    .sdata ALIGN(8) : {
        __SDATA_BEGIN__ = .;
        *(.srodata .srodata*);
        *(.sdata .sdata*);
    }

    .sbss ALIGN(8) : {
        *(.sbss .sbss*);
    }

    .bss ALIGN(8) : {
        *(.bss .bss.*)
    }
    __BSS_END__ = .;

    /* global uninitialized variables are in bss.
     * the global pointer can help to access them,
     * more efficiently */
    __global_pointer$ = MIN(__SDATA_BEGIN__ + 0x800,
                        MAX(__DATA_BEGIN__ + 0x800,
                            __BSS_END__ - 0x800));

      /* both the global adn the stack pointer must be
       * loaded into gp and sp respectively */
    . = ALIGN(128);
    . += 8 * 16K;
    __stack_top$ = .;
    . = ALIGN(128);
}
