#ifndef MAIN_H
#define MAIN_H

#include "lpc_incl.h"

/*
 * main.c
 */

#define PORT_TELNET      1
#define PORT_BINARY      2
#define PORT_ASCII       3

typedef struct {
    int kind;
    int port;
    int fd;
} port_def_t;

extern port_def_t external_port[5];
#ifdef PACKAGE_EXTERNAL
extern char *external_cmd[5];
#endif

extern int t_flag;
extern int comp_flag;
extern int time_to_swap;
extern int time_to_clean_up;
extern char *default_fail_message;
extern int boot_time;
extern int max_cost;
extern int max_array_size;
extern int max_buffer_size;
extern int max_string_length;
extern char *reserved_area;
extern double consts[];
extern int slow_shut_down_to_do;

void CDECL debug_message PROT1V(char *);

#endif
