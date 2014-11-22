#include "log.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "rc.h"  // for CONFIG_*

#define E(x) \
  { #x, DBG_##x }

debug_t levels[] = {
    E(call_out), E(d_flag),   E(connections), E(mapping), E(sockets), E(comp_func_tab),
    E(LPC),      E(LPC_line), E(event),       E(dns),     E(file),    E(add_action),
};

const int sizeof_levels = (sizeof(levels) / sizeof(levels[0]));

void debug_message(const char *fmt, ...) {
  static FILE *debug_message_fp = nullptr;
  static char deb_buf[1024];
  static char *deb = deb_buf;
  va_list args;

  if (!debug_message_fp) {
    /*
     * check whether config file specified this option
     */
    auto dlf = CONFIG_STR(__DEBUG_LOG_FILE__);
    if (strlen(dlf)) {
      snprintf(deb, 1023, "%s/%s", CONFIG_STR(__LOG_DIR__), dlf);
    } else {
      snprintf(deb, 1023, "%s/debug.log", CONFIG_STR(__LOG_DIR__));
    }
    deb[1023] = 0;
    while (*deb == '/') {
      deb++;
    }
    debug_message_fp = fopen(deb, "w");
    if (!debug_message_fp) {
      perror(deb);
      /* use stderr by default. */
      debug_message_fp = stderr;
    }
  }

  char message[1024];

  va_start(args, fmt);
  vsnprintf(message, 1024, fmt, args);
  va_end(args);

  if (debug_message_fp != stderr) {
    fprintf(debug_message_fp, "%s", message);
    fflush(debug_message_fp);
  }
  fprintf(stderr, "%s", message);
  fflush(stderr);
}

int debug_level = 0;

#define NELEM(x) (sizeof(x) / sizeof(x[0]))

void debug_level_set(const char *level) {
  unsigned int i;

  for (i = 0; i < NELEM(levels); i++) {
    if (strcmp(level, levels[i].name) == 0) {
      debug_level |= levels[i].bit;
      return;
    }
  }
}

void debug_level_clear(const char *level) {
  unsigned int i;

  for (i = 0; i < NELEM(levels); i++) {
    if (strcmp(level, levels[i].name) == 0) {
      debug_level &= ~levels[i].bit;
      return;
    }
  }
}
