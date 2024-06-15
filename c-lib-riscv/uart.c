#include "uart.h"
#include "riscv.h"

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
