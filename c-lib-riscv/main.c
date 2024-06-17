#include "config.h"
#include "fmt.h"
#include "riscv.h"
#include <stdint.h>

// implemented in trap.S
extern void trap_direct();

int main();
void trap_zero();
void trap_external();
void plic_init();
void uart_init();

#define print(s)                                                               \
  for (const char *p = s; *p; p++)                                             \
    uart_write(*p);

void start() {
  print("init: machine\n");

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

int main() {
  print("init: supervisor\n");

  csrw(stvec, (size_t)trap_direct);
  csrs(sie, XIE_SEIE);
  csrs(sstatus, XSTATUS_SIE);

  plic_init();
  uart_init();

  print("done: waiting for interrupts\n");
  while (1)
    wfi;
}

void trap_zero() {
  print("trap: ");

  struct XCause cause = SCause();
  if (cause.is_interrupt) {
    print("interrupt: ");
    print(irq_names[cause.code]);
    print("\n");

    trap_external();
    return;
  }

  print("exception: ");
  print(exception_names[cause.code]);
  print("\n");

  while (1)
    ;
}

void trap_external() {
  size_t ctx = plic_context(hartid(), PLIC_MODE_S);
  size_t src = *(uint32_t *)plic_warl(PLIC_BASE, PLIC_CLAIM_OFFSET, ctx);

  if (src == PLIC_SRC_UART) {
    // TODO: check uart IIR
    print("uart: ");
    uart_write(uart_read());
    print("\n");
  }

  // for now, simply compelte any interupt
  *(uint32_t *)plic_warl(PLIC_BASE, PLIC_COMPLETE_OFFSET, ctx) = src;
}

void plic_init() {
  size_t ctx = plic_context(hartid(), 1);

  *(uint32_t *)plic_array(PLIC_BASE, PLIC_PRIORITY_OFFSET, PLIC_SRC_UART) = 1;

  *(uint32_t *)plic_bits(PLIC_BASE, PLIC_ENABLE_OFFSET, ctx, PLIC_SRC_UART) |=
      1 << (PLIC_SRC_UART % 32);

  *(uint32_t *)plic_warl(PLIC_BASE, PLIC_THRESHOLD_OFFSET, ctx) = 0;
}

void uart_init() {
  *(volatile uint8_t *)(UART_BASE + UART_FCR) |=
      UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RX | UART_FCR_CLEAR_TX |
      UART_FCR_TRIGGER_8b;

  *(volatile uint8_t *)(UART_BASE + UART_LCR) |= UART_LCR_WORD_LEN_8b;

  *(volatile uint8_t *)(UART_BASE + UART_IER) |= UART_IER_RX_AVAIL;
}
