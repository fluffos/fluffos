/* mapping.h - 1992/07/19 */

#ifdef PACKAGE_MUDLIB_STATS
#include "packages/mudlib_stats/mudlib_stats.h"
#endif

#ifndef _MAPPING_H
#define _MAPPING_H

// TODO: remove this.
#define MAX_MAPPING_SIZE CONFIG_INT(__MAX_MAPPING_SIZE__)

#define MAP_SVAL_HASH(x) sval_hash(x)

//#define MAP_SVAL_HASH(x) (((POINTER_INT)((x).u.number)) >> 5)
LPC_INT sval_hash(svalue_t);

typedef struct mapping_node_s {
  struct mapping_node_s *next;
  struct svalue_t values[2];
} mapping_node_t;

#define MNB_SIZE 256

typedef struct mapping_node_block_s {
  struct mapping_node_block_s *next;
  mapping_node_t nodes[MNB_SIZE];
} mapping_node_block_t;

#define MAP_HASH_TABLE_SIZE 8 /* must be a power of 2 */
#define FILL_PERCENT 80       /* must not be larger than 99 */

#define MAPSIZE(size) sizeof(mapping_t)

#define MAP_LOCKED 0x80000000
#define MAP_COUNT(m) ((m)->count & ~MAP_LOCKED)

struct mapping_t {
  unsigned short ref; /* how many times this map has been
                       * referenced */
#ifdef DEBUGMALLOC_EXTENSIONS
  int extra_ref;
#endif
  mapping_node_t **table;  /* the hash table */
  unsigned int table_size; /* # of buckets in hash table == power of 2 */
  unsigned int unfilled;   /* # of buckets among 80% of total buckets that do not
                              have
                              entries */
  unsigned int count;      /* total # of nodes actually in mapping  */
#ifdef PACKAGE_MUDLIB_STATS
  struct statgroup_t stats; /* creators of the mapping */
#endif
};

typedef struct finfo_s {
  char *func;
  struct object_t *obj;
  struct svalue_t *extra;
  struct funptr_t *fp;
} finfo_t;

typedef struct vinfo_s {
  array_t *v;
  int pos, size, w;
} vinfo_t;

typedef struct minfo_s {
  mapping_t *map, *newmap;
} minfo_t;

#define mapping_too_large() \
  error("Mapping exceeded maximum allowed size of %d.\n", MAX_MAPPING_SIZE);

/*
 * mapping.c
 */
extern mapping_node_t *locked_map_nodes;

int msameval(svalue_t *, svalue_t *);
int mapping_save_size(mapping_t *);
mapping_t *mapTraverse(mapping_t *, int (*)(mapping_t *, mapping_node_t *, void *), void *);
mapping_t *load_mapping_from_aggregate(svalue_t *, int);
mapping_t *allocate_mapping(int);
mapping_t *allocate_mapping2(array_t *, svalue_t *);
void free_mapping(mapping_t *);
svalue_t *find_in_mapping(mapping_t *, svalue_t *);
svalue_t *find_string_in_mapping(mapping_t *, const char *);
svalue_t *find_for_insert(mapping_t *, svalue_t *, int);
void absorb_mapping(mapping_t *, mapping_t *);
void mapping_delete(mapping_t *, svalue_t *);
mapping_t *add_mapping(mapping_t *, mapping_t *);
mapping_node_t *new_map_node(void);
int restore_hash_string(char **str, svalue_t *);
int growMap(mapping_t *);
void free_node(mapping_t *, mapping_node_t *);
void unlock_mapping(mapping_t *);
void map_mapping(svalue_t *, int);
void filter_mapping(svalue_t *, int);
mapping_t *compose_mapping(mapping_t *, mapping_t *, unsigned short);
array_t *mapping_indices(mapping_t *);
array_t *mapping_values(mapping_t *);
array_t *mapping_each(mapping_t *);
char *save_mapping(mapping_t *);
void dealloc_mapping(mapping_t *);
void mark_mapping_node_blocks(void);
mapping_t *mkmapping(array_t *, array_t *);
LPC_INT svalue_to_int(svalue_t *);
void add_mapping_pair(mapping_t *, const char *, long);
void add_mapping_string(mapping_t *, const char *, const char *);
void add_mapping_malloced_string(mapping_t *, const char *, char *);
void add_mapping_object(mapping_t *, const char *, object_t *);
void add_mapping_array(mapping_t *, const char *, array_t *);
void add_mapping_shared_string(mapping_t *, const char *, char *);

#endif /* _MAPPING_H */
