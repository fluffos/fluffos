#include "std.h"
#include "main.h"
#include "comm.h"

int outoftime = 0;

void set_eval(int time){
#ifndef WIN32
#ifdef SIGALRM
  signal(SIGALRM, sigalrm_handler);
#endif
  
#ifdef HAS_UALARM
  ualarm(max_cost, 0);
#else
  alarm(max_cost/1000000); /* defined in config.h */
#endif
#endif
  outoftime = 0;
}

int get_eval(){
  int ret;
#ifndef WIN32
#ifdef HAS_UALARM
  ret = ualarm(2*max_cost, 0);
  ualarm(ret, 0);
  return ret;
#else
  ret = alarm(0);
  alarm(ret);
  return ret*1000000;
#endif
#endif
}

