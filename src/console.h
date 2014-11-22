/*
 * console.h -- definitions and prototypes for console.c
 *
 */

#ifndef CONSOLE_H
#define CONSOLE_H

extern int has_console;
extern void restore_sigttin(void);

void console_init(struct event_base *);
void on_console_input();

#endif
