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
#define DEBUG_CHECK(x, y) SAFE(if (x) { fatal((y)); })
#define DEBUG_CHECK1(x, y, a) SAFE(if (x) { fatal((y), (a)); })
#define DEBUG_CHECK2(x, y, a, b) SAFE(if (x) { fatal((y), (a), (b)); })
#else
#define DEBUG_CHECK(x, y)
#define DEBUG_CHECK1(x, y, a)
#define DEBUG_CHECK2(x, y, a, b)
#endif

extern unsigned int debug_level;

void handle_debug_level(char *);
void debug_level_set(const char *);
void debug_level_clear(const char *);

#define debug(x, ...)                                                            \
  do {                                                                           \
    if (debug_level & DBG_##x) {                                                 \
      char _buf[1024], _tbuf[64];                                                \
      time_t _rawtime;                                                           \
      time(&_rawtime);                                                           \
      strftime(_tbuf, sizeof(_tbuf), "%Y-%m-%d %H:%M:%S", localtime(&_rawtime)); \
      snprintf(_buf, sizeof(_buf), __VA_ARGS__);                                 \
      debug_message("[%s] %s:%d %s", _tbuf, __FILE__, __LINE__, _buf);           \
    }                                                                            \
  } while (0)

/* bit sets here */
#define DBG_call_out 1u
#define DBG_addr_server 2u
#define DBG_d_flag 4u
#define DBG_connections 8u
#define DBG_mapping 16u
#define DBG_sockets 32u
#define DBG_comp_func_tab 64u
#define DBG_LPC 128u
#define DBG_LPC_line 256u
#define DBG_event 512u
#define DBG_dns 1024u
#define DBG_file 2048u
#define DBG_add_action 4096u
#define DBG_telnet 8192u

#define DBG_DEFAULT (DBG_connections | DBG_telnet)

struct debug_t {
  const char *name;
  int bit;
};

extern const debug_t levels[];
extern const int sizeof_levels;

#endif /* SRC_BASE_LOG_H_ */
