/*
 * uid.c
 * Created by: Erik Kay
 * Description: new uid / euid data structures and security
 * Modification:
 *    11-1-92 - Erik Kay - initial creation
 */

#include <stdio.h>  /* to get NULL */
#ifdef __386BSD__
#include <string.h>
#endif
#include "config.h"
#include "lint.h"
#include "uid.h"

userid_t *uids = NULL;
userid_t *backbone_uid;
userid_t *root_uid;

userid_t *find_uid (name)
char *name;
{
  userid_t *uid = uids;
  while (uid) {
    if (!strcmp(name, uid->name))
      return uid;
    uid = uid->next;
  }
  return NULL;
}

void insert_uid (new_uid)
userid_t *new_uid;
{
  userid_t *uid = uids;

  if (uid) {
    while (uid->next) {
      uid = uid->next;
    }
    uid->next = new_uid;
  } else uids = new_uid;
  return;
}

userid_t *add_uid (name)
char *name;
{
  userid_t *uid;
  if ((uid = find_uid(name)))
    return uid;
  uid = (userid_t *)DMALLOC(sizeof(userid_t), 119, "add_uid: 1");
  uid->next = NULL;
  uid->name = (char *)DMALLOC(strlen(name)+1, 120, "add_uid: 2");
  strcpy(uid->name, name);
  insert_uid(uid);
  return uid;
}

userid_t *set_root_uid (name)
char *name;
{
  userid_t *uid;
  uid = add_uid(name);
  if (uid)
    root_uid = uid;
  return uid;
}
  
userid_t *set_backbone_uid (name)
char *name;
{
  userid_t *uid;
  uid = add_uid(name);
  if (uid)
    backbone_uid = uid;
  return uid;
}
