#include "std.h"
#include "call_out.h"
#include "backend.h"
#include "comm.h"
#include "port.h"
#include "eoperators.h"
#include "sprintf.h"
#include "eval.h"

#define DBG_CALLOUT(...) debug(call_out, __VA_ARGS__)

/*
 * This file implements delayed calls of functions.
 * Static functions can not be called this way.
 *
 * Allocate the structures several in one chunk, to get rid of malloc
 * overhead.
 */

#define CHUNK_SIZE  20

typedef struct pending_call_s {
  int delta;
  union string_or_func function;
  object_t *ob;
  array_t *vs;
  struct pending_call_s *next;
#ifdef THIS_PLAYER_IN_CALL_OUT
  object_t *command_giver;
#endif
#ifdef CALLOUT_HANDLES
  LPC_INT handle;
#endif
} pending_call_t;

static pending_call_t *call_list[CALLOUT_CYCLE_SIZE];
static pending_call_t *call_list_free;
static int num_call;
#ifdef CALLOUT_HANDLES
static int unique = 0;
#endif

static void free_call(pending_call_t *);
static void free_called_call(pending_call_t *);
void remove_all_call_out(object_t *);

/*
 * Free a call out structure.
 */
static void free_called_call(pending_call_t *cop)
{
  cop->next = call_list_free;
  if (cop->ob) {
    free_string(cop->function.s);
    free_object(&cop->ob, "free_call");
  } else {
    free_funp(cop->function.f);
  }
  cop->function.s = 0;
#ifdef THIS_PLAYER_IN_CALL_OUT
  if (cop->command_giver) {
    free_object(&cop->command_giver, "free_call");
    cop->command_giver = 0;
  }
#endif
  cop->ob = 0;
  call_list_free = cop;
}

static void free_call(pending_call_t *cop)
{
  if (cop->vs) {
    free_array(cop->vs);
  }
  free_called_call(cop);
}

/*
 * Setup a new call out.
 */
#ifdef CALLOUT_HANDLES
LPC_INT
#else
void
#endif
new_call_out(object_t *ob, svalue_t *fun, int delay,
             int num_args, svalue_t *arg)
{
  pending_call_t *cop, **copp;
  int tm;

  if (delay < 0) {
    delay = 0;
  }

  DBG_CALLOUT("new_call_out: /%s delay %i\n", ob->obname, delay);

  if (!call_list_free) {
    int i;

    call_list_free = CALLOCATE(CHUNK_SIZE, pending_call_t,
                               TAG_CALL_OUT, "new_call_out: call_list_free");
    for (i = 0; i < CHUNK_SIZE - 1; i++) {
      call_list_free[i].next = &call_list_free[i + 1];
    }
    call_list_free[CHUNK_SIZE - 1].next = 0;
    num_call += CHUNK_SIZE;
  }
  cop = call_list_free;
  call_list_free = call_list_free->next;

  if (fun->type == T_STRING) {
    DBG_CALLOUT("  function: %s\n", fun->u.string);
    cop->function.s = make_shared_string(fun->u.string);
    cop->ob = ob;
    add_ref(ob, "call_out");
  } else {
    DBG_CALLOUT("  function: <function>\n");
    cop->function.f = fun->u.fp;
    fun->u.fp->hdr.ref++;
    cop->ob = 0;
  }
#ifdef THIS_PLAYER_IN_CALL_OUT
  cop->command_giver = command_giver; /* save current user context */
  if (command_giver) {
    add_ref(command_giver, "new_call_out");    /* Bump its ref */
  }
#endif
  if (num_args > 0) {
    cop->vs = allocate_empty_array(num_args);
    memcpy(cop->vs->item, arg, sizeof(svalue_t) * num_args);
  } else {
    cop->vs = 0;
  }

  /* Find out which slot this one fits in */
  tm = (delay + current_time) & (CALLOUT_CYCLE_SIZE - 1);
  /* number of cycles */
  delay = delay / CALLOUT_CYCLE_SIZE;

  DBG_CALLOUT("Current time: %ld  Executes at: %ld  Slot: %d  Delay: %d\n",
              current_time, current_time + delay, tm, delay);

  for (copp = &call_list[tm]; *copp; copp = &(*copp)->next) {
    if ((*copp)->delta > delay) {
      (*copp)->delta -= delay;
      cop->delta = delay;
      cop->next = *copp;
      *copp = cop;
#ifdef CALLOUT_HANDLES
      tm += CALLOUT_CYCLE_SIZE * ++unique;
      cop->handle = tm;
      return tm;
#else
      return;
#endif
    }
    delay -= (*copp)->delta;
  }
  *copp = cop;
  cop->delta = delay;
  cop->next = 0;
#ifdef CALLOUT_HANDLES
  tm += CALLOUT_CYCLE_SIZE * ++unique;
  cop->handle = tm;
  return tm;
#endif
}

