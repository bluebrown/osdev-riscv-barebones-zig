#include <stddef.h>
#include <stdint.h>

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

struct UartDriver UartDriver(size_t base);

void uart_rtxWrite(struct UartDriver *uart, char c);

char uart_rtxRead(struct UartDriver *uart);

void uart_rtxFlush(struct UartDriver *uart);

void uart_fifoInit(struct UartDriver *uart);

uint8_t uart_fifoStatus(struct UartDriver *uart);

void uart_irqEnableSet(struct UartDriver *uart, uint8_t flags);

void uart_irqEnableClear(struct UartDriver *uart, uint8_t flags);

uint8_t uart_irqIsPending(struct UartDriver *uart);
