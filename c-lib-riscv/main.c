#include "config.h"
#include "fmt.h"
#include "plic.h"
#include "riscv.h"
#include "uart.h"

int main();
void trap0();
// implemented in trap.S
extern void trapDirect();

void start() {
  csrw(satp, 0);
  csrw(pmpaddr0, ~0);
  csrw(pmpcfg0, 0xf);

  csrw(mepc, (size_t)main);
  csrc(mstatus, MSTATUS_MPP_M);
  csrs(mstatus, MSTATUS_MPP_S);

  csrs(mstatus, MSTATUS_SIE);
  csrs(mideleg, 0xFFFF);
  csrs(medeleg, 0xFFFF);

  mret;
}

int main() {
  csrw(stvec, (size_t)trapDirect);
  csrs(sie, SIE_SEIE);

  struct UartDriver u = UartDriver(UART_BASE);
  struct Writer *w = &(struct Writer){&u, (Write *)uart_rtxWrite};

  uart_fifoInit(&u);

  logln("supervisor init");

  plicInit(w);
  uart_irqEnableSet(&u, 1);
  logln("interupts enabled");

  logln("done, waiting for interupts");

  // asm volatile("ebreak");

  while (1)
    wfi;
}

void trap0() {
  struct UartDriver u = {(uint8_t *)UART_BASE};
  struct Writer *w = &(struct Writer){&u, (Write *)uart_rtxWrite};

  trace("IRQ", "trap0");

  struct XCause cause = SCause();

  tracex("kind", cause.is_interrupt);
  tracex("code", cause.code);

  if (cause.is_interrupt) {
    trace("name", irq_names[cause.code]);
    trapExternal();
    return;
  }

  traceln("name", exception_names[cause.code]);
}
