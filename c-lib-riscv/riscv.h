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

// the reigsters are layed out such that higher privleges are supersets of lower
// privleges, in terms of layout. For a given register, fields for a given
// privilege level are WIRI (write ignore, read ignore), when accessed from a
// lower privlege level. Thus only the highest privilige level is defined here
// as all other prvilige level can use the same data structured and constants.
// These definition shall be called x<register> where x means it can be used for
// any privilige level. The definitions are not comprehensive and only include
// the fields that are used in this project.

// xstatus register

// 31  30 23  22 21  20  19  18   17
// SD   WPRI TSR TW TVM MXR SUM MPRV
//  1      8   1  1   1   1   1    1
//
// 16   15 14   13 12    11 10 9   8    7    6    5    4   3    2   1   0
// XS[1:0] FS[1:0] MPP[1:0] WPRI SPP MPIE WPRI SPIE UPIE MIE WPRI SIE UIE
//       2       2        2    2   1    1    1    1    1   1    1   1   1
//
// Figure 3.6: Machine-mode status register (mstatus) for RV32.

#define XSTATUS_SIE (1 << 1)
#define XSTATUS_MIE (1 << 3)
#define XSTATUS_SPIE (1 << 5)
#define XSTATUS_MPIE (1 << 7)
#define XSTATUS_SPP (1 << 8)
#define XSTATUS_MPP_M (3 << 11)
#define XSTATUS_MPP_S (1 << 11)

// xie register

// XLEN-1 12   11   10    9    8    7    6    5    4    3    2    1    0
//      WPRI MEIE WPRI SEIE UEIE MTIE WPRI STIE UTIE MSIE WPRI SSIE USIE
//   XLEN-12    1    1    1    1    1    1    1    1    1    1    1    1
//
// Figure 3.12: Machine interrupt-enable register (mie).

#define XIE_SSIE (1 << 1)
#define XIE_MSIE (1 << 3)
#define XIE_STIE (1 << 5)
#define XIE_MTIE (1 << 7)
#define XIE_SEIE (1 << 9)
#define XIE_MEIE (1 << 11)

// xcause register

// Interrupt Exception   Code Description
// ----------------------------------------------------
//         1         0   User software interrupt
//         1         1   Supervisor software interrupt
//         1         2   Reserved
//         1         3   Machine software interrupt
//         1         4   User timer interrupt
//         1         5   Supervisor timer interrupt
//         1         6   Reserved
//         1         7   Machine timer interrupt
//         1         8   User external interrupt
//         1         9   Supervisor external interrupt
//         1        10   Reserved
//         1        11   Machine external interrupt
//         1       ≥12   Reserved
//         0         0   Instruction address misaligned
//         0         1   Instruction access fault
//         0         2   Illegal instruction
//         0         3   Breakpoint
//         0         4   Load address misaligned
//         0         5   Load access fault
//         0         6   Store/AMO address misaligned
//         0         7   Store/AMO access fault
//         0         8   Environment call from U-mode
//         0         9   Environment call from S-mode
//         0        10   Reserved
//         0        11   Environment call from M-mode
//         0        12   Instruction page fault
//         0        13   Load page fault
//         0        14   Reserved
//         0        15   Store/AMO page fault
//         0       ≥16   Reserved
//
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

#endif