/*
 * See if there are any call outs to be called. Set the 'command_giver'
 * if it is a living object. Check for shadowing objects, which may also
 * be living objects.
 */
void call_out()
{
  long extra, real_time;
  static pending_call_t *cop = 0;
  error_context_t econ;
  volatile int tm;
#ifdef CALLOUT_LOOP_PROTECTION
  /* The number of call_outs that will be given a full max_cost time budget;
   * any calls beyond this number will get only what time is left over.
   */
  int num_max_cost_calls;
  int warned_about_recursion = 0;
#endif

  current_interactive = 0;

  /* could be still allocated if an error occured during a call_out */
  if (cop) {
    free_called_call(cop);
    cop = 0;
  }

  real_time = get_current_time();
  DBG_CALLOUT("Calling call_outs: current_time: %ld real_time: %ld difference: %ld\n",
              current_time, real_time, real_time - current_time);

  /* Slowly advance the clock forward towards real_time, doing call_outs
   * as we go.
   */
  save_context(&econ);
  while (1) {

    tm = current_time & (CALLOUT_CYCLE_SIZE - 1);
    DBG_CALLOUT("   slot %i\n", tm);

#ifdef CALLOUT_LOOP_PROTECTION
    /* Count how many call_outs are ready to run.  These each will
     * be given their own max_cost time budget.  If any immediate
     * call_outs are subsequently added to the list, they will all
     * run within a single, final max_cost time allocation.  This
     * prevents infinite call_out("xyz", 0) loops from wedging the
     * driver.
     */
    num_max_cost_calls = 1;
    for (cop = call_list[tm]; cop && cop->delta == 0; cop = cop->next) {
      num_max_cost_calls++;
    }
    cop = 0;
#endif

    while (call_list[tm] && call_list[tm]->delta == 0) {
      object_t *ob, *new_command_giver;

#ifdef CALLOUT_LOOP_PROTECTION
      /* Count this call_out against the allowed number of full time budget calls */
      num_max_cost_calls--;
#endif

      /*
       * Move the first call_out out of the chain.
       */
      cop = call_list[tm];
      call_list[tm] = call_list[tm]->next;
      ob = (cop->ob ? cop->ob : cop->function.f->hdr.owner);

      DBG_CALLOUT("      /%s\n", (ob ? ob->obname : "(null)"));

      if (!ob || (ob->flags & O_DESTRUCTED)) {
        DBG_CALLOUT("         (destructed)\n");
        free_call(cop);
        cop = 0;
      } else {
        try {
          object_t *ob;

          ob = cop->ob;
#ifndef NO_SHADOWS
          if (ob)
            while (ob->shadowing) {
              ob = ob->shadowing;
            }
#endif
          new_command_giver = 0;
#ifdef THIS_PLAYER_IN_CALL_OUT
          if (cop->command_giver &&
              !(cop->command_giver->flags & O_DESTRUCTED)) {
            new_command_giver = cop->command_giver;
          } else if (ob && (ob->flags & O_LISTENER)) {
            new_command_giver = ob;
          }
          if (new_command_giver) {
            DBG_CALLOUT("         command_giver: /%s\n", new_command_giver->obname);
          }
#endif
          save_command_giver(new_command_giver);
          /* current object no longer set */

          if (cop->vs) {
            array_t *vec = cop->vs;
            svalue_t *svp = vec->item + vec->size;

            while (svp-- > vec->item) {
              if (svp->type == T_OBJECT &&
                  (svp->u.ob->flags & O_DESTRUCTED)) {
                free_object(&svp->u.ob, "call_out");
                *svp = const0u;
              }
            }
            /* cop->vs is ref one */
            extra = cop->vs->size;
            transfer_push_some_svalues(cop->vs->item, extra);
            free_empty_array(cop->vs);
          } else {
            extra = 0;
          }

#ifdef CALLOUT_LOOP_PROTECTION
          if (num_max_cost_calls > 0) {
#endif
            set_eval(max_cost);
#ifdef CALLOUT_LOOP_PROTECTION
          }
#endif

          if (cop->ob) {
            if (cop->function.s[0] == APPLY___INIT_SPECIAL_CHAR) {
              error("Illegal function name\n");
            }

            (void) apply(cop->function.s, cop->ob, extra,
                         ORIGIN_INTERNAL);
          } else {
            (void) call_function_pointer(cop->function.f, extra);
          }

          restore_command_giver();
        } catch (const char *) {
          restore_context(&econ);
          if (outoftime) {
#ifdef CALLOUT_LOOP_PROTECTION
            if (num_max_cost_calls > 0) {
#endif
              /* Transfer control to the backend loop so that it can attend to other duties */
              debug_message("Maximum evaluation cost reached while trying to process call_outs\n");
              pop_context(&econ);
              return;
#ifdef CALLOUT_LOOP_PROTECTION
            } else if (!warned_about_recursion) {
              warned_about_recursion = 1;
              /* A newly added immediate call_out tripped max_cost.  Force all reamining
               * call_outs to run with the outoftime flag set, which will force them to
               * throw their errors (this aids in troubleshooting after the fact.)
               */
              debug_message("Maximum evaluation cost reached while processing recursive call_outs; breaking possible loop\n");
            }
#endif
          }
        }
        free_called_call(cop);
        cop = 0;
      }
    }
    /* Ok, no more scheduled call_outs for current_time */
    if (current_time < real_time) {
      /* Time marches onward! */
      if (call_list[tm]) {
        call_list[tm]->delta--;
      }
      current_time++;
      DBG_CALLOUT("   current_time = %ld\n", current_time);
#ifndef POSIX_TIMERS
      if (!(current_time % HEARTBEAT_INTERVAL)) {
        call_heart_beat();
      }
#endif
    } else {
      /* We're done! */
      break;
    }
  }
  DBG_CALLOUT("Done.\n");
  pop_context(&econ);
}

