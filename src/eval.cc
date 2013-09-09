#include "std.h"
#include "main.h"
#include "comm.h"
#include "uvalarm.h"
#include <time.h>
#include "backend.h"
#include "posix_timers.h"

static struct timeval tv;
int outoftime = 0;
LPC_INT max_cost;

void set_eval(LPC_INT etime)
{
#ifdef POSIX_TIMERS
  posix_eval_timer_set(etime);
#else
  signal(SIGVTALRM, (__sighandler_t)sigalrm_handler);
  uvalarm(etime, 0);
#endif
  outoftime = 0;
}

LPC_INT get_eval()
{
#ifndef WIN32
#ifdef POSIX_TIMERS
  return posix_eval_timer_get();
#else
  struct timeval now;
  gettimeofday(&now, NULL);
  return max_cost - ((LPC_INT)(1000000) * (now.tv_sec - tv.tv_sec)) - (now.tv_usec - tv.tv_usec);
#endif
#else
  return 100;
#endif
}

