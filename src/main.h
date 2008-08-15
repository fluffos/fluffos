#ifndef MAIN_H
#define MAIN_H

#include "lpc_incl.h"
#include <time.h>
/*
 * main.c
 */

#define PORT_UNDEFINED   0
#define PORT_TELNET      1
#define PORT_BINARY      2
#define PORT_ASCII       3
#define PORT_MUD         4

typedef struct {
    int kind;
    int port;
    int fd;
#ifdef F_NETWORK_STATS
    int in_packets;
    int in_volume;
    int out_packets;
    int out_volume;
#endif
} port_def_t;

extern port_def_t external_port[5];
#ifdef PACKAGE_EXTERNAL
extern char *external_cmd[NUM_EXTERNAL_CMDS];
#endif

extern int t_flag;
extern int comp_flag;
extern int time_to_clean_up;
extern const char *default_fail_message;
extern time_t boot_time;
extern int max_cost;
extern int max_array_size;
extern int max_buffer_size;
extern int max_string_length;
extern char *reserved_area;
extern double consts[];
extern int slow_shut_down_to_do;

void CDECL debug_message(const char *, ...);

#endif
