/*
 * uid.c
 *
 * Created by: Erik Kay
 * Description: new uid / euid data structures and security
 * Modification:
 *   11-1-92 - Erik Kay - initial creation
 *   94.07.14 - Robocoder - replaced linked list with AVL tree, and
 *                made uids into shared strings
 */

#include "std.h"
#include "uid.h"
#include "avltree.h"
#include "stralloc.h"
#ifdef DEBUGMALLOC_EXTENSIONS
#include "md.h"
#endif

#ifndef NO_UIDS
static tree *uids = NULL;
userid_t *backbone_uid = NULL;
userid_t *root_uid = NULL;

#ifdef DEBUGMALLOC_EXTENSIONS
static void mark_uid_tree P1(tree *, tr) {
    DO_MARK(tr, TAG_UID);
    DO_MARK(tr->tree_p, TAG_UID);
    
    EXTRA_REF(BLOCK(((userid_t *)tr->tree_p)->name))++;
    if (tr->tree_l)
	mark_uid_tree(tr->tree_l);
    if (tr->tree_r)
	mark_uid_tree(tr->tree_r);
}

void mark_all_uid_nodes() {
    if (uids)
	mark_uid_tree(uids);
}
#endif

static int uidcmp PROT((userid_t *, userid_t *));

static int uidcmp P2(userid_t *, uid1, userid_t *, uid2)
{
    register char *name1, *name2;

    name1 = uid1->name;
    name2 = uid2->name;
    return (name1 < name2 ? -1 : (name1 > name2 ? 1 : 0));
}

userid_t *add_uid P1(char *, name)
{
    userid_t *uid, t_uid;
    char *sname;

    sname = make_shared_string(name);
    t_uid.name = sname;
    if (uid = (userid_t *) tree_srch(uids, uidcmp, (char *) &t_uid)) {
	free_string(sname);
    } else {
	uid = ALLOCATE(userid_t, TAG_UID, "add_uid");
	uid->name = sname;
	tree_add(&uids, uidcmp, (char *) uid, NULL);
    }
    return uid;
}

userid_t *set_root_uid P1(char *, name)
{
    if (!root_uid)
	return root_uid = add_uid(name);

    tree_delete(&uids, uidcmp, (char *) root_uid, NULL);
    root_uid->name = make_shared_string(name);
    tree_add(&uids, uidcmp, (char *) root_uid, NULL);
    return root_uid;
}

userid_t *set_backbone_uid P1(char *, name)
{
    if (!backbone_uid)
	return backbone_uid = add_uid(name);

    tree_delete(&uids, uidcmp, (char *) backbone_uid, NULL);
    backbone_uid->name = make_shared_string(name);
    tree_add(&uids, uidcmp, (char *) backbone_uid, NULL);
    return backbone_uid;
}
#endif
