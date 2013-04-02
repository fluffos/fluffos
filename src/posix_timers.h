#ifndef POSIX_TIMERS_H
#define POSIX_TIMERS_H

#ifdef POSIX_TIMERS
void init_posix_timers(void);
void posix_eval_timer_set(LPC_INT micros);
LPC_INT posix_eval_timer_get(void);
#endif

#endif
