#include <stddef.h>

typedef void Write(void *, char);

struct Writer {
  void *impl;
  Write *write;
};

void fprint(struct Writer *w, const char *s);

static const char *hexchars = "0123456789abcdef";

char *itoa(size_t base, size_t num, char *buf);
