/*
 * reclaim.c
 * loops through all variables in all objects looking for the possibility
 * of freeing up destructed objects (that are still hanging around because
 * of references) -- coded by Blackthorn@Genocide Feb. 1993
 */

#include "base/package_api.h"

#include "packages/core/reclaim.h"

#include <functional>
#include <vector>

#include "packages/core/call_out.h"

enum { MAX_RECURSION = 25 };

static void gc_mapping(mapping_t* /*m*/);
static void check_svalue(svalue_t* /*v*/);

static int cleaned, nested;

/* Promises currently on the walk path. check_svalue() is an unmemoized tree
 * walk bounded only by MAX_RECURSION, so a value reachable K times from
 * itself costs K^25 visits. A parked coroutine makes that trivially
 * reachable: `mixed q = p; await p;` puts the awaited promise in two frame
 * slots, and promise -> reaction -> coroutine frame -> same promise is a
 * one-hop loop -- K=3 never finished in 60s. Walking each promise at most
 * once per path removes the blowup at its source. */
static std::vector<promise_t*> walking_promises;

static void check_svalue(svalue_t* v) {
  int idx;

  nested++;
  if (nested > MAX_RECURSION) {
    nested--;  // keep nested balanced, or reclaim stays disabled for the pass
    return;
  }
  switch (v->type) {
    case T_OBJECT:
      if (v->u.ob->flags & O_DESTRUCTED) {
        free_svalue(v, "reclaim_objects");
        *v = const0u;
        cleaned++;
      }
      break;
    case T_MAPPING:
      gc_mapping(v->u.map);
      break;
    case T_ARRAY:
    case T_CLASS:
      for (idx = 0; idx < v->u.arr->size; idx++) {
        check_svalue(&v->u.arr->item[idx]);
      }
      break;
    case T_FUNCTION: {
      svalue_t tmp;

      if (v->u.fp->hdr.owner && (v->u.fp->hdr.owner->flags & O_DESTRUCTED)) {
        // Only release the owner reference; the funptr itself lives on (it is
        // still held by this variable) and dealloc_funp() will decrement the
        // program's func_ref against the funptr's stored creation program when
        // it is finally freed. Decrementing func_ref here too was a double
        // decrement that underflowed func_ref and leaked the program (and
        // could deallocate a program the funptr still referenced). FP_FUNCTIONAL
        // funptrs, which take no special-case here, are already handled
        // correctly this way.
        free_object(&v->u.fp->hdr.owner, "reclaim_objects");
        v->u.fp->hdr.owner = nullptr;
        cleaned++;
      }

      tmp.type = T_ARRAY;
      if ((tmp.u.arr = v->u.fp->hdr.args)) {
        check_svalue(&tmp);
      }
      break;
    }
    case T_PROMISE: {
      promise_t* prom = v->u.prom;
      /* see walking_promises: without this, an ordinary parked `await p`
       * makes the walk exponential */
      for (promise_t* seen : walking_promises) {
        if (seen == prom) {
          nested--;
          return;
        }
      }
      walking_promises.push_back(prom);
      DEFER { walking_promises.pop_back(); };
      check_svalue(&prom->result);
      if (prom->reactions) {
        for (auto& r : *prom->reactions) {
          svalue_t tmp;
          tmp.type = T_FUNCTION;
          if ((tmp.u.fp = r.on_fulfilled)) {
            check_svalue(&tmp);
          }
          if ((tmp.u.fp = r.on_rejected)) {
            check_svalue(&tmp);
          }
          if (r.command_giver && (r.command_giver->flags & O_DESTRUCTED)) {
            free_object(&r.command_giver, "reclaim_objects");
            r.command_giver = nullptr;
            cleaned++;
          }
          if (r.next) {
            svalue_t tmp2;
            tmp2.type = T_PROMISE;
            tmp2.u.prom = r.next;
            check_svalue(&tmp2);
          }
          if (r.coro) {
            /* A PARKED COROUTINE holds svalues too -- its saved frame slice
             * and its defer lists (the async frame's and every acatch
             * marker's). Objects destructed while a frame is suspended are
             * only reclaimable through here; every sibling walker
             * (mark_coroutine, checkmemory's orphan scan, cycles.cc) already
             * covers these. coro->ob/prev_ob/command_giver are deliberately
             * NOT nulled: resume_coroutine() reads ob to decide the frame is
             * stale, and free_coroutine() frees all three. */
            {
              /* the coroutine's own result promise: often its ONLY ref, so
               * nothing else here reaches what that promise's reactions
               * hold. The three sibling walkers (mark_coroutine,
               * checkmemory's orphan scan, cycles.cc) all cover it. */
              svalue_t tmp3;
              tmp3.type = T_PROMISE;
              tmp3.u.prom = r.coro->result_promise;
              check_svalue(&tmp3);
            }
            for (int fi = 0; fi < r.coro->frame_size; fi++) {
              check_svalue(&r.coro->frame[fi]);
            }
            for (struct defer_list* d = r.coro->defers; d; d = d->next) {
              check_svalue(&d->func);
              check_svalue(&d->tp);
            }
            for (auto& mk : r.coro->markers) {
              for (struct defer_list* d = mk.defers; d; d = d->next) {
                check_svalue(&d->func);
                check_svalue(&d->tp);
              }
            }
          }
        }
      }
      break;
    }
  }
  nested--;
}

static void gc_mapping(mapping_t* m) {
  /* Be careful to correctly handle destructed mapping keys.  We can't
   * just call check_svalue() b/c the hash would be wrong and the '0'
   * element we add would be unreferenceable (in most cases)
   */
  mapping_node_t **prev, *elt;
  int j = m->table_size;

  do {
    prev = m->table + j;
    while ((elt = *prev)) {
      if (elt->values[0].type == T_OBJECT) {
        if (elt->values[0].u.ob->flags & O_DESTRUCTED) {
          free_object(&elt->values[0].u.ob, "gc_mapping");
          elt->values[0].u.ob = nullptr;
          /* found one, do a map_delete() */
          if (!(*prev = elt->next) && !m->table[j]) {
            m->unfilled++;
          }
          cleaned++;
          m->count--;
          total_mapping_nodes--;
          total_mapping_size -= sizeof(mapping_node_t);
          free_node(m, elt);
          continue;
        }
      } else {
        /* in case the key is a mapping or something */
        check_svalue(elt->values);
      }
      check_svalue(elt->values + 1);
      prev = &(elt->next);
    }
  } while (j--);
}

int reclaim_objects(bool is_auto) {
  if (is_auto) {
    add_gametick_event(time_to_next_gametick(std::chrono::seconds(30 + random_number(30))),
                       TickEvent::callback_type([] { return reclaim_objects(true); }));
  }
  int i;
  object_t* ob;

  reclaim_call_outs();

  cleaned = nested = 0;
  for (ob = obj_list; ob; ob = ob->next_all) {
    if (ob->prog) {
      for (i = 0; i < ob->prog->num_variables_total; i++) {
        check_svalue(&ob->variables[i]);
      }
    }
  }

  return cleaned;
}
