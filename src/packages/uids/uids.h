/*
 * uid.h
 * Created by: Erik Kay
 * Description: new uid / euid data structures and security
 * Modification:
 *    11-1-92 - Erik Kay - initial creation
 *    94.07.09 - Robocoder - modified to use AVL tree
 */

#ifndef _UID_H_
#define _UID_H_

struct userid_t {
  char *name;
};

extern userid_t *backbone_uid;
extern userid_t *root_uid;

userid_t *add_uid(const char *name);
userid_t *set_root_uid(const char *name);
userid_t *set_backbone_uid(const char *name);

void mark_all_uid_nodes(void);

#endif
