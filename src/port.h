#ifndef PORT_H
#define PORT_H

/*
 * port.c
 */
#ifndef _FUNC_SPEC_
int random_number PROT((int));
int get_current_time PROT((void));
char *time_string PROT((int));
void init_usec_clock PROT((void));
void get_usec_clock PROT((long *, long *));
int get_cpu_times PROT((unsigned long *, unsigned long *));
char *get_current_dir PROT((char *, int));
#ifdef DRAND48
double drand48 PROT((void));
#endif
#ifdef WIN32
char *crypt(char *, char *);
#endif
#ifndef HAS_STRERROR
char *port_strerror PROT((int));
#endif
#endif

#endif
