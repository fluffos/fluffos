#ifndef OTABLE_H
#define OTABLE_H

/*
 * otable.c
 */
void init_otable(void);
void enter_object_hash(struct object_t *);
void remove_object_hash(struct object_t *);
void remove_precompiled_hashes(char *);
struct object_t *lookup_object_hash(const char *);
int show_otable_status(struct outbuffer_t *, int);
struct array_t *find_ch_n(const char *s);

#endif
