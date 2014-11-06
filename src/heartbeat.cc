/*
 * heartbeat.cc
 *
 *  Created on: Nov 5, 2014
 *      Author: sunyc
 */
#include "std.h"

#include "heartbeat.h"

#include "eval.h"  // for set_eval
#include "outbuf.h"
#include "backend.h"

#include <algorithm>
#include <deque>

struct heart_beat_t {
  object_t *ob;
  short heart_beat_ticks;
  short time_to_heart_beat;
};

// Global pointer to current object executing heartbeat.
object_t *g_current_heartbeat_obj;
heart_beat_t *g_current_heartbeat;

static std::deque<heart_beat_t *> heartbeats, heartbeats_next;

static int num_hb_calls = 0; /* How many times has heartbeat been executed? */

/* Call all heart_beat() functions in all objects.  Also call the next reset,
 * and the call out.
 * We do heart beats by moving each object done to the end of the heart beat
 * list before we call its function, and always using the item at the head
 * of the list as our function to call.  We keep calling heart beats until
 * a timeout or we have done num_heart_objs calls.  It is done this way so
 * that objects can delete heart beating objects from the list from within
 * their heart beat without truncating the current round of heart beats.
 *
 * Set command_giver to current_object if it is a living object. If the object
 * is shadowed, check the shadowed object if living. There is no need to save
 * the value of the command_giver, as the caller resets it to 0 anyway.  */

void call_heart_beat() {
  // Register for next call
  add_tick_event(HEARTBEAT_INTERVAL, tick_event::callback_type(call_heart_beat));

  num_hb_calls++;

  // During the execution of heartbeat func, object can add/delete heartbeats, thus we can't use a
  // simple loop here. Instead, we extract each heartbeats, execute it, add it to
  // heartbeats_next. After all execution, heartbeats_after and heartbeats is swapped.
  //
  // NOTE: The order of heartbeat execution is preserved.
  while (!heartbeats.empty()) {
    auto curr_hb = heartbeats.front();
    heartbeats.pop_front();

    auto ob = curr_hb->ob;
    // Skip if we are already removed.
    if (!(ob->flags & O_HEART_BEAT)) {
      delete curr_hb;
      continue;
    }

    curr_hb->heart_beat_ticks--;
    // Not yet its turn
    if (curr_hb->heart_beat_ticks > 0) {
      continue;
    }

    curr_hb->heart_beat_ticks = curr_hb->time_to_heart_beat;

    // No heartbeat function
    if (ob->prog->heart_beat != 0) {
      // TODO: should log a warning
      continue;
    }

    object_t *new_command_giver;

    new_command_giver = ob;
#ifndef NO_SHADOWS
    while (new_command_giver->shadowing) {
      new_command_giver = new_command_giver->shadowing;
    }
#endif
#ifndef NO_ADD_ACTION
    if (!(new_command_giver->flags & O_ENABLE_COMMANDS)) {
      new_command_giver = nullptr;
    }
#endif
#ifdef PACKAGE_MUDLIB_STATS
    add_heart_beats(&ob->stats, 1);
#endif
    save_command_giver(new_command_giver);
    if (ob->interactive) {  // note, NOT same as new_command_giver
      current_interactive = ob;
    }

    current_interactive = nullptr;
    g_current_heartbeat_obj = ob;
    g_current_heartbeat = curr_hb;

    error_context_t econ;
    try {
      set_eval(max_cost);
      save_context(&econ);
      // TODO: provide a safe_call_direct()
      call_direct(ob, ob->prog->heart_beat - 1, ORIGIN_DRIVER, 0);

      pop_stack(); /* pop the return value */
      restore_command_giver();

      pop_context(&econ);
    } catch (const char *) {
      restore_context(&econ);
    }

    current_object = nullptr;
    current_prog = nullptr;
    g_current_heartbeat_obj = nullptr;
    g_current_heartbeat = nullptr;

    // see if we have been removed.
    if (!(ob->flags & O_HEART_BEAT)) {
      delete curr_hb;
      continue;
    }
    // schedule for next round execution
    heartbeats_next.push_back(curr_hb);
  }
  std::swap(heartbeats, heartbeats_next);
} /* call_heart_beat() */

// Query how many ticks remaining for a object
// NOTE: Not a very efficient function.
int query_heart_beat(object_t *ob) {
  if (!(ob->flags & O_HEART_BEAT)) {
    return 0;
  }
  if (g_current_heartbeat_obj == ob) {
    return g_current_heartbeat->time_to_heart_beat;
  }
  for (auto hb : heartbeats) {
    if (hb->ob == ob) {
      return hb->time_to_heart_beat;
    }
  }
  for (auto hb : heartbeats_next) {
    if (hb->ob == ob) {
      return hb->time_to_heart_beat;
    }
  }
  return 0;
} /* query_heart_beat() */

// Add or remove an object from the heart beat list.
int set_heart_beat(object_t *ob, int to) {
  if (ob->flags & O_DESTRUCTED) {
    return 0;
  }

  // This was done in previous driver code, keep here for compat.
  if (to < 0) {
    // TODO: log a warning
    to = 1;
  }

  // Removing heartbeat just need to remove the flag, will be deleted during next heartbeat
  // execution.
  if (!to) {
    ob->flags &= ~O_HEART_BEAT;
    return 1;
  }

  // NOTE: New heartbeat should be added to heartbeats_next.
  auto *hb = new heart_beat_t();
  hb->ob = ob;
  hb->time_to_heart_beat = to;
  hb->heart_beat_ticks = to;
  ob->flags |= O_HEART_BEAT;
  heartbeats_next.push_front(hb);

  return 1;
}

int heart_beat_status(outbuffer_t *buf, int verbose) {
  if (verbose == 1) {
    outbuf_add(buf, "Heart beat information:\n");
    outbuf_add(buf, "-----------------------\n");
    outbuf_addv(buf, "Number of objects with heart beat: %d, starts: %d\n",
                heartbeats.size() + heartbeats_next.size(), num_hb_calls);
  }
  return 0;
} /* heart_beat_status() */

#ifdef F_HEART_BEATS
array_t *get_heart_beats() {
  std::deque<object_t *> result;

  bool display_hidden = true;
#ifdef F_SET_HIDE
  display_hidden = valid_hide(current_object);
#endif

  auto fn = [&](heart_beat_t *hb) {
    if (hb->ob->flags & O_HIDDEN) {
      if (!display_hidden) {
        return;
      }
    }
    result.push_back(hb->ob);
  };

  std::for_each(heartbeats.begin(), heartbeats.end(), fn);
  std::for_each(heartbeats_next.begin(), heartbeats_next.end(), fn);

  array_t *arr = allocate_empty_array(result.size());
  int i = 0;
  for (auto obj : result) {
    arr->item[i].type = T_OBJECT;
    arr->item[i].u.ob = obj;
    add_ref(arr->item[i].u.ob, "get_heart_beats");
    i++;
  }
  return arr;
}
#endif

void clear_heartbeats() {
  // TODO: instead of clearing everything blindly, should go through all objects with heartbeat flag
  // and delete corresponding heartbeats, thus exposing leftovers.
  for (auto hb: heartbeats) {
    delete hb;
  }
  for (auto hb: heartbeats_next) {
    delete hb;
  }
  if (g_current_heartbeat != nullptr) {
    delete g_current_heartbeat;
  }
}

