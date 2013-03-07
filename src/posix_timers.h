#ifndef FLUFF_POSIX_TIMERS_H
#define FLUFF_POSIX_TIMERS_H

#ifdef POSIX_TIMERS
void init_posix_timers(void);
void posix_eval_timer_set(int micros);
int posix_eval_timer_get(void);
#endif

#endif
