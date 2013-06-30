/*
 * console.h -- definitions and prototypes for console.c
 *
 */

#ifndef CONSOLE_H
#define CONSOLE_H

#include "options_incl.h"

#ifdef HAS_CONSOLE
void on_console_input();
void console_command(char *);
#endif

#endif