static int time_left(int slot, int delay)
{
  int current_slot = current_time & (CALLOUT_CYCLE_SIZE - 1);
  if (slot >= current_slot) {
    return (slot - current_slot) + delay * CALLOUT_CYCLE_SIZE;
  } else {
    return (slot - current_slot) + (delay + 1) * CALLOUT_CYCLE_SIZE;
  }
}

/*
 * Throw away a call out. First call to this function is discarded.
 * The time left until execution is returned.
 * -1 is returned if no call out pending.
 */
int remove_call_out(object_t *ob, const char *fun)
{
  pending_call_t **copp, *cop;
  int delay;
  int i;

  if (!ob) { return -1; }

  DBG_CALLOUT("remove_call_out: /%s \"%s\"\n", ob->obname, fun);

  for (i = 0; i < CALLOUT_CYCLE_SIZE; i++) {
    delay = 0;
    for (copp = &call_list[i]; *copp; copp = &(*copp)->next) {
      DBG_CALLOUT("   Slot: %i\n", i);
      delay += (*copp)->delta;
      if ((*copp)->ob == ob && strcmp((*copp)->function.s, fun) == 0) {
        cop = *copp;
        if (cop->next) {
          cop->next->delta += cop->delta;
        }
        *copp = cop->next;
        free_call(cop);
        DBG_CALLOUT("   found.\n");
        return time_left(i, delay);
      }
    }
  }
  DBG_CALLOUT("   not found.\n");
  return -1;
}

