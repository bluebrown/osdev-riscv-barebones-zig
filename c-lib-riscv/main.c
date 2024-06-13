#include "config.h"
#include "cpu.h"
#include "fmt.h"
#include "plic.h"
#include "uart.h"

#define hotloop                                                                \
  while (1) {                                                                  \
  }

// trap vec is a 4 byte aligned
// jump switch in trapvec.S
extern void trapvec();

int main() {
  // load the trap first, so any exception during init
  // will be reported
  csrw(mtvec, (uint32_t)trapvec | 1);

  // mmio driver instances
  struct UartDriver u = UartDriver(UART_BASE);
  struct Writer w = (struct Writer){&u, (Write *)uart_rtxWrite};

  // configure uart before printing
  uart_fifoInit(&u);

  // start the initialization
  fprint(&w, "init\n");

  // enable interrupts
  csrs(mstatus, MSTATUS_MPP_M | MSTATUS_MPIE | MSTATUS_MIE);
  csrs(mie, MIE_MSIE | MIE_MEIE);
  fprint(&w, "interrupts enabled\n");

  // enable uart interrupts on PLIC
  struct PlicDriver p = PlicDriver(PLIC_BASE);

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
    wfi;
}

void trap0() {
  char buf[35];
  struct UartDriver u = {(uint8_t *)UART_BASE};
  struct Writer w = (struct Writer){&u, (Write *)uart_rtxWrite};

  fprint(&w, "irq: base: ");

  struct XCause cause = MCause();
  fprint(&w, itoa(2, cause.is_interrupt, buf));
  fprint(&w, ": code: ");
  fprint(&w, itoa(16, cause.code, buf));
  fprint(&w, ": ");

  if (cause.is_interrupt)
    fprint(&w, irq_names[cause.code]);
  else
    fprint(&w, exception_names[cause.code]);

  fprint(&w, "\n");
  hotloop;
}

void trapExternal() {
  char buf[35];
  struct UartDriver u = {(uint8_t *)UART_BASE};
  struct Writer w = (struct Writer){&u, (Write *)uart_rtxWrite};

  fprint(&w, "irq: external: ");

  struct PlicDriver p = PlicDriver(PLIC_BASE);
  int idx = plic_wordIndex(0);
  size_t src = plic_claim(&p, idx);

  fprint(&w, "PLIC source: ");
  fprint(&w, itoa(16, src, buf));
  fprint(&w, "\n");

  if (src == PLIC_SRC_UART) {
    // TODO: check uart IIR
    fprint(&w, "UART received data: ");
    uart_rtxWrite(&u, uart_rtxRead(&u));
  }

  plic_complete(&p, idx, src);
  fprint(&w, "\n");

  // FIXME: the stack and registers should be broken
  // at this point. they need to be saved at the
  // beginning of the irq handler and restored before!!
  mret;
}
