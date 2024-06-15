#include <stddef.h>
#include <stdint.h>

struct UartDriver {
  uint8_t *ports;
};

struct UartDriver UartDriver(size_t base);

void uart_rtxWrite(struct UartDriver *uart, char c);

char uart_rtxRead(struct UartDriver *uart);

void uart_rtxFlush(struct UartDriver *uart);

void uart_fifoInit(struct UartDriver *uart);

uint8_t uart_fifoStatus(struct UartDriver *uart);

void uart_irqEnableSet(struct UartDriver *uart, uint8_t flags);

void uart_irqEnableClear(struct UartDriver *uart, uint8_t flags);

uint8_t uart_irqIsPending(struct UartDriver *uart);
