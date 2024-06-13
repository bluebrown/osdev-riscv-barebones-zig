.attribute arch, "rv32g"

.section .text

.global _start
.type   _start, @function

_start:
	.cfi_startproc

	.option push
	.option norelax
	la      gp, __global_pointer$
	.option pop

	csrr a3, mhartid
	mv   tp, a3
	addi a0, a3, 1
	li   a1, 0x4000
	mul  a0, a0, a1
	la   sp, __stack_top$
	add  sp, sp, a0
  beqz a3, _deadlock

	tail main

	.cfi_endproc

_deadlock:
	wfi
	tail _deadlock
