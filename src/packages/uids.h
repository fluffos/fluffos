/*
 * uid.h
 * Created by: Erik Kay
 * Description: new uid / euid data structures and security
 * Modification:
 *    11-1-92 - Erik Kay - initial creation
 *    94.07.09 - Robocoder - modified to use AVL tree
 */

#if !defined(_UID_H_) && defined(PACKAGE_UIDS)
#define _UID_H_

#include "../std.h"

typedef struct _uid {
    char *name;
}    userid_t;

extern userid_t *backbone_uid;
extern userid_t *root_uid;

userid_t *add_uid (const char *name);
userid_t *set_root_uid (const char *name);
userid_t *set_backbone_uid (const char *name);
#ifdef DEBUGMALLOC_EXTENSIONS
void mark_all_uid_nodes (void);
#endif

#endif
