SECTIONS {
    . = 0x80000000;

    .text ALIGN(8) : {
      *(.text .text*)
    }

    .rodata ALIGN(8) : {
      *(.rodata .rodata*)
    }

    .data ALIGN(8) : {
      *(.data .data*)
    }

    .sdata ALIGN(8) : {
      *(.srodata .srodata*)
      *(.sdata .sdata*)
    }

    .sbss ALIGN(8) : {
      *(.sbss .sbss*)
    }

    .bss ALIGN(8) : {
      *(.bss .bss.*)
      . = ALIGN(8);
      PROVIDE(__stack_top$ = .);
      . += 64K;
    }

}

