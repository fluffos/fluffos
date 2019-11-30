// Timer implementation with POSIX timer, only works on linux
#ifdef __linux__

#include "base/std.h"

#include "vm/internal/posix_timers.h"

#include <cstdio>   // for perror()
#include <cstdlib>  // for exit()
#include <sys/signal.h>
#include <time.h>

#include "vm/internal/eval_limit.h"

static timer_t eval_timer_id;
/*
 * SIGALRM handler.
 */
void sigalrm_handler(int sig, siginfo_t *si, void *uc) {
  if (!si->si_value.sival_ptr) {
    outoftime = 1;
  }
} /* sigalrm_handler() */
/* Called by main() to initialize all timers (currently only eval_cost) */
void init_posix_timers(void) {
  struct sigevent sev;
  struct sigaction sa;
  /* This mimics the behavior of setitimer in uvalarm.c */
  memset(&sev, 0, sizeof(sev));
  sev.sigev_signo = SIGVTALRM;
  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_value.sival_ptr = NULL;

  int i = -1;
// Only CLOCK_REALTIME is standard.
#if defined(CLOCK_MONOTONIC_COARSE)
  i = timer_create(CLOCK_MONOTONIC_COARSE, &sev, &eval_timer_id);
#endif
#if defined(CLOCK_MONOTONIC)
  if (i < 0) {
    i = timer_create(CLOCK_MONOTONIC, &sev, &eval_timer_id);
  }
#endif
  if (i < 0) {
    i = timer_create(CLOCK_REALTIME, &sev, &eval_timer_id);
  }
  if (i < 0) {
    perror("init_posix_timers: timer_create");
    exit(-1);
  }

  sa.sa_sigaction = sigalrm_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO;

  i = sigaction(SIGVTALRM, &sa, NULL);
  if (i < 0) {
    perror("init_posix_timers: sigaction");
    exit(-1);
  }
}

/* Set the eval_timer to the given number of microseconds */
void posix_eval_timer_set(uint64_t micros) {
  struct itimerspec it;

  it.it_interval.tv_sec = 0;
  it.it_interval.tv_nsec = 0;

  it.it_value.tv_sec = micros / 1000000;
  it.it_value.tv_nsec = micros % 1000000 * 1000;

  timer_settime(eval_timer_id, 0, &it, NULL);
}

/* Return the number of microseconds remaining on the eval_timer */
uint64_t posix_eval_timer_get(void) {
  struct itimerspec it;

  if (timer_gettime(eval_timer_id, &it) < 0) {
    return 100;
  }

  return it.it_value.tv_sec * static_cast<uint64_t>(1000000) + it.it_value.tv_nsec / 1000;
}

#endif  // __linux__
