#ifndef _OUTBUF_H_
#define _OUTBUF_H_

struct outbuffer_t {
  int real_size;
  char *buffer;
};

void outbuf_zero(outbuffer_t *);
void outbuf_add(outbuffer_t *, const char *);
void outbuf_addchar(outbuffer_t *, char);
void outbuf_addv(outbuffer_t *, const char *, ...);
void outbuf_fix(outbuffer_t *);
int outbuf_extend(outbuffer_t *, int);

#endif
