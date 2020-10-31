#ifndef PORT_H
#define PORT_H

#include <cstdint>  // for int64_t

#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

#ifdef __WIN32
#define gmtime_r(a, b) (gmtime_s(b, a) == 0 ? b : NULL)
#define localtime_r(a, b) (localtime_s(b, a) == 0 ? b : NULL)
#define ctime_r(a, b) (ctime_s(b, sizeof(b), a) == 0 ? b : NULL)
#endif

/*
 * port.c
 */
int64_t random_number(int64_t);
int64_t secure_random_number(int64_t);
time_t get_current_time(void);
void get_usec_clock(long *, long *);
long get_cpu_times(unsigned long *, unsigned long *);
char *get_current_dir(char *, int);

#endif
