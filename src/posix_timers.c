#include "std.h"
#include <signal.h>
#include <time.h>
#include "comm.h"
#include "posix_timers.h"
#include "eval.h"
#include "backend.h"

static timer_t eval_timer_id,  hb_timer_id;
/*
 * SIGALRM handler.
 */
#ifdef SIGNAL_FUNC_TAKES_INT
void sigalrm_handler (int sig, siginfo_t *si, void *uc)
#else
void sigalrm_handler()
#endif
{
#ifndef POSIX_TIMERS
	outoftime = 1;
#else
	if(!si->si_value.sival_ptr)
		outoftime = 1;
	else{
		if(si->si_value.sival_ptr == call_heart_beat){
			time_for_hb++;
		}
	}
#endif
}                               /* sigalrm_handler() */
#ifdef POSIX_TIMERS
/* Called by main() to initialize all timers (currently only eval_cost) */
void init_posix_timers(void)
{
	struct sigevent sev;
	struct sigaction sa;
	int i;

	/* This mimics the behavior of setitimer in uvalarm.c */
    memset(&sev, 0, sizeof(sev));
	sev.sigev_signo = SIGVTALRM;
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_value.sival_ptr = NULL;
#ifndef __CYGWIN__
	i = timer_create(CLOCK_THREAD_CPUTIME_ID, &sev, &eval_timer_id);
#else
	i = timer_create(CLOCK_REALTIME, &sev, &eval_timer_id);
#endif
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
    double interv = HEARTBEAT_INTERVAL;
    struct itimerspec it;
    it.it_interval.tv_sec = (int)interv;
    it.it_interval.tv_nsec = (interv-(double)it.it_interval.tv_sec)*1000000000;
    it.it_value.tv_sec = 0;
    it.it_value.tv_nsec = 1;
    sev.sigev_value.sival_ptr = call_heart_beat;
    i = timer_create(CLOCK_MONOTONIC, &sev, &hb_timer_id);
    if (i < 0) {
    	perror("init_posix_timers: timer_create");
    	exit(-1);
    }
    timer_settime(hb_timer_id, 0, &it, NULL);
}

/* Set the eval_timer to the given number of microseconds */
void posix_eval_timer_set(int micros)
{
	struct itimerspec it;

	it.it_interval.tv_sec = 0;
	it.it_interval.tv_nsec = 0;

	it.it_value.tv_sec = micros / 1000000;
	it.it_value.tv_nsec = micros % 1000000 * 1000;

	timer_settime(eval_timer_id, 0, &it, NULL);
}

/* Return the number of microseconds remaining on the eval_timer */
int posix_eval_timer_get(void)
{
	struct itimerspec it;

	if (timer_gettime(eval_timer_id, &it) < 0) {
		return 100;
	}

	return it.it_value.tv_sec * 1000000 + it.it_value.tv_nsec / 1000;
}


#endif
