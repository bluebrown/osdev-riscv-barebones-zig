#include <stdio.h>

typedef void Write(void *, char);

struct Writer {
  enum { WRITER_ANY, WRITER_BUFFER } type;
  void *impl;
  Write *write;
};

void fprint(struct Writer *w, const char *s) {
  while (*s)
    w->write(w->impl, *s++);
}

struct SomeFile {
  FILE *f;
};

void SomeFile_write(struct SomeFile *w, const char c) { fputc(c, w->f); }

struct Buffer {
  char *buf;
  size_t cap;
  size_t off;
};

void Buffer_write(struct Buffer *b, const char c) {
  if (b->off < b->cap)
    b->buf[b->off++] = c;
}

int main() {
  struct Writer stdw = (struct Writer){
      .impl = &(struct SomeFile){stdout},
      .write = (Write *)SomeFile_write,
  };

  fprintf(stdout, "stdw tag: %d\n", stdw.type);

  struct Writer bufw = (struct Writer){
      .type = WRITER_BUFFER,
      .impl = &(struct Buffer){.buf = (char[1024]){}, .cap = 1024},
      .write = (Write *)Buffer_write,
  };

  fprint(&stdw, "console meat\n");
  fprint(&bufw, "buffered beefalo\n");
  if (bufw.type == WRITER_BUFFER)
    fprint(&stdw, ((struct Buffer *)bufw.impl)->buf);
}
