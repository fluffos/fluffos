#include "std.h"
#include "main.h"
#include "comm.h"
#include <time.h>
#include "backend.h"
#include "posix_timers.h"

static struct timeval tv;
int outoftime = 0;
LPC_INT max_cost;

void set_eval(LPC_INT etime) {
  posix_eval_timer_set(etime);
  outoftime = 0;
}

LPC_INT get_eval() {
  return posix_eval_timer_get();
}
