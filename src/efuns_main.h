#ifndef EFUNS_MAIN_H
#define EFUNS_MAIN_H

/*
 * efuns_main.c
 */
extern int using_bsd_malloc;
extern int using_smalloc;
extern int call_origin;

void print_cache_stats PROT((void));
int inherits PROT((struct program *, struct program *));
void add_mapping_pair PROT((struct mapping *, char *, int));
void add_mapping_string PROT((struct mapping *, char *, char *));
void add_mapping_object PROT((struct mapping *, char *, struct object *));
void add_mapping_array PROT((struct mapping *, char *, struct vector *));
void add_mapping_shared_string PROT((struct mapping *, char *, char *));

#endif
