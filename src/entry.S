.equ STACK_SIZE, 1024 * 16

.section .text
.global  _start

_start:
	csrr a0, mhartid
	addi a0, a0, 1
	li   a1, STACK_SIZE
	mul  a0, a0, a1
	la   sp, __stack_top$
	add  sp, sp, a0
	tail main

halt:
	wfi
	j halt
