#include <stddef.h>
#include <stdint.h>

#ifdef BOARD_QEMU_RISCV_VIRT
#define PLIC_BASE (0x0c000000)
#endif
// 32 bit registers (word sized)
// context are 4k aligned
#define PLIC_PRIORITY (0x0000 / 0x4)
#define PLIC_ENABLE (0x2000 / 0x4)
#define PLIC_THRESHOLD (0x200000 / 0x4)
#define PLIC_CLAIM (0x200004 / 0x4)
#define PLIC_COMPLETE (0x200004 / 0x4)

static inline size_t plic_wordIndex(size_t mode) {
  int hart;
  asm volatile("csrr %0, mhartid" : "=r"(hart));
  // assume 4k page size, 4 byte word size, and
  // reduce the calculation. The full formula would be
  // (hart << 1 | mode) * PAGE_SIZE / WORD_SIZE
  return (hart << 1 | mode) << 10;
}

struct PlicDriver {
  uint32_t *volatile base;
};

struct PlicDriver PlicDriver(size_t base);

void plic_priority(struct PlicDriver *p, uint32_t src, size_t prio);

void plic_enable(struct PlicDriver *p, size_t idx, size_t src);

void plic_threshold(struct PlicDriver *p, size_t idx, size_t th);

size_t plic_claim(struct PlicDriver *p, size_t idx);

void plic_complete(struct PlicDriver *p, size_t idx, size_t src);
