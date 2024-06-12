#include "plic.h"

struct PlicDriver PlicDriver(size_t base) {
  return (struct PlicDriver){
      .base = (uint32_t *)base,
  };
}

void plic_priority(struct PlicDriver *p, uint32_t src, size_t prio) {
  p->base[PLIC_PRIORITY + src] = prio;
}

void plic_enable(struct PlicDriver *p, size_t idx, size_t src) {
  // assume 32 bit word size
  p->base[PLIC_ENABLE + ((idx + src) >> 5)] |= 1 << (src & 31);
}

void plic_threshold(struct PlicDriver *p, size_t idx, size_t th) {
  p->base[PLIC_THRESHOLD + idx] = th;
}

size_t plic_claim(struct PlicDriver *p, size_t idx) {
  return p->base[PLIC_CLAIM + idx];
}

void plic_complete(struct PlicDriver *p, size_t idx, size_t src) {
  p->base[PLIC_COMPLETE + idx] = src;
}
