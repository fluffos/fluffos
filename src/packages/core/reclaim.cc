/*
 * reclaim.c
 * loops through all variables in all objects looking for the possibility
 * of freeing up destructed objects (that are still hanging around because
 * of references) -- coded by Blackthorn@Genocide Feb. 1993
 */

#include "base/package_api.h"

#include "packages/core/reclaim.h"

#include <functional>

#include "packages/core/call_out.h"

#define MAX_RECURSION 25

static void gc_mapping(mapping_t * /*m*/);
static void check_svalue(svalue_t * /*v*/);

static int cleaned, nested;

static void check_svalue(svalue_t *v) {
  int idx;

  nested++;
  if (nested > MAX_RECURSION) {
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
      program_t *prog;

      if (v->u.fp->hdr.owner && (v->u.fp->hdr.owner->flags & O_DESTRUCTED)) {
        if (v->u.fp->hdr.type == (FP_LOCAL | FP_NOT_BINDABLE)) {
          prog = v->u.fp->hdr.owner->prog;
          prog->func_ref--;
          debug(d_flag, "subtr func ref /%s: now %i\n", prog->filename, prog->func_ref);
          if (!prog->ref && !prog->func_ref) {
            deallocate_program(prog);
          }
        }
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
  }
  nested--;
  return;
}

static void gc_mapping(mapping_t *m) {
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
    add_gametick_event(std::chrono::seconds(30 + random_number(30)),
                       tick_event::callback_type([] { return reclaim_objects(true); }));
  }
  int i;
  object_t *ob;

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
