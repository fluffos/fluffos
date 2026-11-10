#ifndef RECLAIM_H
#define RECLAIM_H

#include <chrono>

struct object_t;

/*
 * reclaim.c
 */
int reclaim_objects(bool);

/* The incremental automatic pass ('reclaim slice budget' > 0). The automatic
 * reclaim drives these itself; they are exposed for the unit tests.
 *
 * reclaim_pass_start() begins a pass (no-op while one is running);
 * reclaim_pass_step() walks objects until `budget` has elapsed, always at
 * least one, and returns true once the pass has walked the whole object list.
 * reclaim_pass_cleaned() is the running pass's count of the references it
 * cleared, the number reclaim_objects() returns for a whole walk. */
void reclaim_pass_start();
bool reclaim_pass_step(std::chrono::microseconds budget);
bool reclaim_pass_running();
int reclaim_pass_cleaned();

/* destruct_object() calls this before it unlinks `ob` from obj_list, so an
 * incremental pass never resumes from an object that has left the list. */
void reclaim_object_unlinked(object_t* ob);

#endif
