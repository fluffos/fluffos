/*
 * console.h -- definitions and prototypes for console.c
 *
 */

#ifndef CONSOLE_H
#define CONSOLE_H

#include "options_incl.h"

void console_init(struct event_base *);
void on_console_input();
void console_command(char *);

#endif
