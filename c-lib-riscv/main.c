#include "config.h"
#include "cpu.h"
#include "fmt.h"
#include "plic.h"
#include "uart.h"

#define hotloop                                                                \
  while (1) {                                                                  \
  }

void irqHandler();

int main() {
  // load the trap as first thing, to catch any exception.
  // the setup uart in order to print debug messages.
  csrw(mtvec, &irqHandler);

  struct UartDriver u = UartDriver(UART_BASE);
  struct Writer w = (struct Writer){&u, (Write *)uart_rtxWrite};

  uart_fifoInit(&u);

  // start the real initialization
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

void irqHandler() {
  char buf[35];
  struct UartDriver u = {(uint8_t *)UART_BASE};
  struct Writer w = (struct Writer){&u, (Write *)uart_rtxWrite};

  fprint(&w, "irq: ");

  struct XCause cause = MCause();
  fprint(&w, itoa(2, cause.is_interrupt, buf));
  fprint(&w, ": code: ");
  fprint(&w, itoa(16, cause.code, buf));

  if (!cause.is_interrupt) {
    fprint(&w, ": exception: ");
    fprint(&w, exception_names[cause.code]);
    hotloop;
  }

  fprint(&w, ": interrupt: ");
  fprint(&w, irq_names[cause.code]);

  if (cause.code != IRQ_MACHINE_EXTERNAL_INTERRUPT) {
    fprint(&w, "\n");
    hotloop;
  }

  struct PlicDriver p = PlicDriver(PLIC_BASE);

  int idx = plic_wordIndex(0);
  size_t src = plic_claim(&p, idx);

  fprint(&w, ": PLIC source: ");
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
