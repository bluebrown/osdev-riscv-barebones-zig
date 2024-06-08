set confirm off
set architecture riscv:rv32
target remote 127.0.0.1:1234
symbol-file greeting.elf
set disassemble-next-line auto
set riscv use-compressed-breakpoints yes
set breakpoint pending on
break _start
break main
tui enable
layout reg
cont
