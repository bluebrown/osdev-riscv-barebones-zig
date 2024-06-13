#include <stddef.h>
#include <stdint.h>

// common assembly instructions

#define wfi asm volatile("wfi")

#define mret                                                                   \
  asm volatile("mret");                                                        \
  __builtin_unreachable()

// csr instructions

#define csrr(csr, rd) asm volatile("csrr %0, " #csr : "=r"(rd))
#define csrw(csr, rs1) asm volatile("csrw " #csr ", %0" ::"r"(rs1))
#define csrs(csr, rs1) asm volatile("csrs " #csr ", %0" ::"r"(rs1))
#define csrc(csr, rs1) asm volatile("csrc " #csr ", %0" ::"r"(rs1))

// control and status registers

// mstatus register

#define MSTATUS_MIE (1 << 3)
#define MSTATUS_MPIE (1 << 7)
#define MSTATUS_MPP (3 << 11)
#define MSTATUS_MPP_M (3 << 11)
#define MSTATUS_MPP_S (1 << 11)

// mie register

#define MIE_MSIE (1 << 3)
#define MIE_MTIE (1 << 7)
#define MIE_MEIE (1 << 11)

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

struct __attribute__((packed)) XCause {
  uint32_t code : 31;
  uint32_t is_interrupt : 1;
};

static inline struct XCause MCause() {
  struct XCause cause;
  asm volatile("csrr %0, mcause" : "=r"(cause));
  return cause;
}
