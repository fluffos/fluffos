/*
 * user.h
 *
 *  Created on: Oct 16, 2014
 *      Author: sunyc
 */

#ifndef USER_H_
#define USER_H_

#include <functional>
#include <vector>

struct interactive_t;

// APIs
interactive_t *user_add();
void user_del(interactive_t *);
// Returns all users
const std::vector<interactive_t *> &users();
// Count users
int users_num(bool);
void users_foreach(std::function<void(interactive_t *)>);

#endif /* USER_H_ */
