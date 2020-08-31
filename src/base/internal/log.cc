#include "log.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cerrno>

#include "rc.h"  // for CONFIG_*

#define E(x) \
  { #x, DBG_##x }

const debug_t levels[] = {
    E(call_out), E(d_flag),   E(connections), E(mapping), E(sockets), E(comp_func_tab),
    E(LPC),      E(LPC_line), E(event),       E(dns),     E(file),    E(add_action),
};

const int sizeof_levels = (sizeof(levels) / sizeof(levels[0]));

void debug_message(const char *fmt, ...) {
  static FILE *debug_message_fp = nullptr;
  static char deb_buf[1024];
  static char *deb = deb_buf;

  if (!debug_message_fp) {
    /*
     * check whether config file specified this option
     */
    auto dlf = CONFIG_STR(__DEBUG_LOG_FILE__);
    if (dlf && strlen(dlf)) {
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
      fprintf(stderr, "Unable to open log file: \"%s\", error: \"%s\". Use stderr only.\n", deb,
              strerror(errno));
      /* use stderr by default. */
      debug_message_fp = stderr;
    }
  }

  va_list args1, args2;
  va_start(args1, fmt);
  va_copy(args2, args1);

  if (debug_message_fp != stderr) {
    vfprintf(debug_message_fp, fmt, args1);
    fflush(debug_message_fp);
  }
  va_end(args1);

#ifndef _WIN32
  vfprintf(stderr, fmt, args2);
  fflush(stderr);
#else
  // TODO: figure out how to output utf8 properly
  vfprintf(stderr, fmt, args2);
  fflush(stderr);
#endif
  va_end(args2);
}

unsigned int debug_level = 0;

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
