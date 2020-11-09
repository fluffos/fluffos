#include "base/std.h"

#include "vm/internal/master.h"

#include <cstdio>   // for fprintf
#include <cstdlib>  // for exit

#include "packages/mudlib_stats/mudlib_stats.h"  // for set_domain_author etc
#include "packages/uids/uids.h"                  // for set_uids etc

#include "vm/internal/base/machine.h"
#include "applies_table.autogen.h"
#include "vm/internal/apply.h"
#include "vm/internal/simulate.h"

struct object_t *master_ob = nullptr;
struct function_lookup_info_t *master_applies = nullptr;

/* Note that now, once the master object loads once, there is ALWAYS a
 * master object, so the only way this can fail is if the master object
 * hasn't loaded yet.  In that case, we return (svalue_t *)-1, and the
 * calling routine should let the check succeed.
 */
svalue_t *apply_master_ob(int fun, int num_arg) {
  if (!master_ob) {
    pop_n_elems(num_arg);
    return (svalue_t *)-1;
  }

  if (master_applies[fun].func) {
    call_direct(master_ob, master_applies[fun].index, ORIGIN_DRIVER, num_arg);
    free_svalue(&apply_ret_value, "apply_master_ob");
    apply_ret_value = *sp--;
    return &apply_ret_value;
  } else {
    pop_n_elems(num_arg);
    return nullptr;
  }
}

/* Hmm, need something like a safe_call_direct() to do this one */
svalue_t *safe_apply_master_ob(int fun, int num_arg) {
  if (!master_ob) {
    pop_n_elems(num_arg);
    return (svalue_t *)-1;
  }
  return safe_apply(applies_table[fun], master_ob, num_arg, ORIGIN_DRIVER);
}

void init_master(const char *master_file) {
  char buf[512];
  object_t *new_ob;

  if (!filename_to_obname(master_file, buf, sizeof buf)) {
    error("Illegal master file name '%s'\n", master_file);
  }

  new_ob = load_object(buf, 1);
  if (new_ob == nullptr) {
    debug_message("The master file %s was not loaded.\n", master_file);
    exit(-1);
  }
  set_master(new_ob);
}

static void get_master_applies(object_t *ob) {
  int i;

  /* master_applies will be allocated if we're recompiling master_ob */
  if (master_applies) {
    FREE(master_applies);
  }
  master_applies = reinterpret_cast<function_lookup_info_t *>(DCALLOC(
      NUM_MASTER_APPLIES, sizeof(function_lookup_info_t), TAG_SIMULS, "get_master_applies"));

  for (i = 0; i < NUM_MASTER_APPLIES; i++) {
    const char *name = applies_table[i];
    int ind, ri;

    if (find_function_by_name(ob, name, &ind, &ri)) {
      master_applies[i].func = find_func_entry(ob->prog, ri);
      master_applies[i].index = ri;
    } else {
      master_applies[i].func = nullptr;
    }
  }
}

