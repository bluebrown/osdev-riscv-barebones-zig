#ifndef FMT_H
#define FMT_H

#include <stddef.h>

typedef void Write(void *, char);

struct Writer {
  void *impl;
  Write *write;
};

void fprint(struct Writer *w, const char *s);

static const char *hexchars = "0123456789abcdef";

char *itoa(size_t base, size_t num, char *buf);

#define log(msg) fprint(w, msg);
#define logln(msg)                                                             \
  log(msg);                                                                    \
  log("\n");

#define trace(msg, val)                                                        \
  fprint(w, msg);                                                              \
  fprint(w, ":");                                                              \
  fprint(w, val);                                                              \
  fprint(w, " -> ");

#define traceln(msg, val)                                                      \
  fprint(w, msg);                                                              \
  fprint(w, ":");                                                              \
  fprint(w, val);                                                              \
  fprint(w, "\n");

#define tracex(msg, num) trace(msg, itoa(16, num, (char[35]){0}));
#define tracexln(msg, num) traceln(msg, itoa(16, num, (char[35]){0}));

#endif
