#include "base/internal/port.h"

#include "base/internal/rc.h"
#include "base/internal/rusage.h"
#include "config.h"

#include <random>
#include <unistd.h>

// Returns a pseudo-random number in the range 0 .. n-1
int64_t random_number(int64_t n) {
  static bool called = false;
  static std::mt19937_64 engine;

  if (!called) {
    std::random_device rd;
    engine.seed(rd());
    called = true;
  }

  std::uniform_int_distribution<int64_t> dist(0, n - 1);
  return dist(engine);
}

// Returns a secure random number in the range 0 .. n-1
int64_t secure_random_number(int64_t n) {
#ifdef __WIN32
  // On windows we trust default, since we use MINGW which is pretty recent
  static std::random_device rd;
#else
  // On linux & osx we use urandom by default
  static std::random_device rd("/dev/urandom");
#endif
  std::uniform_int_distribution<int64_t> dist(0, n - 1);
  return dist(rd);
}

/*
 * The function time() can't really be trusted to return an integer.
 * But MudOS uses the 'current_time', which is an integer number
 * of seconds. To make this more portable, the following functions
 * should be defined in such a way as to return the number of seconds since
 * some chosen year. The old behaviour of time(), is to return the number
 * of seconds since 1970.
 */

time_t get_current_time() { return time(nullptr); /* Just use the old time() for now */ }

/*
 * Get a microsecond clock sample.
 */
void get_usec_clock(long *sec, long *usec) {
  struct timeval tv {};

  gettimeofday(&tv, nullptr);
  *sec = tv.tv_sec;
  *usec = tv.tv_usec;
}

long get_cpu_times(unsigned long *secs, unsigned long *usecs) {
  struct rusage rus {};

  if (getrusage(RUSAGE_SELF, &rus) < 0) {
    return 0;
  }
  *secs = rus.ru_utime.tv_sec + rus.ru_stime.tv_sec;
  *usecs = rus.ru_utime.tv_usec + rus.ru_stime.tv_usec;

  return 1;
}

/* return the current working directory */
char *get_current_dir(char *buf, int limit) { return getcwd(buf, limit); /* POSIX */ }
