#ifndef MASTER_H
#define MASTER_H

/* Did a master approval apply say yes?
 *
 * `v` is an apply_master_ob()/safe_apply_master_ob() result: the sentinel
 * (svalue_t *)-1 when no master has loaded yet (the deliberate bootstrap
 * "allow" -- see apply_master_ob()), null when the apply does not exist or
 * errored, otherwise the returned value. Anything that is not the integer 0
 * approves; `apply_name` names the apply in the denial message.
 *
 * A T_PROMISE is DENIED. An async function hands back a promise the instant
 * its body parks, before it has decided anything, and "not the integer 0"
 * would read that as a grant -- across every gate that goes through here:
 * valid_seteuid, valid_bind, valid_shadow, valid_socket, valid_object,
 * valid_link, valid_hide, valid_override. Same rule, and the same reason, as
 * check_valid_path() and the command parser (AGENTS.md section 13.24). */
int master_approved(struct svalue_t* v, const char* apply_name);

/* for apply_master_ob */
#define MASTER_APPROVED(x, name) master_approved((x), (name))

extern struct object_t* master_ob;

// Initialize master object.
void init_master(const char*);

// @Deprecated : should use safe_apply_master_ob
struct svalue_t* apply_master_ob(int, int);
struct svalue_t* safe_apply_master_ob(int, int);
void set_master(struct object_t*);
void rebuild_master_applies();

#endif
