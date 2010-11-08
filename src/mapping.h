/* mapping.h - 1992/07/19 */

/* It is usually better to include "lpc_incl.h" instead of including this
   directly */

#ifndef _MAPPING_H
#define _MAPPING_H

#define MAP_SVAL_HASH(x) sval_hash(x)
#include "hash.h"
#include "stralloc.h"
//#define MAP_SVAL_HASH(x) (((POINTER_INT)((x).u.number)) >> 5)
static unsigned long sval_hash(svalue_t x){
    switch(x.type)
    {
        case T_STRING:
            return HASH(BLOCK(x.u.string));
        case T_NUMBER:
            return (unsigned long)x.u.number;
        case T_OBJECT:
            //return HASH(BLOCK(x.u.ob->obname));
        default:
            return (unsigned long)(((POINTER_INT)((x).u.number)) >> 5);
    }
}

typedef struct mapping_node_s {
    struct mapping_node_s *next;
    svalue_t values[2];
} mapping_node_t;

#define MNB_SIZE 256

typedef struct mapping_node_block_s {
    struct mapping_node_block_s *next;
    mapping_node_t nodes[MNB_SIZE];
} mapping_node_block_t;

#define MAP_HASH_TABLE_SIZE 8   /* must be a power of 2 */
#define FILL_PERCENT 80         /* must not be larger than 99 */

#define MAPSIZE(size) sizeof(mapping_t)

#define MAP_LOCKED 0x80000000
#define MAP_COUNT(m) ((m)->count & ~MAP_LOCKED)

typedef struct mapping_s {
    unsigned short ref;         /* how many times this map has been
                                 * referenced */
#ifdef DEBUG
    int extra_ref;
#endif
    mapping_node_t **table;     /* the hash table */
    unsigned int table_size;  /* # of buckets in hash table == power of 2 */
    unsigned int unfilled;    /* # of buckets among 80% of total buckets that do not have entries */
    unsigned int count;         /* total # of nodes actually in mapping  */
#ifdef PACKAGE_MUDLIB_STATS
    statgroup_t stats;          /* creators of the mapping */
#endif
} mapping_t;

typedef struct finfo_s {
    char *func;
    object_t *obj;
    svalue_t *extra;
    funptr_t *fp;
}       finfo_t;

typedef struct vinfo_s {
    array_t *v;
    int pos, size, w;
}       vinfo_t;

typedef struct minfo_s {
    mapping_t *map, *newmap;
}       minfo_t;

#define mapping_too_large() \
    error("Mapping exceeded maximum allowed size of %d.\n",MAX_MAPPING_SIZE);

#ifndef max
#define max(x,y) ((x) > (y)) ? (x) : (y)
#endif

/*
 * mapping.c
 */
extern int num_mappings;
extern int total_mapping_size;
extern int total_mapping_nodes;
extern mapping_node_t *locked_map_nodes;

int msameval (svalue_t *, svalue_t *);
int mapping_save_size (mapping_t *);
INLINE mapping_t *mapTraverse (mapping_t *, int (*) (mapping_t *, mapping_node_t *, void *), void *);
INLINE mapping_t *load_mapping_from_aggregate (svalue_t *, int);
INLINE mapping_t *allocate_mapping (int);
INLINE mapping_t *allocate_mapping2 (array_t *, svalue_t *);
INLINE void free_mapping (mapping_t *);
INLINE svalue_t *find_in_mapping (mapping_t *, svalue_t *);
svalue_t *find_string_in_mapping (mapping_t *, const char *);
INLINE svalue_t *find_for_insert (mapping_t *, svalue_t *, int);
INLINE void absorb_mapping (mapping_t *, mapping_t *);
INLINE void mapping_delete (mapping_t *, svalue_t *);
INLINE mapping_t *add_mapping (mapping_t *, mapping_t *);
mapping_node_t *new_map_node (void);
int restore_hash_string (char **str, svalue_t *);
int growMap (mapping_t *);
void free_node (mapping_t *, mapping_node_t *);
void unlock_mapping (mapping_t *);
void map_mapping (svalue_t *, int);
void filter_mapping (svalue_t *, int);
INLINE mapping_t *compose_mapping (mapping_t *, mapping_t *, unsigned short);
array_t *mapping_indices (mapping_t *);
array_t *mapping_values (mapping_t *);
array_t *mapping_each (mapping_t *);
char *save_mapping (mapping_t *);
void dealloc_mapping (mapping_t *);
void mark_mapping_node_blocks (void);
mapping_t *mkmapping (array_t *, array_t *);
int svalue_to_int (svalue_t *);
void add_mapping_pair (mapping_t *, const char *, long);
void add_mapping_string (mapping_t *, const char *, const char *);
void add_mapping_malloced_string (mapping_t *, const char *, char *);
void add_mapping_object (mapping_t *, const char *, object_t *);
void add_mapping_array (mapping_t *, const char *, array_t *);
void add_mapping_shared_string (mapping_t *, const char *, char *);

#endif                          /* _MAPPING_H */
