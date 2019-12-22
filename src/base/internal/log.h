/*
 * log.h
 *
 *  Created on: Nov 16, 2014
 *      Author: sunyc
 */

#ifndef SRC_BASE_LOG_H_
#define SRC_BASE_LOG_H_

#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#include <stdio.h>  // for snprintf
#include "base/internal/options_incl.h"

// Defined by simulate.cc, this belongs to driver layer.
[[noreturn]] extern void fatal(const char *, ...);

void debug_message(const char *, ...);

#define SAFE(x) \
  do {          \
    x           \
  } while (0)

#ifdef DEBUG
#define DEBUG_CHECK(x, y) SAFE(if (x) { fatal((y)); });
#define DEBUG_CHECK1(x, y, a) SAFE(if (x) { fatal((y), (a)); });
#define DEBUG_CHECK2(x, y, a, b) SAFE(if (x) { fatal((y), (a), (b)); });
#else
#define DEBUG_CHECK(x, y)
#define DEBUG_CHECK1(x, y, a)
#define DEBUG_CHECK2(x, y, a, b)
#endif

extern int debug_level;

void handle_debug_level(char *);
void debug_level_set(const char *);
void debug_level_clear(const char *);

#define debug(x, ...)                                                          \
  if (debug_level & DBG_##x) {                                                 \
    char _buf[1024], _tbuf[64];                                                \
    time_t _rawtime;                                                           \
    time(&_rawtime);                                                           \
    strftime(_tbuf, sizeof(_tbuf), "%Y-%m-%d %H:%M:%S", localtime(&_rawtime)); \
    snprintf(_buf, sizeof(_buf), __VA_ARGS__);                                 \
    debug_message("[%s] %s:%d %s", _tbuf, __FILE__, __LINE__, _buf);           \
  }

/* bit sets here */
#define DBG_call_out 1
#define DBG_addr_server 2
#define DBG_d_flag 4
#define DBG_connections 8
#define DBG_mapping 16
#define DBG_sockets 32
#define DBG_comp_func_tab 64
#define DBG_LPC 128
#define DBG_LPC_line 256
#define DBG_event 512
#define DBG_dns 1024
#define DBG_file 2048
#define DBG_add_action 4096
#define DBG_telnet 8192

#define DBG_DEFAULT (DBG_connections | DBG_telnet)

struct debug_t {
  const char *name;
  int bit;
};

extern const debug_t levels[];
extern const int sizeof_levels;

#endif /* SRC_BASE_LOG_H_ */
