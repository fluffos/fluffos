#include "std.h"
#include "main.h"
#include "comm.h"
#include "uvalarm.h"
#include <time.h>
#include "backend.h"

int outoftime = 0;
struct timeval tv;
int lasttime;

void set_eval(int etime){
#ifndef WIN32
  long diff;
  gettimeofday(&tv, NULL);
  if((diff = tv.tv_sec-current_time) > 1){
	  diff *= 1000000;
	  if(diff > max_cost*100){
		  //put some hard limit to eval times
		  outoftime = 1;
		  return;
	  }
  }
  signal(SIGVTALRM, sigalrm_handler);
  uvalarm(etime, 0);
#endif
  outoftime = 0;

}

int get_eval(){
#ifndef WIN32
  struct timeval now;
  gettimeofday(&now, NULL);
  return max_cost - (1000000*(now.tv_sec - tv.tv_sec))-(now.tv_usec - tv.tv_usec);
#else
  return 100;
#endif
}

