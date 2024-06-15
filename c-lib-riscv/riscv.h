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
// 1       8   1  1   1   1   1    1
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

#define EXC_INSTRUCTION_ACCESS_FAULT (1)
#define EXC_ILLEGAL_INSTRUCTION (2)
#define EXC_LOAD_ACCESS_FAULT (5)
#define EXC_STORE_AMO_ACCESS_FAULT (7)
#define EXC_INSTRUCTION_PAGE_FAULT (12)
#define EXC_LOAD_PAGE_FAULT (13)
#define EXC_STORE_AMO_PAGE_FAULT (15)

static const char *exception_names[] = {
    [EXC_INSTRUCTION_ACCESS_FAULT] = "Instruction access fault",
    [EXC_ILLEGAL_INSTRUCTION] = "Illegal instruction",
    [EXC_LOAD_ACCESS_FAULT] = "Load access fault",
    [EXC_STORE_AMO_ACCESS_FAULT] = "Store/AMO access fault",
    [EXC_INSTRUCTION_PAGE_FAULT] = "Instruction page fault",
    [EXC_LOAD_PAGE_FAULT] = "Load page fault",
    [EXC_STORE_AMO_PAGE_FAULT] = "Store/AMO page fault",
};

#define IRQ_MACHINE_SOFTWARE_INTERRUPT (3)
#define IRQ_MACHINE_TIMER_INTERRUPT (7)
#define IRQ_MACHINE_EXTERNAL_INTERRUPT (11)
#define IRQ_COUNTER_OVERFLOW_INTERRUPT (13)

static const char *irq_names[] = {
    [IRQ_MACHINE_SOFTWARE_INTERRUPT] = "Machine software interrupt",
    [IRQ_MACHINE_TIMER_INTERRUPT] = "Machine timer interrupt",
    [IRQ_MACHINE_EXTERNAL_INTERRUPT] = "Machine external interrupt",
    [IRQ_COUNTER_OVERFLOW_INTERRUPT] = "Counter overflow interrupt",
};

struct __attribute__((packed)) MCause {
  uint32_t code : 31;
  uint32_t is_interrupt : 1;
};

static inline struct MCause MCause() {
  struct MCause cause;
  asm volatile("csrr %0, mcause" : "=r"(cause));
  return cause;
}

// MMIO

// plic

#ifdef BOARD_QEMU_RISCV_VIRT
#define PLIC_BASE (0x0c000000)
#endif
#define PLIC_PRIORITY_OFFSET 0x0000
#define PLIC_PENDING_OFFSET 0x1000
#define PLIC_PENDING_STRIDE 0x80
#define PLIC_ENABLE_OFFSET 0x2000
#define PLIC_ENABLE_STRIDE 0x80
#define PLIC_THRESHOLD_OFFSET 0x200000
#define PLIC_THRESHOLD_STRIDE 0x1000
#define PLIC_CLAIM_OFFSET 0x200004
#define PLIC_CLAIM_STRIDE 0x1000
#define PLIC_COMPLETE_OFFSET 0x200004
#define PLIC_COMPLETE_STRIDE 0x1000


// dynamic macros are smelling bad lets do static inline functions

// #define PLIC_CONTEXT(hart, mode) ((hart << 1) | mode)
static inline size_t plicContext(size_t hart, size_t mode) {
  return (hart << 1) | mode;
}

// #define PLIC_ACCESS_ARRAY(offset, src) (PLIC_BASE + PLIC_##offset##_OFFSET + src * 4)
static inline size_t plicArray(size_t base, size_t offset, size_t src) {
  return base + offset + src * 4;
}

// #define PLIC_ACCESS_WARL(offset, context)                                     \
//   (PLIC_BASE + PLIC_##offset##_OFFSET + context * PLIC_##offset##_STRIDE)
static inline size_t plicWarl(size_t base, size_t offset, size_t context) {
  return base + offset + context * 0x1000;
}

// #define PLIC_ACCESS_BITS(offset, context, src)                                 \
//   (PLIC_BASE + PLIC_##offset##_OFFSET + context * PLIC_##offset##_STRIDE + src / 32)
static inline size_t plicBits(size_t base, size_t offset, size_t context, size_t src) {
  return base + offset + context * 0x80 + src / 32;
}

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


// scause
// XLEN-1 XLEN-2 0
// Interrupt Exception Code (WLRL)
// 1 XLEN-1
// Figure 4.9: Supervisor Cause register scause.
// Interrupt Exception Code Description
// 1 0 User software interrupt
// 1 1 Supervisor software interrupt
// 1 2–3 Reserved
// 1 4 User timer interrupt
// 1 5 Supervisor timer interrupt
// 1 6–7 Reserved
// 1 8 User external interrupt
// 1 9 Supervisor external interrupt
// 1 ≥10 Reserved
// 0 0 Instruction address misaligned
// 0 1 Instruction access fault
// 0 2 Illegal instruction
// 0 3 Breakpoint
// 0 4 Reserved
// 0 5 Load access fault
// 0 6 AMO address misaligned
// 0 7 Store/AMO access fault
// 0 8 Environment call
// 0 9–11 Reserved
// 0 12 Instruction page fault
// 0 13 Load page fault
// 0 14 Reserved
// 0 15 Store/AMO page fault
// 0 ≥16 Reserved
struct __attribute__((packed)) SCause {
  uint32_t code : 31;
  uint32_t is_interrupt : 1;
};

static inline struct SCause SCause() {
  struct SCause cause;
  asm volatile("csrr %0, scause" : "=r"(cause));
  return cause;
}

#define SIRQ_USER_SOFTWARE_INTERRUPT (0)
#define SIRQ_SUPERVISOR_SOFTWARE_INTERRUPT (1)
#define SIRQ_USER_TIMER_INTERRUPT (4)
#define SIRQ_SUPERVISOR_TIMER_INTERRUPT (5)
#define SIRQ_USER_EXTERNAL_INTERRUPT (8)
#define SIRQ_SUPERVISOR_EXTERNAL_INTERRUPT (9)

static const char *sirq_names[] = {
    [SIRQ_USER_SOFTWARE_INTERRUPT] = "User software interrupt",
    [SIRQ_SUPERVISOR_SOFTWARE_INTERRUPT] = "Supervisor software interrupt",
    [SIRQ_USER_TIMER_INTERRUPT] = "User timer interrupt",
    [SIRQ_SUPERVISOR_TIMER_INTERRUPT] = "Supervisor timer interrupt",
    [SIRQ_USER_EXTERNAL_INTERRUPT] = "User external interrupt",
    [SIRQ_SUPERVISOR_EXTERNAL_INTERRUPT] = "Supervisor external interrupt",
};

#define SEXC_INSTRUCTION_ACCESS_FAULT (1)
#define SEXC_ILLEGAL_INSTRUCTION (2)
#define SEXC_BREAKPOINT (3)
#define SEXC_LOAD_ACCESS (5)
#define SEXC_AMO_ADDRESS_MISALIGNED (6)

static const char *sexception_names[] = {
    [SEXC_INSTRUCTION_ACCESS_FAULT] = "Instruction access fault",
    [SEXC_ILLEGAL_INSTRUCTION] = "Illegal instruction",
    [SEXC_BREAKPOINT] = "Breakpoint",
    [SEXC_LOAD_ACCESS] = "Load access fault",
    [SEXC_AMO_ADDRESS_MISALIGNED] = "AMO address misaligned",
};

#endif
