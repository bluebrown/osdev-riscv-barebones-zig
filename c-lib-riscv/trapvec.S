// instruction must be 32 bit aligned to form the
// trap vector. on interupt the cpu will jump to
// addr of exception code * 4(byte)
.section .text
.global trapvec
trapvec:
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
