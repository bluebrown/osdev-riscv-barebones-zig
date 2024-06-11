#define BOARD_QEMU_RISCV_VIRT

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

#define EXC_INSTRUCTION_ACCESS_FAULT 1
#define EXC_ILLEGAL_INSTRUCTION 2
#define EXC_LOAD_ACCESS_FAULT 5
#define EXC_STORE_AMO_ACCESS_FAULT 7
#define EXC_INSTRUCTION_PAGE_FAULT 12
#define EXC_LOAD_PAGE_FAULT 13
#define EXC_STORE_AMO_PAGE_FAULT 15

const char *exception_names[] = {
    [EXC_INSTRUCTION_ACCESS_FAULT] = "Instruction access fault",
    [EXC_ILLEGAL_INSTRUCTION] = "Illegal instruction",
    [EXC_LOAD_ACCESS_FAULT] = "Load access fault",
    [EXC_STORE_AMO_ACCESS_FAULT] = "Store/AMO access fault",
    [EXC_INSTRUCTION_PAGE_FAULT] = "Instruction page fault",
    [EXC_LOAD_PAGE_FAULT] = "Load page fault",
    [EXC_STORE_AMO_PAGE_FAULT] = "Store/AMO page fault",
};

#define IRQ_MACHINE_SOFTWARE_INTERRUPT 3
#define IRQ_MACHINE_TIMER_INTERRUPT 7
#define IRQ_MACHINE_EXTERNAL_INTERRUPT 11
#define IRQ_COUNTER_OVERFLOW_INTERRUPT 13

const char *irq_names[] = {
    [IRQ_MACHINE_SOFTWARE_INTERRUPT] = "Machine software interrupt",
    [IRQ_MACHINE_TIMER_INTERRUPT] = "Machine timer interrupt",
    [IRQ_MACHINE_EXTERNAL_INTERRUPT] = "Machine external interrupt",
    [IRQ_COUNTER_OVERFLOW_INTERRUPT] = "Counter overflow interrupt",
};

struct __attribute__((packed)) XCause {
  uint32_t code : 31;
  uint32_t is_interrupt : 1;
};

struct XCause MCause() {
  uint32_t cause;
  asm volatile("csrr %0, mcause" : "=r"(cause));
  return *(struct XCause *)&cause;
}

// plic

#ifdef BOARD_QEMU_RISCV_VIRT
#define PLIC_BASE (0x0c000000)
#endif
// 32 bit registers (word sized)
// context are 4k aligned
#define PLIC_PRIORITY (0x0000 / 0x4)
#define PLIC_ENABLE (0x2000 / 0x4)
#define PLIC_THRESHOLD (0x200000 / 0x4)
#define PLIC_CLAIM (0x200004 / 0x4)
#define PLIC_COMPLETE (0x200004 / 0x4)

struct PlicDriver {
  uint32_t *volatile base;
};

struct PlicDriver PlicDriver(size_t base) {
  return (struct PlicDriver){
      .base = (uint32_t *)base,
  };
}

static inline size_t plic_wordIndex(size_t mode) {
  int hart;
  asm volatile("csrr %0, mhartid" : "=r"(hart));
  // assume 4k page size, 4 byte word size, and
  // reduce the calculation. The full formula would be
  // (hart << 1 | mode) * PAGE_SIZE / WORD_SIZE
  return (hart << 1 | mode) << 10;
}

void plic_priority(struct PlicDriver *p, uint32_t src, size_t prio) {
  p->base[PLIC_PRIORITY + src] = prio;
}

void plic_enable(struct PlicDriver *p, size_t idx, size_t src) {
  // assume 32 bit word size
  p->base[PLIC_ENABLE + ((idx + src) >> 5)] |= 1 << (src & 31);
}

void plic_threshold(struct PlicDriver *p, size_t idx, size_t th) {
  p->base[PLIC_THRESHOLD + idx] = th;
}

size_t plic_claim(struct PlicDriver *p, size_t idx) {
  return p->base[PLIC_CLAIM + idx];
}