#ifdef CALLOUT_HANDLES
int remove_call_out_by_handle(LPC_INT handle)
{
  pending_call_t **copp, *cop;
  int delay = 0;

  DBG_CALLOUT("remove_call_out_by_handle: handle: %i slot: %i\n",
              handle, handle & (CALLOUT_CYCLE_SIZE - 1));

  for (copp = &call_list[handle & (CALLOUT_CYCLE_SIZE - 1)]; *copp; copp = &(*copp)->next) {
    delay += (*copp)->delta;
    if ((*copp)->handle == handle) {
      cop = *copp;
      if (cop->next) {
        cop->next->delta += cop->delta;
      }
      *copp = cop->next;
      free_call(cop);
      return time_left(handle & (CALLOUT_CYCLE_SIZE - 1), delay);
    }
  }
  return -1;
}

int find_call_out_by_handle(LPC_INT handle)
{
  pending_call_t *cop;
  int delay = 0;

  DBG_CALLOUT("find_call_out_by_handle: handle: %" LPC_INT_FMTSTR_P " slot: %" LPC_INT_FMTSTR_P "\n",
              handle, handle & (CALLOUT_CYCLE_SIZE - 1));

  for (cop = call_list[handle & (CALLOUT_CYCLE_SIZE - 1)]; cop; cop = cop->next) {
    delay += cop->delta;
    if (cop->handle == handle) {
      return time_left(handle & (CALLOUT_CYCLE_SIZE - 1), delay);
    }
  }
  return -1;
}
#endif

int find_call_out(object_t *ob, const char *fun)
{
  pending_call_t *cop;
  int delay;
  int i;

  if (!ob) { return -1; }

  DBG_CALLOUT("find_call_out: /%s \"%s\"\n", ob->obname, fun);

  for (i = 0; i < CALLOUT_CYCLE_SIZE; i++) {
    delay = 0;
    DBG_CALLOUT("   Slot: %i\n", i);
    for (cop = call_list[i]; cop; cop = cop->next) {
      delay += cop->delta;
      if (cop->ob == ob && strcmp(cop->function.s, fun) == 0) {
        return time_left(i, delay);
      }
    }
  }
  return -1;
}

int print_call_out_usage(outbuffer_t *ob, int verbose)
{
  int i, j;
  pending_call_t *cop;

  for (i = 0, j = 0; j < CALLOUT_CYCLE_SIZE; j++)
    for (cop = call_list[j]; cop; cop = cop->next) {
      i++;
    }

  if (verbose == 1) {
    outbuf_add(ob, "Call out information:\n");
    outbuf_add(ob, "---------------------\n");
    outbuf_addv(ob, "Number of allocated call outs: %8d, %8d bytes\n",
                num_call, num_call * sizeof(pending_call_t));
    outbuf_addv(ob, "Current length: %d\n", i);
  } else {
    if (verbose != -1)
      outbuf_addv(ob, "call out:\t\t\t%8d %8d (current length %d)\n", num_call,
                  num_call * sizeof(pending_call_t), i);
  }
  return num_call * sizeof(pending_call_t);
}

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_call_outs()
{
  pending_call_t *cop;
  int i;

  for (i = 0; i < CALLOUT_CYCLE_SIZE; i++) {
    for (cop = call_list[i]; cop; cop = cop->next) {
      if (cop->vs) {
        cop->vs->extra_ref++;
      }
      if (cop->ob) {
        cop->ob->extra_ref++;
        EXTRA_REF(BLOCK(cop->function.s))++;
      } else {
        cop->function.f->hdr.extra_ref++;
      }
#ifdef THIS_PLAYER_IN_CALL_OUT
      if (cop->command_giver) {
        cop->command_giver->extra_ref++;
      }
#endif
    }
  }
}
#endif
/*
 * Construct an array of all pending call_outs. Every item in the array
 * consists of 3 items (but only if the object not is destructed):
 * 0: The object.
 * 1: The function (string).
 * 2: The delay.
 */
