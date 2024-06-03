// set some attributes and options to help any cc, to understand how this is
// supposed to be compiled. it would be nice if we didnt have to do it, but
// from my expxierence when using langauges such as rust or zig, they dont
// always get it right.
.attribute arch, "rv64g" // riscv64 general
.option    arch, +c      // enable c extensions, just for demo

.equ STACK_SIZE, 1096 * 16
.equ MAX_HARTS, 8

.section .text
.global  _start
.type    _start, @function

_start:
	// set the global pointer and make sure
	// "gp relaxation" is disabled for this.
	// is required. It will avoid relative
	// addressing problems.
	.option push
	.option norelax
	la      gp, __global_pointer$
	.option pop

	// put hartid in a3 and keep it,
	// for the next intructions.
	csrr a3, mhartid

	// set the thread pointer. This helps
	// to identify the current thread, later.
	mv tp, a3

	// set the stack pointer with an offset
	// of (hartid + 1) * STACK_SIZE.
	addi a0, a3, 1
	li   a1, STACK_SIZE * MAX_HARTS
	mul  a0, a0, a1
	la   sp, __stack_top$
	add  sp, sp, a0

	// call the main function.
	// main will run in machine mode and
	// will be responsible for boostrapping
	// the hart in supervisor mode.
	tail main

	// deadlock, if main returns
	// for some reason.
deadlock:
	wfi
	j deadlock
