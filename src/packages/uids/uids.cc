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

vm::Value f_export_uid(vm::EfunArgs&, object_t* target) {
  if (current_object->euid == nullptr) {
    error("Illegal to export uid 0\n");
  }
  ob = target;
  if (ob->euid) {
    return 0;
  }
  ob->uid = current_object->euid;
  return 1;
}

#endif

#ifdef F_GETEUID

vm::Value f_geteuid(vm::EfunArgs&, vm::Arg who) {
  if (who.is(T_OBJECT)) {
    ob = who.object();
    if (ob->euid) {
      return vm::Value::str_shared(ob->euid->name);
    }
    return 0;
  }
  funptr_t* fp = who.funptr();
  if (fp->hdr.owner && fp->hdr.owner->euid) {
    return vm::Value::str_shared(fp->hdr.owner->euid->name);
  }
  return 0;
}

#endif

#ifdef F_GETUID

vm::Value f_getuid(vm::EfunArgs&, object_t* target) {
  ob = target;

  if (!ob->uid) {
    error("Object has no UID: This could happen if you call getuid() in master:creator_file().");
  }

  return vm::Value::str_shared(ob->uid->name);
}

#endif

#ifdef F_SETEUID

vm::Value f_seteuid(vm::EfunArgs&, vm::Arg euid) {
  if (euid.is(T_NUMBER)) {
    if (euid.number()) {
      bad_arg(1, F_SETEUID);
    }
    current_object->euid = nullptr;
    return 1;
  }
  vm::CallResult ret =
      vm::Call(vm::Value::object(current_object), euid.copy()).apply_master(APPLY_VALID_SETEUID);
  if (!MASTER_APPROVED(ret, "valid_seteuid")) {
    return 0;
  }
  current_object->euid = add_uid(euid.string().c_str());
  return 1;
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
