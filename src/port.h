#ifndef PORT_H
#define PORT_H

/*
 * port.c
 */
#ifndef _FUNC_SPEC_
long random_number (long);
long get_current_time (void);
char *time_string (time_t);
void init_usec_clock (void);
void get_usec_clock (long *, long *);
int get_cpu_times (unsigned long *, unsigned long *);
char *get_current_dir (char *, int);
#ifdef DRAND48
double drand48 (void);
#endif
#ifndef HAS_STRERROR
char *port_strerror (int);
#endif
#endif

#endif
