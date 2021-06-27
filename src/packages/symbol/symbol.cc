#include <stdio.h>
#include "symbol.h"
#include <string.h>
#include <fcntl.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

static FILE *out = NULL;

static char buff[1024];

static int enable = 0;

const char *dir = "log/symbol/";

void symbol_enable(int op) { enable = op; }

void symbol_start(const char *filename) {
  int i;
  if (!enable) return;
  if (out != NULL) symbol_end();
  sprintf(buff, "%s%s", dir, filename);
#ifdef _WIN32
  _mkdir(dir);
#else
  mkdir(dir, 0700);
#endif
  for (i = strlen(dir); i < strlen(buff) && i < sizeof(buff); i++) {
    if (buff[i] == '/') buff[i] = '#';
  }
  out = fopen(buff, "w");
}

void symbol_end() {
  if (!enable) return;
  if (out == NULL) return;
  fclose(out);
  out = NULL;
}

void symbol_record(int op, const char *file, int line, const char *detail) {
  if (!enable) return;
  if (out == NULL) return;

  fprintf(out, "%d %s %d %s\n", op, file, line, detail);
}
