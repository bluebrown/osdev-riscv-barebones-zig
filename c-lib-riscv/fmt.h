#ifndef FMT_H
#define FMT_H

#include <stddef.h>

static const char *hexchars = "0123456789abcdef";

char *itoa(size_t base, size_t num, char *buf);

#endif
