#include "config.h"
#include "fmt.h"
#include "riscv.h"
#include "uart.h"

void plicInit(struct Writer *w) {
  log("PLIC: ");

  size_t uart_src = 10;

  int hart;
  asm volatile("mv %0, tp" : "=r"(hart));

  size_t context = plicContext(hart, 1);
  tracex("context", context);

  size_t addr = 0;

  addr = plicArray(PLIC_BASE, PLIC_PRIORITY_OFFSET, uart_src);
  *(uint32_t *)addr = 1;
  tracex("priority", addr);

  addr = plicBits(PLIC_BASE, PLIC_ENABLE_OFFSET, context, uart_src);
  *(uint32_t *)addr = 1 << (uart_src % 32);
  tracex("enable", addr);

  addr = plicWarl(PLIC_BASE, PLIC_THRESHOLD_OFFSET, context);
  *(uint32_t *)addr = 0;
  tracex("threshold", addr);

  addr = plicWarl(PLIC_BASE, PLIC_CLAIM_OFFSET, context);
  tracex("claim", addr);

  addr = plicWarl(PLIC_BASE, PLIC_COMPLETE_OFFSET, context);
  tracexln("complete", addr);
}

void trapExternal() {
  struct UartDriver u = {(uint8_t *)UART_BASE};
  struct Writer *w = &(struct Writer){&u, (Write *)uart_rtxWrite};

  int hart;
  asm volatile("mv %0, tp" : "=r"(hart));

  size_t context = plicContext(hart, 1);
  size_t src = *(uint32_t *)plicWarl(PLIC_BASE, PLIC_CLAIM_OFFSET, context);

  tracex("PLIC source", src);

  if (src == PLIC_SRC_UART) {
    // TODO: check uart IIR
    log("\nUART received data: ");
    uart_rtxWrite(&u, uart_rtxRead(&u));
  }

  *(uint32_t *)plicWarl(PLIC_BASE, PLIC_COMPLETE_OFFSET, context) = src;

  logln("\n");
}
