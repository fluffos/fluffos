#ifndef MAIN_H
#define MAIN_H

#include "std.h"

/*
 * main.c
 */
#ifdef DEBUG
extern int d_flag;
#endif
extern int t_flag;
extern int comp_flag;
extern int time_to_swap;
extern int time_to_clean_up;
extern char *default_fail_message;
extern int port_number;
extern int boot_time;
extern int max_cost;
extern int max_array_size;
extern int max_buffer_size;
extern int max_string_length;
extern char *master_file_name;
extern char *reserved_area;
extern struct svalue const0;
extern struct svalue const1;
extern struct svalue const0u;
extern struct svalue const0n;
extern int st_num_arg;
extern double consts[];
extern int slow_shut_down_to_do;
extern struct object *master_ob;

void debug_message PROTVARGS(());

#ifdef TRAP_CRASHES
void crash_MudOS PROT((char *));
#endif

#endif
