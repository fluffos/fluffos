#include "std.h"
#include "main.h"
#include "comm.h"
#include "uvalarm.h"
#include <time.h>
#include "backend.h"
#include "posix_timers.h"

int outoftime = 0;
struct timeval tv;
int lasttime;
LPC_INT max_cost;

void set_eval(int etime){
#ifndef WIN32
	long diff;
	gettimeofday(&tv, NULL);
	if((diff = tv.tv_sec-current_time) > 1){
		diff *= 1000000;
		if(diff > max_cost*100L){
			//put some hard limit to eval times
			debug(d_flag, ("difft:%ld, max_cost:%d", diff, max_cost));
			outoftime = 1;
			return;
		}
	}
#ifdef POSIX_TIMERS
	posix_eval_timer_set(etime);
#else
	signal(SIGVTALRM, sigalrm_handler);
	uvalarm(etime, 0);
#endif
#endif
	outoftime = 0;
}

LPC_INT get_eval(){
#ifndef WIN32
#ifdef POSIX_TIMERS
	return posix_eval_timer_get();
#else
	struct timeval now;
	gettimeofday(&now, NULL);
	return max_cost - (1000000*(now.tv_sec - tv.tv_sec))-(now.tv_usec - tv.tv_usec);
#endif
#else
	return 100;
#endif
}

