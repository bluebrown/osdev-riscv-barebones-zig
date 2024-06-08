unsigned char *UART = (unsigned char *)0x10000000;

int main() {
  char *msg = "Hello, world!\n";

  while (*msg)
    *UART = *msg++;

  while (1)
    asm volatile("wfi");
}
