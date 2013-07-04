#ifndef PORT_H
#define PORT_H

/*
 * port.c
 */
#ifndef _FUNC_SPEC_
int64_t random_number(int64_t);
long get_current_time(void);
const char *time_string(time_t);
void get_usec_clock(long *, long *);
long get_cpu_times(unsigned long *, unsigned long *);
char *get_current_dir(char *, int);
#endif

#endif
