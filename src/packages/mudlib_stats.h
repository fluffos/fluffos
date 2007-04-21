/*
 * mudlib_stats.h
 * created by: Erik Kay
 * last modified: 11/1/92
 * this file is a replacement for wiz_list.h and all of its purposes
 * the idea is that it will be more domain based, rather than user based
 * and will be a little more general purpose than wiz_list was
 */

#if !defined(MUDLIB_STATS_H) && defined(PACKAGE_MUDLIB_STATS)
#define MUDLIB_STATS_H

typedef struct _mstats {
    /* the name of the stat group */
    char *name;
    int length;
    /* the next data element in the linked list */
    struct _mstats *next;
    /* statistics */
    int moves;
    int heart_beats;
    int size_array;
    int errors;
    int objects;
}       mudlib_stats_t;


typedef struct _statgroup {
    mudlib_stats_t *domain;
    mudlib_stats_t *author;
}          statgroup_t;

#define DOMAIN_STATS_FILE_NAME "domain_stats"
#define AUTHOR_STATS_FILE_NAME "author_stats"

void assign_stats (statgroup_t *, struct object_s *);
void null_stats (statgroup_t *);
void init_stats_for_object (struct object_s *);

void add_moves (statgroup_t *, int);
INLINE void add_heart_beats (statgroup_t *, int);
void add_array_size (statgroup_t *, int);
void add_errors (statgroup_t *, int);
void add_errors_for_file (char *, int);
void add_objects (statgroup_t *, int);
struct mapping_s *get_domain_stats (char *);
struct mapping_s *get_author_stats (char *);
void mudlib_stats_decay (void);
void save_stat_files (void);
void restore_stat_files (void);
void set_author (char *);
mudlib_stats_t *set_master_author (char *);
mudlib_stats_t *set_backbone_domain (char *);
int check_valid_stat_entry (mudlib_stats_t *);
#ifdef DEBUGMALLOC_EXTENSIONS
void mark_mudlib_stats (void);
#endif

#endif