void set_master(object_t *ob) {
#if defined(PACKAGE_UIDS) || defined(PACKAGE_MUDLIB_STATS)
  int first_load = (!master_ob);
#endif
  svalue_t *ret;

  get_master_applies(ob);
  master_ob = ob;  // from here on apply_master_ob returns -1 only as return from the apply

  /* Make sure master_ob is never made a dangling pointer. */
  add_ref(master_ob, "set_master");
#ifndef PACKAGE_UIDS
#ifdef PACKAGE_MUDLIB_STATS
  if (first_load) {
    // 'master::domain_file' should return an apropriate "domain" for each
    // file (directory), as such we should get the backbone domain by
    // asking for 'master::domain_file("/")'
    push_constant_string("/");
    ret = apply_master_ob(APPLY_DOMAIN_FILE, 1);
    if (ret == 0 || ret->type != T_STRING) {
      // we didn't got the expected value?
      // emit warning and fall back to old behavior
      debug_message(
          "%s() in the master file does not work, using 'BACKBONE' as fallback (see %s.4)\n",
          applies_table[APPLY_DOMAIN_FILE], applies_table[APPLY_DOMAIN_FILE]);
      set_backbone_domain("BACKBONE");
    } else {
      set_backbone_domain(ret->u.string);
    }
    // 'master::author_file' should return an apropriate "author" for each
    // file (directory), as such we should get the master author by
    // asking for 'master::author_file(__MASTER_FILE__)'
    push_malloced_string(add_slash(CONFIG_STR(__MASTER_FILE__)));
    ret = apply_master_ob(APPLY_AUTHOR_FILE, 1);
    if (ret == 0 || ret->type != T_STRING) {
      // we didn't got the expected value?
      // emit warning and fall back to old behavior
      debug_message(
          "%s() in the master file does not work, using 'NONAME' as fallback (see %s.4)\n",
          applies_table[APPLY_AUTHOR_FILE], applies_table[APPLY_AUTHOR_FILE]);
      set_master_author("NONAME");
    } else {
      set_master_author(ret->u.string);
    }
  }
#endif
#else
  ret = apply_master_ob(APPLY_GET_ROOT_UID, 0);
  if (!ret) {
    debug_message(
        "No function %s() in master object; possibly the mudlib doesn't want "
        "PACKAGE_UIDS to be defined.\n",
        applies_table[APPLY_GET_ROOT_UID]);
    exit(-1);
  }
  if (ret->type != T_STRING) {
    debug_message("%s() in master object does not work.\n", applies_table[APPLY_GET_ROOT_UID]);
    exit(-1);
  }
  if (first_load) {
    master_ob->uid = set_root_uid(ret->u.string);
    master_ob->euid = master_ob->uid;
#ifdef PACKAGE_MUDLIB_STATS
    // 'master::author_file' should return an apropriate "author" for each
    // file (directory), as such we should get the master author by
    // asking for 'master::author_file(__MASTER_FILE__)'
    push_malloced_string(add_slash(CONFIG_STR(__MASTER_FILE__)));
    ret = apply_master_ob(APPLY_AUTHOR_FILE, 1);
    if (ret == nullptr || ret->type != T_STRING) {
      // we didn't got the expected value?
      // emit warning and fall back to old behavior
      debug_message(
          "%s() in the master file does not work, using root_uid as fallback (see %s.4)\n",
          applies_table[APPLY_AUTHOR_FILE], applies_table[APPLY_AUTHOR_FILE]);
      set_master_author(master_ob->uid->name);
    } else {
      set_master_author(ret->u.string);
    }
#endif
    ret = apply_master_ob(APPLY_GET_BACKBONE_UID, 0);
    if (ret == nullptr || ret->type != T_STRING) {
      debug_message("%s() in the master file does not work\n",
                    applies_table[APPLY_GET_BACKBONE_UID]);
      exit(-1);
    }
    set_backbone_uid(ret->u.string);
#ifdef PACKAGE_MUDLIB_STATS
    // 'master::domain_file' should return an apropriate "domain" for each
    // file (directory), as such we should get the backbone domain by
    // asking for 'master::domain_file("/")'
    push_constant_string("/");
    ret = apply_master_ob(APPLY_DOMAIN_FILE, 1);
    if (ret == nullptr || ret->type != T_STRING) {
      // we didn't got the expected value?
      // emit warning and fall back to old behavior
      debug_message("%s() in the master file does not work, using bb_ui as fallback (see %s.4)d\n",
                    applies_table[APPLY_DOMAIN_FILE], applies_table[APPLY_DOMAIN_FILE]);
      set_backbone_domain(backbone_uid->name);
    } else {
      set_backbone_domain(ret->u.string);
    }
#endif
  } else {
    master_ob->uid = add_uid(ret->u.string);
    master_ob->euid = master_ob->uid;
  }
#endif
}
