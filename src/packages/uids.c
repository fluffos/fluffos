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
#ifdef LATTICE
#include "/lpc_incl.h"
#include "/eoperators.h"
#include "/avltree.h"
#include "/md.h"
#include "/uids.h"
#include "/master.h"
#else
#include "../lpc_incl.h"
#include "../eoperators.h"
#include "../avltree.h"
#include "../md.h"
#include "uids.h"
#include "../master.h"
#include "../efun_protos.h"
#endif

static object_t *ob;

#ifdef F_EXPORT_UID
void
f_export_uid (void)
{
    if (current_object->euid == NULL)
        error("Illegal to export uid 0\n");
    ob = sp->u.ob;
    if (ob->euid) {
        free_object(&ob, "f_export_uid:1");
        *sp = const0;
    }
    else {
        ob->uid = current_object->euid;
        free_object(&ob, "f_export_uid:2");
        *sp = const1;
    }
}
#endif

#ifdef F_GETEUID
void
f_geteuid (void)
{
    if (sp->type & T_OBJECT) {
        ob = sp->u.ob;
        if (ob->euid) {
            put_constant_string(ob->euid->name);
            free_object(&ob, "f_geteuid:1");
            return;
        } else {
            free_object(&ob, "f_geteuid:2");
            *sp = const0;
            return;
        }
    } else if (sp->type & T_FUNCTION) {
        funptr_t *fp;
        if ((fp = sp->u.fp)->hdr.owner && fp->hdr.owner->euid) {
            put_constant_string(fp->hdr.owner->euid->name);
            free_funp(fp);
            return;
        } 
        free_funp(fp);
        *sp = const0;
    }
}
#endif

#ifdef F_GETUID
void
f_getuid (void)
{
    ob = sp->u.ob;

    DEBUG_CHECK(ob->uid == NULL, "UID is a null pointer\n");
    put_constant_string(ob->uid->name);
    free_object(&ob, "f_getuid");
}
#endif

#ifdef F_SETEUID
void
f_seteuid (void)
{
    svalue_t *arg;
    svalue_t *ret;

    if (sp->type & T_NUMBER) {
        if (sp->u.number)
            bad_arg(1, F_SETEUID);
        current_object->euid = NULL;
        sp->u.number = 1;
        return;
    }
    arg = sp;
    push_object(current_object);
    push_svalue(arg);
    ret = apply_master_ob(APPLY_VALID_SETEUID, 2);
    if (!MASTER_APPROVED(ret)) {
        free_string_svalue(sp);
        *sp = const0;
        return;
    }
    current_object->euid = add_uid(sp->u.string);
    free_string_svalue(sp);
    *sp = const1;
}
#endif

/* Support functions */
static tree *uids = NULL;
userid_t *backbone_uid = NULL;
userid_t *root_uid = NULL;

#ifdef DEBUGMALLOC_EXTENSIONS
static void mark_uid_tree (tree * tr) {
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

static int uidcmp (void *, void *);

static int uidcmp (void *uid1, void *uid2)
{
    register char *name1, *name2;

    name1 = ((userid_t *)uid1)->name;
    name2 = ((userid_t *)uid2)->name;
    return (name1 < name2 ? -1 : (name1 > name2 ? 1 : 0));
}

userid_t *add_uid (const char * name)
{
    userid_t *uid, t_uid;
    char *sname;

    sname = make_shared_string(name);
    t_uid.name = sname;
    if ((uid = (userid_t *) tree_srch(uids, uidcmp, (char *) &t_uid))) {
        free_string(sname);
    } else {
        uid = ALLOCATE(userid_t, TAG_UID, "add_uid");
        uid->name = sname;
        tree_add(&uids, uidcmp, (char *) uid, NULL);
    }
    return uid;
}

userid_t *set_root_uid (const char * name)
{
    if (!root_uid)
        return root_uid = add_uid(name);

    tree_delete(&uids, uidcmp, (char *) root_uid, NULL);
    root_uid->name = make_shared_string(name);
    tree_add(&uids, uidcmp, (char *) root_uid, NULL);
    return root_uid;
}

userid_t *set_backbone_uid (const char * name)
{
    if (!backbone_uid)
        return backbone_uid = add_uid(name);

    tree_delete(&uids, uidcmp, (char *) backbone_uid, NULL);
    backbone_uid->name = make_shared_string(name);
    tree_add(&uids, uidcmp, (char *) backbone_uid, NULL);
    return backbone_uid;
}
