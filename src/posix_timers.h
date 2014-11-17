#ifndef POSIX_TIMERS_H
#define POSIX_TIMERS_H

void sigalrm_handler(int, siginfo_t *, void *);

void init_posix_timers(void);
void posix_eval_timer_set(uint64_t micros);
uint64_t posix_eval_timer_get(void);

#endif
