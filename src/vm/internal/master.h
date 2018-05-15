#ifndef MASTER_H
#define MASTER_H

/* for apply_master_ob */
#define MASTER_APPROVED(x) \
  (((x) == (svalue_t *)-1) || ((x) && (((x)->type != T_NUMBER) || (x)->u.number)))

extern struct object_t *master_ob;

// Initialize master object.
void init_master(const char *);

// @Deprecated : should use safe_apply_master_ob
struct svalue_t *apply_master_ob(int, int);
struct svalue_t *safe_apply_master_ob(int, int);
void set_master(struct object_t *);

#endif
