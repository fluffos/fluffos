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

static object_t* ob;

#ifdef F_EXPORT_UID

void f_export_uid(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  if (current_object->euid == nullptr) {
    error("Illegal to export uid 0\n");
  }
  ob = f.object(0);
  if (ob->euid) {
    f.ret(0);
  } else {
    ob->uid = current_object->euid;
    f.ret(1);
  }
}

#endif

#ifdef F_GETEUID

void f_geteuid(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  if (f.type(0) & T_OBJECT) {
    ob = f.object(0);
    if (ob->euid) {
      f.ret_str_shared(ob->euid->name);
      return;
    }
    f.ret(0);
    return;

  } else if (f.type(0) & T_FUNCTION) {
    funptr_t* fp;
    if ((fp = f.funptr(0))->hdr.owner && fp->hdr.owner->euid) {
      f.ret_str_shared(fp->hdr.owner->euid->name);
      return;
    }
    f.ret(0);
  }
}

#endif

#ifdef F_GETUID

void f_getuid(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  ob = f.object(0);

  if (!ob->uid) {
    error("Object has no UID: This could happen if you call getuid() in master:creator_file().");
  }

  f.ret_str_shared(ob->uid->name);
}

#endif

#ifdef F_SETEUID

void f_seteuid(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  svalue_t* ret;

  if (f.type(0) & T_NUMBER) {
    if (f.number(0)) {
      bad_arg(1, F_SETEUID);
    }
    current_object->euid = nullptr;
    f.set_top_number(1);
    f.done();
    return;
  }
  {
    LPCFrame b = LPCFrame::builder();
    b.push(current_object);
    b.push(f.argp(0));
    ret = apply_master_ob(APPLY_VALID_SETEUID, 2);
    b.done();
  }
  if (!MASTER_APPROVED(ret, "valid_seteuid")) {
    f.ret(0);
    return;
  }
  current_object->euid = add_uid(f.string(0));
  f.ret(1);
}

#endif

/* Support functions */
static auto comp = [](userid_t* uid1, userid_t* uid2) { return uid1->name < uid2->name; };
static auto uids = std::set<userid_t*, decltype(comp)>(comp);

// static tree *uids = NULL;
userid_t* backbone_uid = nullptr;
userid_t* root_uid = nullptr;

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_all_uid_nodes() {
  for (auto* i : uids) {
    ++EXTRA_REF(BLOCK(i->name));
  }
}
#endif

userid_t* add_uid(const char* name) {
  userid_t* uid;
  userid_t t_uid;

  t_uid.name = make_shared_string(name);
  auto i = uids.find(&t_uid);
  if (i != uids.end()) {
    free_string(t_uid.name);
    return *i;
  }
  uid = reinterpret_cast<userid_t*>(DMALLOC(sizeof(userid_t), TAG_UID, "add_uid"));
  uid->name = t_uid.name;
  uids.insert(uid);
  return uid;
}

userid_t* set_root_uid(const char* name) {
  if (!root_uid) {
    return root_uid = add_uid(name);
  }
  auto i = uids.find(root_uid);
  uids.erase(i);
  root_uid->name = make_shared_string(name);
  uids.insert(root_uid);
  return root_uid;
}

userid_t* set_backbone_uid(const char* name) {
  if (!backbone_uid) {
    return backbone_uid = add_uid(name);
  }
  auto i = uids.find(backbone_uid);
  uids.erase(i);
  backbone_uid->name = make_shared_string(name);
  uids.insert(backbone_uid);
  return backbone_uid;
}
