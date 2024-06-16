#include "config.h"
#include "fmt.h"
#include "plic.h"
#include "riscv.h"
#include "uart.h"
#include <stdint.h>

int main();

void start() {
  // clear all config in satp. this sets the mode to bare, which disables
  // protection and translation
  csrc(satp, 0);

  // pmp config

  // If TOR is selected, the associated address register forms the top of
  // the address range, and the preceding PMP address register forms the
  // bottom of the address range. For pmp0 the lower bound is 0.

  // top of address spec is the maximum address, fow now. set all bits set to 1
  csrw(pmpaddr0, ~0);

  // set the pmpcfg0 to TOR and allow all access,
  // but dont lock the config
  csrw(pmpcfg0, PMPCFG_A_TOR | PMPCFG_R | PMPCFG_W | PMPCFG_X);

  // setup the previous mode and program counter
  csrw(mepc, (size_t)main);
  csrc(mstatus, XSTATUS_MPP_M);
  csrs(mstatus, XSTATUS_MPP_S);

  // delegate interupts
  // the bits in deleg register enable certain interupts.
  // it is implementation specific which ones exit.
  // set this to all 1s to enable all interupts
  csrs(mideleg, ~0);
  csrs(medeleg, ~0);

  // use mret to jump to main with the new priviledge level
  mret;
}

// implemented in trap.S
extern void trapDirect();

int main() {
  csrw(stvec, (size_t)trapDirect);
  csrs(sie, XIE_SEIE);
  csrs(sstatus, XSTATUS_SIE);

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
