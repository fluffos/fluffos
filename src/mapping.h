/* mapping.h - 1992/07/19 */

#ifndef _MAPPING_H
#define _MAPPING_H

struct node {
    svalue values[2];
    struct node *next;
    unsigned short hashval;
};

#define MAX_TABLE_SIZE 32768
#define MAP_HASH_TABLE_SIZE 8	/* must be a power of 2 */
#define FILL_PERCENT 80		/* must not be larger than 99 */

#define MAPSIZE(size) sizeof(struct mapping)

struct mapping {
    unsigned short ref;		/* how many times this map has been
				 * referenced */
#ifdef DEBUG
    int extra_ref;
#endif
    struct node **table;	/* the hash table */
    unsigned short table_size;	/* # of buckets in hash table == power of 2 */
    unsigned short unfilled;	/* # of buckets among 80% of total buckets that do not have entries */
    int count;			/* total # of nodes actually in mapping  */
#ifndef NO_MUDLIB_STATS
    statgroup_t stats;		/* creators of the mapping */
#endif
#ifdef EACH
    struct object *eachObj;	/* object that last called each() on this map */
    struct node *elt;		/* keeps track of where each() is in the map */
    unsigned short bucket;	/* keeps track of where each() currently is */
#endif
};

typedef struct finfo_s {
    char *func;
    struct object *obj;
    struct svalue *extra;
    struct funp *fp;
}       finfo_t;

typedef struct vinfo_s {
    struct vector *v;
    int pos, size, w;
}       vinfo_t;

typedef struct minfo_s {
    struct mapping *map, *newmap;
}       minfo_t;

#define mapping_too_large() \
    error("Mapping exceeded maximum allowed size of %d.\n",MAX_MAPPING_SIZE);

#ifndef max
#define max(x,y) ((x) > (y)) ? (x) : (y)
#endif

/* used by mapHashStr: do not make larger than the size of the coeff array */
#define MAX_KEY_LEN 25

#endif				/* _MAPPING_H */
