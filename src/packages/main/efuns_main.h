#ifndef EFUNS_MAIN_H
#define EFUNS_MAIN_H

/* These really should be moved somewhere else */
/*
 * efuns_main.c
 */
extern int call_origin;

int inherits(struct program_t *, struct program_t *);
void add_mapping_pair(struct mapping_t *, const char *, long);
void add_mapping_string(struct mapping_t *, const char *, const char *);
void add_mapping_object(struct mapping_t *, const char *, object_t *);
void add_mapping_array(struct mapping_t *, const char *, array_t *);
void add_mapping_shared_string(struct mapping_t *, const char *, char *);

#endif
