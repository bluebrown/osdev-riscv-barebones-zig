#include <stddef.h>
#include <stdint.h>

unsigned char *volatile UART = (unsigned char *)0x10000000;
uint32_t *volatile PLIC = (uint32_t *)0xc000000;

void print(const char *s);
void print_exception(int code);
void print_interupt(int code);
char *itoa(size_t base, size_t num);
void irq_handler();

static inline size_t get_context() {
  size_t hartid;
  asm volatile("csrr %0, mhartid" : "=r"(hartid));
  return (hartid << 1) | 0; // machine mode 0
}

int main() {
  print("init\n");

  // get the context of the current hardware thread.
  // assumes machine mode.
  size_t ctx = get_context();

  // the uart irq id, can be found in qemus device tree:
  //
  //     qemu-system-riscv32 -nographic -machine virt,dumpdtb=virt.out
  //     dtdump virt.out > qemu.fdt.txt
  //
  size_t src = 0xA;

  // set trap vector
  asm volatile("csrw mtvec, %0" ::"r"(&irq_handler));

  // configure PLIC
  // note diving by wordsize (4) to be able to
  // use array notation PLIC[index]

  // raise priority
  PLIC[(0x4 * src) >> 2] = 1;

  // set enable bit
  // interrupt ID N is stored in bit (N mod 32) of word (N/32)
  PLIC[(0x002000 + 0x80 * ctx + (src >> 5)) >> 2] |= 0b1 << (src & 31);

  // lower treshold
  PLIC[(0x200000 + 0x1000 * ctx) >> 2] = 0;

  // enable external interupts
  asm volatile("csrs mie, %0" ::"r"(1 << 11));
  asm volatile("csrs mstatus, %0" ::"r"(1 << 3));

  // enable all interupts on UART
  UART[1] |= 0xf;

  print("init done\n");
  while (1)
    asm volatile("wfi");
}

void irq_handler() {
  print("irq: ");
  int mcause;
  asm volatile("csrr %0, mcause" : "=r"(mcause));

  int is_interrupt = mcause < 0;
  size_t code = mcause & 0x7FFFFFFF;

  print(itoa(2, code));
  print(": ");

  if (!is_interrupt) {
    print("exception: ");
    print_exception(code);
    while (1)
      asm volatile("wfi");
  }

  print("interrupt: ");
  print_interupt(code);
  print(": ");

  // handle external interupt
  if (code == 11) {
    // claim the interupt
    size_t id = *(uint32_t *)(0xc000000 + 0x200004 + 0x1000 * get_context());
    // handle the interupt
    if (id == 0xA) {
      print("UART: ");
      int reason = (UART[2] >> 1) & 0b111;
      print(itoa(2, reason));
    } else {
      print("unknown interrupt");
    }
  }

  print("\n");

  // NOTE: this is probably broken as stack and registers have been
  // corrupted by the interrupt. This is just a simple example.
  asm volatile("mret");
}

void print(const char *s) {
  while (*s) {
    while ((UART[5] & 0x20) == 0)
      ;
    *UART = *s++;
  }
}

void print_exception(int code) {
  switch (code) {
  case 1:
    print("Instruction access fault");
    break;
  case 2:
    print("Illegal instruction");
    break;
  case 5:
    print("Load access fault");
    break;
  case 7:
    print("Store/AMO access fault");
    break;
  case 12:
    print("Instruction page fault");
    break;
  case 13:
    print("Load page fault");
    break;
  case 15:
    print("Store/AMO page fault");
    break;
  default:
    print("unknown exception");
    break;
  }
}

void print_interupt(int code) {
  switch (code) {
  case 3:
    print("Machine software interrupt");
    break;
  case 7:
    print("Machine timer interrupt");
    break;
  case 11:
    print("Machine external interrupt");
    break;
  case 13:
    print("Counter overflow interrupt");
    break;
  default:
    print("unknown interrupt");
    break;
  }
}

const char *hexchars = "0123456789abcdef";

char *itoa(size_t base, size_t num) {
  // stunt is performed by a clown,
  // do not try this at home.
  static char buf[32 + 3];
  char *p = buf + sizeof(buf);

  *--p = '\0';

  do {
    *--p = hexchars[num % base];
    num /= base;
  } while (num);

  if (base == 2)
    *--p = 'b';
  if (base == 8)
    *--p = 'o';
  if (base == 10)
    *--p = 'd';
  if (base == 16)
    *--p = 'x';

  *--p = '0';

  return p;
}
