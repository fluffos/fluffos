/*
 * user.cc
 *
 *  Created on: Oct 16, 2014
 *      Author: sunyc
 */
#include "base/std.h"

#include "user.h"

#include <algorithm>   // for count_if, for_each, remove
#include <cstring>     // for memset
#include <functional>  // for function
#include <utility>
#include <vector>  // for vector

#include "interactive.h"  // for interactive_t->ob
#include "vm/vm.h"

// structure that holds all users
static std::vector<interactive_t *> all_users;

interactive_t *user_add() {
  auto user = reinterpret_cast<interactive_t *>(
      DMALLOC(sizeof(interactive_t), TAG_INTERACTIVE, "new_user_handler"));
  memset(user, 0, sizeof(*user));
  all_users.push_back(user);
  return user;
}

void user_del(interactive_t *user) {
  // remove it from global table.
  all_users.erase(std::remove(all_users.begin(), all_users.end(), user), all_users.end());
}

// Get a copy of all users
const std::vector<interactive_t *> &users() { return all_users; }

// Count users
int users_num(bool include_hidden) {
  if (include_hidden) {
    return all_users.size();
  } else {
    return std::count_if(all_users.begin(), all_users.end(),
                         [](interactive_t *user) { return (user->ob->flags & O_HIDDEN) == 0; });
  }
}

void users_foreach(std::function<void(interactive_t *)> func) {
  std::for_each(all_users.begin(), all_users.end(), std::move(func));
}
