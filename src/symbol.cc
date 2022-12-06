#include <cstdio>
#include "symbol.h"
#include <cstring>
#include <fcntl.h>
#include <string>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

static FILE *out = nullptr;

static int enable = 0;

std::string symbol_dir = "log/symbol/";

void symbol_enable(int op) { enable = op; }

void symbol_start(const char *filename) {
  std::string symbolfile;

  if (!enable) return;
  if (out != nullptr) symbol_end();
  symbolfile.append(symbol_dir);
  symbolfile.append(filename);

#ifdef _WIN32
  _mkdir(symbol_dir.c_str());
#else
  mkdir(symbol_dir.c_str(), 0700);
#endif
  for (size_t i = symbol_dir.length(); i < symbolfile.length(); i++) {
    if (symbolfile[i] == '/') symbolfile[i] = '#';
  }

  out = fopen(symbolfile.c_str(), "w");
}

void symbol_end() {
  if (!enable) return;
  if (out == nullptr) return;
  fclose(out);
  out = nullptr;
}

void symbol_record(int op, const char *file, int line, const char *detail) {
  if (!enable) return;
  if (out == nullptr) return;

  fprintf(out, "%d %s %d %s\n", op, file, line, detail);
}
