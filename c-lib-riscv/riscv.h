#ifndef RISCV_H
#define RISCV_H

#include <stddef.h>
#include <stdint.h>

// common assembly instructions

#define wfi asm volatile("wfi")

#define mret                                                                   \
  asm volatile("mret");                                                        \
  __builtin_unreachable()

// csr instructions

#define csrr(rd, csr) asm volatile("csrr %0, " #csr : "=r"(rd))
#define csrw(csr, rs1) asm volatile("csrw " #csr ", %0" ::"r"(rs1))
#define csrs(csr, rs1) asm volatile("csrs " #csr ", %0" ::"r"(rs1))
#define csrc(csr, rs1) asm volatile("csrc " #csr ", %0" ::"r"(rs1))

// control and status registers

// mstatus register

// 31  30 23  22 21  20  19  18   17
// SD   WPRI TSR TW TVM MXR SUM MPRV
//  1      8   1  1   1   1   1    1
//
// 16   15 14   13 12    11 10 9   8    7    6    5    4   3    2   1   0
// XS[1:0] FS[1:0] MPP[1:0] WPRI SPP MPIE WPRI SPIE UPIE MIE WPRI SIE UIE
//       2       2        2    2   1    1    1    1    1   1    1   1   1
//
// Figure 3.6: Machine-mode status register (mstatus) for RV32.

#define MSTATUS_SIE (1 << 1)
#define MSTATUS_MIE (1 << 3)
#define MSTATUS_SPIE (1 << 5)
#define MSTATUS_MPIE (1 << 7)
#define MSTATUS_SPP (1 << 8)
#define MSTATUS_MPP_M (3 << 11)
#define MSTATUS_MPP_S (1 << 11)

// mie register

#define MIE_MSIE (1 << 3)
#define MIE_MTIE (1 << 7)
#define MIE_MEIE (1 << 11)

// sie register

#define SIE_SSIE (1 << 1)
#define SIE_STIE (1 << 5)
#define SIE_SEIE (1 << 9)

// xcause register

// Table 3.6: Machine cause register (mcause) values after trap.
struct __attribute__((packed)) XCause {
  uint32_t code : 31;
  uint32_t is_interrupt : 1;
};

static inline struct XCause MCause() {
  struct XCause cause;
  asm volatile("csrr %0, mcause" : "=r"(cause));
  return cause;
}

static inline struct XCause SCause() {
  struct XCause cause;
  asm volatile("csrr %0, scause" : "=r"(cause));
  return cause;
}

#define IRQ_USER_SOFTWARE_INTERRUPT (0)
#define IRQ_SUPERVISOR_SOFTWARE_INTERRUPT (1)
#define IRQ_MACHINE_SOFTWARE_INTERRUPT (3)
#define IRQ_USER_TIMER_INTERRUPT (4)
#define IRQ_SUPERVISOR_TIMER_INTERRUPT (5)
#define IRQ_MACHINE_TIMER_INTERRUPT (7)
#define IRQ_USER_EXTERNAL_INTERRUPT (8)
#define IRQ_SUPERVISOR_EXTERNAL_INTERRUPT (9)
#define IRQ_MACHINE_EXTERNAL_INTERRUPT (11)

static const char *irq_names[] = {
    [IRQ_USER_SOFTWARE_INTERRUPT] = "User software interrupt",
    [IRQ_SUPERVISOR_SOFTWARE_INTERRUPT] = "Supervisor software interrupt",
    [IRQ_MACHINE_SOFTWARE_INTERRUPT] = "Machine software interrupt",
    [IRQ_USER_TIMER_INTERRUPT] = "User timer interrupt",
    [IRQ_SUPERVISOR_TIMER_INTERRUPT] = "Supervisor timer interrupt",
    [IRQ_MACHINE_TIMER_INTERRUPT] = "Machine timer interrupt",
    [IRQ_USER_EXTERNAL_INTERRUPT] = "User external interrupt",
    [IRQ_SUPERVISOR_EXTERNAL_INTERRUPT] = "Supervisor external interrupt",
    [IRQ_MACHINE_EXTERNAL_INTERRUPT] = "Machine external interrupt",
};

#define EXC_INSTRUCTION_ACCESS_FAULT (1)
#define EXC_ILLEGAL_INSTRUCTION (2)
#define EXC_BREAKPOINT (3)
#define EXC_LOAD_ACCESS_FAULT (5)
#define EXC_STORE_AMO_ACCESS_FAULT (7)
#define EXC_INSTRUCTION_PAGE_FAULT (12)
#define EXC_LOAD_PAGE_FAULT (13)
#define EXC_STORE_AMO_PAGE_FAULT (15)

