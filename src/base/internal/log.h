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

void reset_debug_message_fp();
void debug_message(const char *, ...);

#define SAFE(x) \
  do {          \
    x           \
  } while (0)

#ifdef DEBUG
#define DEBUG_CHECK(x, y) SAFE(if (x) { fatal((y)); })
#define DEBUG_CHECK1(x, y, a) SAFE(if (x) { fatal((y), (a)); })
#define DEBUG_CHECK2(x, y, a, b) SAFE(if (x) { fatal((y), (a), (b)); })
#define DEBUG_FATAL(x) SAFE(fatal(x);)
#else
#define DEBUG_CHECK(x, y)
#define DEBUG_CHECK1(x, y, a)
#define DEBUG_CHECK2(x, y, a, b)
#define DEBUG_FATAL(x) SAFE(debug_message(x);)
#endif

extern unsigned int debug_level;

void handle_debug_level(char *);
void debug_level_set(const char *);
void debug_level_clear(const char *);

#define debug(x, ...)                                                                    \
  do {                                                                                   \
    if (debug_level & DBG_##x) {                                                         \
      char _buf[1024], _tbuf[64];                                                        \
      time_t _rawtime;                                                                   \
      time(&_rawtime);                                                                   \
      struct tm res = {};                                                                \
      strftime(_tbuf, sizeof(_tbuf), "%Y-%m-%d %H:%M:%S", localtime_r(&_rawtime, &res)); \
      snprintf(_buf, sizeof(_buf), __VA_ARGS__);                                         \
      debug_message("[%s] %s:%d %s", _tbuf, __FILE__, __LINE__, _buf);                   \
    }                                                                                    \
  } while (0)

/* bit sets here */
#define DBG_all ~0u
#define DBG_call_out 1u << 0
#define DBG_addr_server 1u << 1
#define DBG_d_flag 1u << 2
#define DBG_connections 1u << 3
#define DBG_mapping 1u << 4
#define DBG_sockets 1u << 5
#define DBG_comp_func_tab 1u << 6
#define DBG_LPC 1u << 7
#define DBG_LPC_line 1u << 8
#define DBG_event 1u << 9
#define DBG_dns 1u << 10
#define DBG_file 1u << 11
#define DBG_add_action 1u << 12
#define DBG_telnet 1u << 13
#define DBG_websocket 1u << 14
// remember to add new entry to levels in log.cc!

#define DBG_DEFAULT (DBG_connections | DBG_telnet)

struct debug_t {
  const char *name;
  unsigned int bit;
};

extern const debug_t levels[];
extern const int sizeof_levels;

#endif /* SRC_BASE_LOG_H_ */