array_t *get_all_call_outs()
{
  int i, j, delay;
  pending_call_t *cop;
  array_t *v;

  for (i = 0, j = 0; j < CALLOUT_CYCLE_SIZE; j++)
    for (cop = call_list[j]; cop; cop = cop->next) {
      object_t *ob = (cop->ob ? cop->ob : cop->function.f->hdr.owner);
      if (ob && !(ob->flags & O_DESTRUCTED)) {
        i++;
      }
    }

  v = allocate_empty_array(i);

  for (i = 0, j = 0; j < CALLOUT_CYCLE_SIZE; j++) {
    delay = 0;
    for (cop = call_list[j]; cop; cop = cop->next) {
      array_t *vv;
      object_t *ob;

      delay += cop->delta;
      ob = (cop->ob ? cop->ob : cop->function.f->hdr.owner);
      if (!ob || (ob->flags & O_DESTRUCTED)) {
        continue;
      }
      vv = allocate_empty_array(3);
      if (cop->ob) {
        vv->item[0].type = T_OBJECT;
        vv->item[0].u.ob = cop->ob;
        add_ref(cop->ob, "get_all_call_outs");
        vv->item[1].type = T_STRING;
        vv->item[1].subtype = STRING_SHARED;
        vv->item[1].u.string = make_shared_string(cop->function.s);
      } else {
        outbuffer_t tmpbuf;
        svalue_t tmpval;

        tmpbuf.real_size = 0;
        tmpbuf.buffer = 0;

        tmpval.type = T_FUNCTION;
        tmpval.u.fp = cop->function.f;

        svalue_to_string(&tmpval, &tmpbuf, 0, 0, 0);

        vv->item[0].type = T_OBJECT;
        vv->item[0].u.ob = cop->function.f->hdr.owner;
        add_ref(cop->function.f->hdr.owner, "get_all_call_outs");
        vv->item[1].type = T_STRING;
        vv->item[1].subtype = STRING_SHARED;
        vv->item[1].u.string = make_shared_string(tmpbuf.buffer);
        FREE_MSTR(tmpbuf.buffer);
      }
      vv->item[2].type = T_NUMBER;
      vv->item[2].u.number = time_left(j, delay);

      v->item[i].type = T_ARRAY;
      v->item[i++].u.arr = vv;  /* Ref count is already 1 */
    }
  }
  return v;
}

void
remove_all_call_out(object_t *obj)
{
  pending_call_t **copp, *cop;
  int i;

  for (i = 0; i < CALLOUT_CYCLE_SIZE; i++) {
    copp = &call_list[i];
    while (*copp) {
      if (((*copp)->ob &&
           (((*copp)->ob == obj) || ((*copp)->ob->flags & O_DESTRUCTED))) ||
          (!(*copp)->ob &&
           ((*copp)->function.f->hdr.owner == obj ||
            !(*copp)->function.f->hdr.owner ||
            (*copp)->function.f->hdr.owner->flags & O_DESTRUCTED))) {
        cop = *copp;
        if (cop->next) {
          cop->next->delta += cop->delta;
        }
        *copp = cop->next;
        free_call(cop);
      } else {
        copp = &(*copp)->next;
      }
    }
  }
}

void reclaim_call_outs()
{
  pending_call_t *cop;
  int i;

  remove_all_call_out(0); /* removes call_outs to destructed objects */

#ifdef THIS_PLAYER_IN_CALL_OUT
  for (i = 0; i < CALLOUT_CYCLE_SIZE; i++) {
    cop = call_list[i];
    while (cop) {
      if (cop->command_giver && (cop->command_giver->flags & O_DESTRUCTED)) {
        free_object(&cop->command_giver, "reclaim_call_outs");
        cop->command_giver = 0;
      }
      cop = cop->next;
    }
  }
#endif
}
