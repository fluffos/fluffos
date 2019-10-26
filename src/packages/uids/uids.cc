/*
 * uid.c
 *
 * Created by: Erik Kay
 * Description: new uid / euid data structures and security
 * Modification:
 *   11-1-92 - Erik Kay - initial creation
 *   94.07.14 - Robocoder - replaced linked list with AVL tree, and
 *                made uids into shared strings
 * 2016.09.15 - Carter Cheng - replaced AVL tree with std::set.
 */
#include "base/package_api.h"

#include "packages/uids/uids.h"

#include <set>

static object_t *ob;

#ifdef F_EXPORT_UID

void f_export_uid(void) {
  if (current_object->euid == nullptr) {
    error("Illegal to export uid 0\n");
  }
  ob = sp->u.ob;
  if (ob->euid) {
    free_object(&ob, "f_export_uid:1");
    *sp = const0;
  } else {
    ob->uid = current_object->euid;
    free_object(&ob, "f_export_uid:2");
    *sp = const1;
  }
}

#endif

#ifdef F_GETEUID

void f_geteuid(void) {
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

void f_getuid(void) {
  ob = sp->u.ob;

  if (!ob->uid) {
    error("Object has no UID: This could happen if you call getuid() in master:creator_file().");
  }

  put_constant_string(ob->uid->name);
  free_object(&ob, "f_getuid");
}

#endif

#ifdef F_SETEUID

void f_seteuid(void) {
  svalue_t *arg;
  svalue_t *ret;

  if (sp->type & T_NUMBER) {
    if (sp->u.number) {
      bad_arg(1, F_SETEUID);
    }
    current_object->euid = nullptr;
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
static auto comp = [](userid_t *uid1, userid_t *uid2) { return uid1->name < uid2->name; };
static auto uids = std::set<userid_t *, decltype(comp)>(comp);

// static tree *uids = NULL;
userid_t *backbone_uid = nullptr;
userid_t *root_uid = nullptr;

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_all_uid_nodes() {
  for (auto i : uids) {
    ++EXTRA_REF(BLOCK(i->name));
  }
}
#endif

userid_t *add_uid(const char *name) {
  userid_t *uid;
  userid_t t_uid;

  t_uid.name = make_shared_string(name);
  auto i = uids.find(&t_uid);
  if (i != uids.end()) {
    free_string(t_uid.name);
    return *i;
  } else {
    uid = reinterpret_cast<userid_t *>(DMALLOC(sizeof(userid_t), TAG_UID, "add_uid"));
    uid->name = t_uid.name;
    uids.insert(uid);
    return uid;
  }
}

userid_t *set_root_uid(const char *name) {
  if (!root_uid) {
    return root_uid = add_uid(name);
  } else {
    auto i = uids.find(root_uid);
    uids.erase(i);
    root_uid->name = make_shared_string(name);
    uids.insert(root_uid);
    return root_uid;
  }
}

userid_t *set_backbone_uid(const char *name) {
  if (!backbone_uid) {
    return backbone_uid = add_uid(name);
  } else {
    auto i = uids.find(backbone_uid);
    uids.erase(i);
    backbone_uid->name = make_shared_string(name);
    uids.insert(backbone_uid);
    return backbone_uid;
  }
}