void plic_complete(struct PlicDriver *p, size_t idx, size_t src) {
  p->base[PLIC_COMPLETE + idx] = src;
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

struct UartDriver {
  uint8_t *ports;
};

struct UartDriver UartDriver(size_t base) {
  return (struct UartDriver){.ports = (uint8_t *)base};
}

void uart_rtxWrite(struct UartDriver *uart, char c) {
  while ((uart->ports[UART_LSR] & 0x20) == 0)
    ;
  if (c == '\n')
    uart->ports[UART_THR] = '\r';
  uart->ports[UART_THR] = c;
}

char uart_rtxRead(struct UartDriver *uart) {
  while ((uart->ports[UART_LSR] & 0x1) == 0)
    ;
  char c = uart->ports[UART_RBR];
  if (c == '\r')
    c = '\n';
  return c;
}

void uart_rtxFlush(struct UartDriver *uart) {
  while ((uart->ports[UART_LSR] & 0x40) == 0)
    ;
}

void uart_fifoInit(struct UartDriver *uart) {
  uart->ports[UART_FCR] |= (1 | 3 << 1);
}

uint8_t uart_fifoStatus(struct UartDriver *uart) {
  return uart->ports[UART_IER] >> 6 & 3;
}

void uart_irqEnableSet(struct UartDriver *uart, uint8_t flags) {
  uart->ports[UART_IER] |= (flags & 0xf);
}

void uart_irqEnableClear(struct UartDriver *uart, uint8_t flags) {
  uart->ports[UART_IER] &= ~(flags & 0xf);
}

uint8_t uart_irqIsPending(struct UartDriver *uart) {
  return (uart->ports[UART_IIR] & 1) == 0;
}

// print functions

typedef void Write(void *, char);

struct Writer {
  void *impl;
  Write *write;
};

void fprint(struct Writer *w, const char *s) {
  while (*s)
    w->write(w->impl, *s++);
}

// string functions

const char *hexchars = "0123456789abcdef";

char *itoa(size_t base, size_t num, char *buf) {
  char *p = buf + 35;

  *--p = '\0';

  do {
    *--p = hexchars[num % base];
    num /= base;
  } while (num);

  if (base == 2)
    *--p = 'b';
  if (base == 8)
    *--p = 'o';
  if (base == 10)
    *--p = 'd';
  if (base == 16)
    *--p = 'x';

  *--p = '0';

  return p;
}

// ====================================
// main program
// ====================================

#define new

#define hotloop                                                                \
  while (1) {                                                                  \
  }

void irqHandler();

int main() {
  // load the trap as first thing, to catch any exception.
  // the setup uart in order to print debug messages.
  csrw(mtvec, &irqHandler);

  struct UartDriver u = new UartDriver(UART_BASE);
  struct Writer w =
      (struct Writer){.impl = &u, .write = (Write *)uart_rtxWrite};

  uart_fifoInit(&u);

  // start the real initialization
  fprint(&w, "init\n");

  // enable interrupts
  csrs(mstatus, MSTATUS_MPP_M | MSTATUS_MPIE | MSTATUS_MIE);
  csrs(mie, MIE_MSIE | MIE_MEIE);
  fprint(&w, "interrupts enabled\n");

  // enable uart interrupts on PLIC
  struct PlicDriver p = new PlicDriver(PLIC_BASE);

  int ctx = plic_wordIndex(0);
  fprint(&w, "PLIC context: ");
  fprint(&w, itoa(16, ctx, (char[35]){0}));
  fprint(&w, "\n");

  plic_priority(&p, PLIC_SRC_UART, 1);
  plic_enable(&p, ctx, PLIC_SRC_UART);
  plic_threshold(&p, ctx, 0);
  fprint(&w, "PLIC configured for UART\n");

  // enable uart interrupts
  uart_irqEnableSet(&u, 1);
  fprint(&w, "UART interrupts enabled\n");

  fprint(&w, "waiting for interrupts\n");
  while (1)
    uart_rtxWrite(&u, uart_rtxRead(&u));
}

void irqHandler() {
  struct UartDriver uart = new UartDriver(UART_BASE);
  struct Writer w =
      (struct Writer){.impl = &uart, .write = (Write *)uart_rtxWrite};
  char buf[35];

  fprint(&w, "irq: ");

  struct XCause cause = new MCause();
  fprint(&w, itoa(2, cause.is_interrupt, buf));
  fprint(&w, ": code: ");
  fprint(&w, itoa(16, cause.code, buf));
  fprint(&w, ": ");

  if (!cause.is_interrupt) {
    fprint(&w, "exception: ");
    fprint(&w, exception_names[cause.code]);
    hotloop;
  }

  fprint(&w, "interrupt: ");
  fprint(&w, irq_names[cause.code]);
  fprint(&w, ": ");

  if (cause.code == 11) {
    struct PlicDriver p = new PlicDriver(PLIC_BASE);
    int idx = plic_wordIndex(0);
    size_t src = plic_claim(&p, idx);
    fprint(&w, "PLIC source: ");
    fprint(&w, itoa(16, src, buf));
    plic_complete(&p, idx, src);
    fprint(&w, "\n");
    hotloop;
  }

  fprint(&w, "\n");
  hotloop;
}
