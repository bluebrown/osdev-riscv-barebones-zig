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

struct StandardOut {};

void StandardOut_write(struct StandardOut *w, char c) { fputc(c, stdout); }

struct Buffer {
  char *data;
  size_t cap;
  size_t pos;
};

void Buffer_write(struct Buffer *b, char c) {
  if (b->pos < b->cap)
    b->data[b->pos++] = c;
}

int main() {
  struct Writer stdw = (struct Writer){
      .impl = &(struct StandardOut){},
      .write = (Write *)StandardOut_write,
  };

  fprintf(stdout, "stdw tag: %d\n", stdw.type);

  struct Writer bufw = (struct Writer){
      .type = WRITER_BUFFER,
      .impl = &(struct Buffer){.data = (char[1024]){}, .cap = 1024},
      .write = (Write *)Buffer_write,
  };

  fprint(&stdw, "console meat\n");
  fprint(&bufw, "buffered beefalo\n");
  if (bufw.type == WRITER_BUFFER)
    fprint(&stdw, ((struct Buffer *)bufw.impl)->data);
}