static const char *exception_names[] = {
    [EXC_INSTRUCTION_ACCESS_FAULT] = "Instruction access fault",
    [EXC_ILLEGAL_INSTRUCTION] = "Illegal instruction",
    [EXC_BREAKPOINT] = "Breakpoint",
    [EXC_LOAD_ACCESS_FAULT] = "Load access fault",
    [EXC_STORE_AMO_ACCESS_FAULT] = "Store/AMO access fault",
    [EXC_INSTRUCTION_PAGE_FAULT] = "Instruction page fault",
    [EXC_LOAD_PAGE_FAULT] = "Load page fault",
    [EXC_STORE_AMO_PAGE_FAULT] = "Store/AMO page fault",
};

// MMIO

// plic

// these are defined in the ISA specification
#define PLIC_NUMSOURCE (1024)
#define PLIC_ALIGNMENT  (32)
#define PLIC_WORDSIZE (PLIC_ALIGNMENT / 8)

// a WARL registers uses a full word for each value so the max number of
// possible sources must be multiplied by the word length
#define PLIC_WARL_STRIDE (PLIC_NUMSOURCE * PLIC_WORDSIZE) // 4KB(0x1000)

// as pending/interupt bits are indvidual bits, the max number of sources must
// be divided by the alignemnt to get the number of words needed. As each word
// is n bytes the result must be multiplied by the word size to get the number
// of bytes needed
#define PLIC_BITS_STRIDE (PLIC_NUMSOURCE / PLIC_ALIGNMENT * PLIC_WORDSIZE) // 128B(0x80)

static inline size_t plicContext(size_t hart, size_t mode) {
  return (hart << 1) | mode;
}

// plic array access the plic as if it were a word size aligned array.
// Therefore, the provided source is multiplied by the word size to get the
// the offset from the base address
static inline size_t plicArray(size_t base, size_t offset, size_t src) {
  return base + offset + src * PLIC_WORDSIZE;
}

// the warl access only gives the block start based on the context. This is
// because PLIC only uses the block start for each context to store one of
// treshold, claim or complete registers
static inline size_t plicWarl(size_t base, size_t offset, size_t context) {
  return base + offset + context * PLIC_WARL_STRIDE;
}

// deviding the source by the alignment gives the word offset, the remainder
// gives the bit offset in the word and the base is added to get the address
// of the bit:
//
//      id = 31 -> word = 31 / 32 = 0
//      id = 32 -> word = 32 / 32 = 1
//
// Likewise, the bit, for a given source, within that word, must be determined
// by the remainder of the division:
//
//      id = 31 -> bit = 31 % 32 = 31
//      id = 32 -> bit = 32 % 32 = 0
//
static inline size_t plicBits(size_t base, size_t offset, size_t context, size_t src) {
  return base + offset + context * PLIC_BITS_STRIDE + src / PLIC_ALIGNMENT;
}

#ifdef BOARD_QEMU_RISCV_VIRT
#define PLIC_BASE (0x0c000000)
#endif

// stride here is fixed as showed on the ISA spec. above calculation produces
// the same result but is less clear to the reader. Therefore plain values are
// kept here.

// 1024 * 4 = 4096(0x1000) bytes
#define PLIC_PRIORITY_OFFSET 0x0000

// 1024 / 8 = 128(0x80) bytes
#define PLIC_PENDING_OFFSET 0x1000
#define PLIC_PENDING_STRIDE 0x80

// 1024 / 8 = 128(0x80) bytes
#define PLIC_ENABLE_OFFSET 0x2000
#define PLIC_ENABLE_STRIDE 0x80

// 4096 * 15872 = 65011712(0x3e00000) bytes
#define PLIC_THRESHOLD_OFFSET 0x200000
#define PLIC_THRESHOLD_STRIDE 0x1000

// 4096 * 15872 = 65011712(0x3e00000) bytes
#define PLIC_CLAIM_OFFSET 0x200004
#define PLIC_CLAIM_STRIDE 0x1000

// 4096 * 15872 = 65011712(0x3e00000) bytes
#define PLIC_COMPLETE_OFFSET 0x200004
#define PLIC_COMPLETE_STRIDE 0x1000

// uart

#ifdef BOARD_QEMU_RISCV_VIRT
#define UART_BASE (0x10000000)
#define PLIC_SRC_UART (10)
#endif
// 8 bit registers
#define UART_RBR (0x0)
#define UART_THR (0x0)
#define UART_DLL (0x0)
#define UART_IER (0x1)
#define UART_DLH (0x1)
#define UART_IIR (0x2)
#define UART_FCR (0x2)
#define UART_LCR (0x3)
#define UART_MCR (0x4)
#define UART_LSR (0x5)
#define UART_MSR (0x6)
#define UART_SR (0x7)

#endif
