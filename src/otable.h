#ifndef OTABLE_H
#define OTABLE_H

/*
 * otable.c
 */
void init_otable PROT((void));
void enter_object_hash PROT((struct object *));
void remove_object_hash PROT((struct object *));
struct object *lookup_object_hash PROT((char *));
int show_otable_status PROT((int));

#endif
