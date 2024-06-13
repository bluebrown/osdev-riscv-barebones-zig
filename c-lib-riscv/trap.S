.attribute arch, "rv32g"

// trap to be used in vectored mode.
// FIXME: doesn save/restore registers
.section .text
.global   trapVector
.align    2
// instruction must be 32 bit aligned to form the
// trap vector. on interupt the cpu will jump to
// addr of exception code * 4(byte)
trapVector:
  j trap0 // 0
  j trap0 // 1
  j trap0 // 2
  j trap0 // 3
  j trap0 // 4
  j trap0 // 5
  j trap0 // 6
  j trap0 // 7
  j trap0 // 8
  j trap0 // 9
  j trap0 // 10
  j trapExternal // 11
  j trap0 // 12
  j trap0 // 13
  j trap0 // 14
  j trap0 // 15

// trap to be used in direct mode. This is
// currently in use. It saves and restored
// the registers properly. calls trap0,
// implemented in c. trap0 should return
.section .text
.globl trapDirect
.align 4
trapDirect:
  // make room for 30 registers on the stack
  // using addi. 32-2 (x0 and tp (x4))
  addi sp, sp, -120

  // save the registers before the trap handler
  // can be called. They will be restored later
  sw  x1,   0(sp)
  sw  x2,   4(sp)
  sw  x3,   8(sp)
  sw  x5,  12(sp)
  sw  x6,  16(sp)
  sw  x7,  20(sp)
  sw  x8,  24(sp)
  sw  x9,  28(sp)
  sw x10,  32(sp)
  sw x11,  36(sp)
  sw x12,  40(sp)
  sw x13,  44(sp)
  sw x14,  48(sp)
  sw x15,  52(sp)
  sw x16,  56(sp)
  sw x17,  60(sp)
  sw x18,  64(sp)
  sw x19,  68(sp)
  sw x20,  72(sp)
  sw x21,  76(sp)
  sw x22,  80(sp)
  sw x23,  84(sp)
  sw x24,  88(sp)
  sw x25,  92(sp)
  sw x26,  96(sp)
  sw x27, 100(sp)
  sw x28, 104(sp)
  sw x29, 108(sp)
  sw x30, 112(sp)
  sw x31, 116(sp)

  // call the trap handler
  call trap0

  // retore registers
  lw  x1,   0(sp)
  lw  x2,   4(sp)
  lw  x3,   8(sp)
  lw  x5,  12(sp)
  lw  x6,  16(sp)
  lw  x7,  20(sp)
  lw  x8,  24(sp)
  lw  x9,  28(sp)
  lw x10,  32(sp)
  lw x11,  36(sp)
  lw x12,  40(sp)
  lw x13,  44(sp)
  lw x14,  48(sp)
  lw x15,  52(sp)
  lw x16,  56(sp)
  lw x17,  60(sp)
  lw x18,  64(sp)
  lw x19,  68(sp)
  lw x20,  72(sp)
  lw x21,  76(sp)
  lw x22,  80(sp)
  lw x23,  84(sp)
  lw x24,  88(sp)
  lw x25,  92(sp)
  lw x26,  96(sp)
  lw x27, 100(sp)
  lw x28, 104(sp)
  lw x29, 108(sp)
  lw x30, 112(sp)
  lw x31, 116(sp)

  // restore the stack pointer
  addi sp, sp, 120

  // return from trap
  mret
