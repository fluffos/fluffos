#ifndef FLUFFOS_ICONV_H
#define FLUFFOS_ICONV_H

typedef void *iconv_t;

struct translation {
  char *name;
  iconv_t incoming;
  iconv_t outgoing;
  struct translation *next;
};

char *translate(iconv_t tr, const char *mes, int inlen, int *outlen);
char *translate_easy(iconv_t tr, const char *mes);
struct translation *get_translator(const char *encoding);
#endif
