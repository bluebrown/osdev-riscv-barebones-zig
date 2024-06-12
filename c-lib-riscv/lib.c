#include "lib.h"

void fprint(struct Writer *w, const char *s) {
  while (*s)
    w->write(w->impl, *s++);
}

char *itoa(size_t base, size_t num, char *buf) {
  char *p = buf + 35;

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
