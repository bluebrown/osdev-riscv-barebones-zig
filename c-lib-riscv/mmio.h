#include <stdint.h>

#ifdef BOARD_QEMU_RISCV_VIRT
#define PLIC_BASE (0x0c000000)
#endif
#define PLIC_TYPE (uint32_t)
#define PLIC_PRIORITY_OFFSET (0x0000)
#define PLIC_PRIORITY_ALIGNMENT (0x4)
#define PLIC_ENABLE_OFFSET (0x2000)
#define PLIC_ENABLE_ALIGNMENT (0x80)
#define PLIC_THRESHOLD_OFFSET (0x200000)
#define PLIC_THRESHOLD_ALIGNMENT (0x1000)
#define PLIC_CLAIM_OFFSET (0x200004)
#define PLIC_CLAIM_ALIGNMENT (0x1000)
#define PLIC_COMPLETE_OFFSET (0x200004)
#define PLIC_COMPLETE_ALIGNMENT (0x1000)

#ifdef BOARD_QEMU_RISCV_VIRT
#define UART_BASE (0x10000000)
#define PLIC_SRC_UART (10)
#endif
#define UART_TYPE (uint8_t)
#define UART_RBR (0x0)
#define UART_THR (0x0)
#define UART_IER (0x4)
#define UART_DLL (0x0)
#define UART_DLH (0x4)
#define UART_FCR (0x8)
#define UART_LCR (0xc)
#define UART_MCR (0x10)
#define UART_LSR (0x14)
#define UART_MSR (0x18)
#define UART_SCR (0x1c)
