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

enum MStatus {
  MSTATUS_MIE = 1 << 3,
  MSTATUS_MPIE = 1 << 7,
  MSTATUS_MPP = 3 << 11,
  MSTATUS_MPP_M = 3 << 11,
  MSTATUS_MPP_S = 1 << 11,
};

// mie register

enum MIE {
  MIE_MSIE = 1 << 3,
  MIE_MTIE = 1 << 7,
  MIE_MEIE = 1 << 11,
};

// xcause register

enum XCauseException {
  EXC_INSTRUCTION_ACCESS_FAULT = 1,
  EXC_ILLEGAL_INSTRUCTION = 2,
  EXC_LOAD_ACCESS_FAULT = 5,
  EXC_STORE_AMO_ACCESS_FAULT = 7,
  EXC_INSTRUCTION_PAGE_FAULT = 12,
  EXC_LOAD_PAGE_FAULT = 13,
  EXC_STORE_AMO_PAGE_FAULT = 15,
};

const char *exception_names[] = {
    [EXC_INSTRUCTION_ACCESS_FAULT] = "Instruction access fault",
    [EXC_ILLEGAL_INSTRUCTION] = "Illegal instruction",
    [EXC_LOAD_ACCESS_FAULT] = "Load access fault",
    [EXC_STORE_AMO_ACCESS_FAULT] = "Store/AMO access fault",
    [EXC_INSTRUCTION_PAGE_FAULT] = "Instruction page fault",
    [EXC_LOAD_PAGE_FAULT] = "Load page fault",
    [EXC_STORE_AMO_PAGE_FAULT] = "Store/AMO page fault",
};

enum XCauseInterrupt {
  IRQ_MACHINE_SOFTWARE_INTERRUPT = 3,
  IRQ_MACHINE_TIMER_INTERRUPT = 7,
  IRQ_MACHINE_EXTERNAL_INTERRUPT = 11,
  IRQ_COUNTER_OVERFLOW_INTERRUPT = 13,
};

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

struct PlicDriver {
  uint32_t *volatile base;
  size_t priority;
  size_t enable;
  size_t threshold;
  size_t claim;
  size_t complete;
};

struct PlicDriver PlicDriver(size_t base) {
  return (struct PlicDriver){
      .base = (uint32_t *)base,
      .priority = 0x0,
      .enable = 0x2000 / 0x4,
      .threshold = 0x200000 / 0x4,
      .claim = 0x200004 / 0x4,
      .complete = 0x200004 / 0x4,
  };
}

enum PlicSource {
  PLIC_SRC_UART = 0xA,
};

// One pagesize per context, aligned at wordsize:
//
//   pagesize = 0x1000 (4kb);
//   wordsize = 0x4    (4bytes, 32bits);
//   hart     = mhartid;
//   mode     = 0 (machine mode) or 1 (supervisor mode);
//   ctx      = hart << 1 | mode;
//   start    = ctx * pagesize;
//   index    = start / wordsize;
//
size_t plic_wordIndex(size_t mode) {
  int hart;
  asm volatile("csrr %0, mhartid" : "=r"(hart));
  return (hart << 1 | mode) << 10;
}

void plic_priority(struct PlicDriver *p, enum PlicSource src, size_t prio) {
  p->base[p->priority + src] = prio;
}

void plic_enable(struct PlicDriver *p, size_t idx, size_t src) {
  p->base[p->enable + ((idx + src) >> 5)] |= 1 << (src & 31);
}

void plic_threshold(struct PlicDriver *p, size_t idx, size_t th) {
  p->base[p->threshold + idx] = th;
}

size_t plic_claim(struct PlicDriver *p, size_t idx) {
  return p->base[p->claim + idx];
}

void plic_complete(struct PlicDriver *p, size_t idx, size_t src) {
  p->base[p->complete + idx] = src;
}

// uart

struct UartDriver {
  uint8_t *ports;
};

struct UartDriver UartDriver(size_t base) {
  return (struct UartDriver){.ports = (uint8_t *)base};
}

void uart_rtxWrite(struct UartDriver *uart, char c) {
  while ((uart->ports[0x5] & 0x20) == 0)
    ;
  if (c == '\n')
    uart->ports[0x0] = '\r';
  uart->ports[0x0] = c;
}

char uart_rtxRead(struct UartDriver *uart) {
  while ((uart->ports[0x5] & 0x1) == 0)
    ;
  char c = uart->ports[0x0];
  if (c == '\r')
    c = '\n';
  return c;
}

void uart_rtxFlush(struct UartDriver *uart) {
  while ((uart->ports[0x5] & 0x40) == 0)
    ;
}

void uart_fifoInit(struct UartDriver *uart) {
  uart->ports[0x2] |= (1 | 3 << 1);
}

uint8_t uart_fifoStatus(struct UartDriver *uart) {
  return uart->ports[0x2] >> 6 & 3;
}

void uart_irqEnableSet(struct UartDriver *uart, uint8_t flags) {
  uart->ports[0x1] |= (flags & 0xf);
}

void uart_irqEnableClear(struct UartDriver *uart, uint8_t flags) {
  uart->ports[0x1] &= ~(flags & 0xf);
}

uint8_t uart_irqIsPending(struct UartDriver *uart) {
  return (uart->ports[0x2] & 1) == 0;
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

  struct UartDriver u = new UartDriver(0x10000000);
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
  struct PlicDriver p = new PlicDriver(0x0c000000);

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
  struct UartDriver uart = new UartDriver(0x10000000);
  struct Writer w =
      (struct Writer){.impl = &uart, .write = (Write *)uart_rtxWrite};
  char buf[35];

  fprint(&w, "irq: ");

  struct XCause cause = new MCause();
  fprint(&w, itoa(2, cause.is_interrupt, buf));
  fprint(&w, " : code: ");
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
    struct PlicDriver p = new PlicDriver(0x0c000000);
    int ctx = plic_wordIndex(0);
    size_t src = plic_claim(&p, ctx);
    fprint(&w, "PLIC source: ");
    fprint(&w, itoa(16, src, buf));
    plic_complete(&p, ctx, src);
    fprint(&w, "\n");
    hotloop;
  }

  fprint(&w, "\n");
  hotloop;
}
