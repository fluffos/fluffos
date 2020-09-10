/*
 * heartbeat.cc
 */

#include "base/package_api.h"

#include "packages/core/heartbeat.h"

#include <algorithm>
#include <deque>
#include <set>

struct heart_beat_t {
  bool deleted;

  object_t *ob;
  short heart_beat_ticks;
  short time_to_heart_beat;
};

// Global pointer to current object executing heartbeat.
object_t *g_current_heartbeat_obj;
static heart_beat_t *g_current_heartbeat;

/*
 * TODO: ideally we should be using vector here for performance, however dealing with
 * enable/disable heartbeat during execution make it diffcult to implement correctly.
 */
static std::deque<heart_beat_t *> heartbeats, heartbeats_next;

/* Call all heart_beat() functions in all objects.
 *
 * Set command_giver to current_object if it is a living object. If the object
 * is shadowed, check the shadowed object if living. There is no need to save
 * the value of the command_giver, as the caller resets it to 0 anyway.  */

void call_heart_beat() {
  // Register for next call
  add_gametick_event(std::chrono::milliseconds(CONFIG_INT(__RC_HEARTBEAT_INTERVAL_MSEC__)),
                     tick_event::callback_type(call_heart_beat));

  if (!heartbeats_next.empty()) {
    for (auto hb : heartbeats_next) {
      heartbeats.push_back(hb);
    }
    heartbeats_next.clear();
  }
  // During the execution of heartbeat func, object can add/delete heartbeats, thus we can't use a
  // simple loop here. Instead, we extract each heartbeats, execute it, add it to
  // heartbeats_next. After all execution, heartbeats_after and heartbeats is swapped.
  //
  // NOTE: The order of heartbeat execution is preserved.
  while (!heartbeats.empty()) {
    auto curr_hb = heartbeats.front();

    // Move heartbeat into heartbeat_next, if still valid.
    {
      heartbeats.pop_front();

      if (curr_hb->deleted) {
        delete curr_hb;
        continue;
      }

      // Skip if we are in some weird situation.
      if (curr_hb->ob == nullptr || !(curr_hb->ob->flags & O_HEART_BEAT) ||
          curr_hb->ob->flags & O_DESTRUCTED) {
        delete curr_hb;
        continue;
      }
      heartbeats_next.push_back(curr_hb);
    }

    auto ob = curr_hb->ob;

    if (--curr_hb->heart_beat_ticks > 0) {
      continue;
    } else {
      curr_hb->heart_beat_ticks = curr_hb->time_to_heart_beat;
    }

    // No heartbeat function
    if (ob->prog->heart_beat == 0) {
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
    g_current_heartbeat_obj = ob;
    g_current_heartbeat = curr_hb;

    error_context_t econ;

    save_context(&econ);
    try {
      set_eval(max_eval_cost);
      // TODO: provide a safe_call_direct()
      call_direct(ob, ob->prog->heart_beat - 1, ORIGIN_DRIVER, 0);
      pop_stack(); /* pop the return value */
    } catch (const char *) {
      restore_context(&econ);
    }
    pop_context(&econ);

    restore_command_giver();
    g_current_heartbeat_obj = nullptr;
    g_current_heartbeat = nullptr;
  }
  std::swap(heartbeats, heartbeats_next);
} /* call_heart_beat() */

// Query heartbeat interval for a object
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

// Modifying heartbeat for a object.
//
// NOTE: This may get called during heartbeat. Care must be taken to
// make sure it works.
//
// Removing heartbeat just need to remove the flag from objects.
// New heartbeats must be added to heartbeats_next.
int set_heart_beat(object_t *ob, int to) {
  if (ob->flags & O_DESTRUCTED) {
    return 0;
  }

  // This was done in previous driver code, keep here for compat.
  if (to < 0) {
    // TODO: log a warning
    to = 1;
  }

  // Here are 3 possible cases:
  // 1) Object currently doesn't have heartbeat.
  // 2) Object currently have heartbeat and is in the queue.
  // 3) Object is modifying itself during its heartbeat execution.

  // Search from 3 places for target_hb.
  heart_beat_t *target_hb = nullptr;

  if (g_current_heartbeat_obj == ob) {
    target_hb = g_current_heartbeat;
  }
  if (target_hb == nullptr) {
    for (auto hb : heartbeats_next) {
      if (hb->ob == ob) {
        target_hb = hb;
        break;
      }
    }
  }
  if (target_hb == nullptr) {
    for (auto hb : heartbeats) {
      if (hb->ob == ob) {
        target_hb = hb;
        break;
      }
    }
  }

  // Removal: set the flag and hb will be deleted in next round.
  if (to == 0) {
    ob->flags &= ~O_HEART_BEAT;
    if (target_hb != nullptr) {
      target_hb->deleted = true;
      return 1;
    } else {
      return 0;
    }
  } else {
    ob->flags |= O_HEART_BEAT;
    // Add: Didn't find target_hb, we need to create a new one.
    if (target_hb == nullptr) {
      target_hb = new heart_beat_t();
      heartbeats_next.push_back(target_hb);

      target_hb->ob = ob;
      target_hb->time_to_heart_beat = to;
      target_hb->heart_beat_ticks = to;
      return 1;
    } else {
      // Modifying: target_hb is found.
      DEBUG_CHECK(target_hb->ob != ob, "Driver BUG: set_heart_beat");
      target_hb->time_to_heart_beat = to;
      target_hb->heart_beat_ticks = to;
      return 1;
    }
  }
  return 0;
}

int heart_beat_status(outbuffer_t *buf, int verbose) {
  if (verbose == 1) {
    outbuf_add(buf, "Heart beat information:\n");
    outbuf_add(buf, "-----------------------\n");
    outbuf_addv(buf, "Number of objects with heart beat: %" PRIu64 ".\n",
                heartbeats.size() + heartbeats_next.size());
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

void check_heartbeats() {
  std::set<object_t *> objset;
  for (auto hb : heartbeats) {
    objset.insert(hb->ob);
  }
  for (auto hb : heartbeats_next) {
    objset.insert(hb->ob);
  }
  DEBUG_CHECK((objset.size() != heartbeats.size() + heartbeats_next.size()),
              "Driver BUG: Duplicated/Missing heartbeats found");
}

void clear_heartbeats() {
  // TODO: instead of clearing everything blindly, should go through all objects with heartbeat flag
  // and delete corresponding heartbeats, thus exposing leftovers.
  for (auto hb : heartbeats) {
    delete hb;
  }
  for (auto hb : heartbeats_next) {
    delete hb;
  }
}
