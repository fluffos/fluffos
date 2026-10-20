#include "base/package_api.h"

#include "packages/core/replace_string.h"

// from add_action.h
extern object_t* find_living_object(const char*, int);

/* Hideous mangling of C code by Taffyd. */
vm::Value query_multiple_short(array_t* arr, const char* type, size_t type_len, int no_dollars,
                               int quiet, int dark) {
  auto const max_string_length = CONFIG_INT(__MAX_STRING_LENGTH__);
  char m[] = "$M$";
  char s[] = "_short";
  char default_function[] = "a_short";
  char separator[] = ", ";
  char andsep[] = " and ";
  int const mlen = static_cast<int>(sizeof(m) - 1);
  int const slen = static_cast<int>(sizeof(s) - 1);
  int const seplen = static_cast<int>(sizeof(separator) - 1);
  int const andlen = static_cast<int>(sizeof(andsep) - 1);

  svalue_t* sv;
  int const size = arr->size;
  int i;
  int len;
  int total_len;
  char *str, *res;
  object_t* ob;
  char* fun;

  if (!size) {
    str = new_string(0, "f_query_multiple_short");
    str[0] = '\0';
    return vm::Value::str_malloced(str);
  }

  /*
   if (no_dollars && sizeof(args) && objectp(args[0]) && undefinedp(dark) &&
   this_player() && environment(this_player())) {
   dark = this_player()->check_dark(environment(this_player())->query_light());
   if (dark) {
   return "some objects you cannot make out";
   }
   } */

  if (no_dollars && arr->item->type == T_OBJECT && !dark && command_giver && command_giver->super) {
    /* check_dark(query_light()), passing 0 when there is no query_light() */
    vm::CallResult light = vm::Call().apply("query_light", command_giver->super, ORIGIN_EFUN);
    vm::CallResult v = vm::Call(light.take()).apply("check_dark", command_giver, ORIGIN_EFUN);

    if (v.is(T_NUMBER) && v->u.number) {
      return vm::Value::str_copy("some objects you cannot make out");
    }
  }

  /* If we don't have a type parameter, then use default_function */
  /* We need to free this value with FREE_MSTR() */

  if (!type) {
    len = static_cast<int>(sizeof(default_function) - 1);
    fun = new_string(len, "f_query_multiple_short");
    fun[len] = '\0';
    memcpy(fun, default_function, static_cast<size_t>(len));
  } else {
    len = static_cast<int>(type_len) + slen;
    fun = new_string(len, "f_query_multiple_short");
    fun[len] = '\0';
    memcpy(fun, type, type_len);
    memcpy(fun + type_len, s, static_cast<size_t>(slen));
  }

  /* Check to see if there are any non-objects in the array. */
  for (i = 0; i < size; i++) {
    if ((arr->item + i)->type != T_OBJECT) {
      break;
    }
  }
  /* The array consists only of objects, and will use the $M$
   expansion code. */
  if (i == size && !no_dollars) {
    str = new_string(max_string_length, "f_query_multiple_short");
    str[max_string_length] = '\0';
    memcpy(str, m, static_cast<size_t>(mlen));
    total_len = mlen;

    for (i = 0; i < size; i++) {
      sv = (arr->item + i);
      vm::CallResult v = vm::Call(quiet).apply(fun, sv->u.ob, ORIGIN_EFUN);

      if (!v.is(T_STRING)) {
        continue;
      }
      if (total_len + SVALUE_STRLEN(v.get()) > max_string_length - mlen) {
        continue;
      }
      len = SVALUE_STRLEN(v.get());
      memcpy(str + total_len, v->u.string, static_cast<size_t>(len));
      total_len += len;
    }

    memcpy(str + total_len, m, static_cast<size_t>(mlen));
    total_len += mlen;

    res = new_string(total_len, "f_query_multiple_short");
    res[total_len] = '\0';
    memcpy(res, str, total_len);

    /* Clean up our temporary buffer. */

    FREE_MSTR(str);
    FREE_MSTR(fun);

    return vm::Value::str_malloced(res);
  }

  /* This is a mixed array, so we don't use $M$ format.  Instead, we
   do as much $a_short$ conversion as we can etc.  */

  str = new_string(max_string_length, "f_query_multiple_short");
  str[max_string_length] = '\0';
  total_len = 0;

  for (i = 0; i < size; i++) {
    sv = (arr->item + i);

    switch (sv->type) {
      case T_STRING:
        len = SVALUE_STRLEN(sv);
        if (total_len + len < max_string_length) {
          memcpy(str + total_len, sv->u.string, static_cast<size_t>(len));
          total_len += len;
        }
        break;
      case T_OBJECT: {
        vm::CallResult v = vm::Call(quiet).apply(fun, sv->u.ob, ORIGIN_EFUN);

        if (!v.is(T_STRING)) {
          continue;
        }

        if (total_len + SVALUE_STRLEN(v.get()) < max_string_length) {
          len = SVALUE_STRLEN(v.get());
          memcpy(str + total_len, v->u.string, static_cast<size_t>(len));
          total_len += len;
        }

        break;
      }
      case T_ARRAY:
      /* Does anyone use this? */
      /* args[ i ] = "$"+ type +"_short:"+ file_name( args[ i ][ 1 ] ) +"$"; */
      default:
        /* Get the next element. */
        continue;
        break;
    }

    if (len && size > 1) {
      if (i < size - 2) {
        if (total_len + seplen < max_string_length) {
          memcpy(str + total_len, separator, static_cast<size_t>(seplen));
          total_len += seplen;
        }
      } else {
        if (i < size - 1) {
          if (total_len + andlen < max_string_length) {
            memcpy(str + total_len, andsep, static_cast<size_t>(andlen));
            total_len += andlen;
          }
        }
      }
    }
  }

  FREE_MSTR(fun);

  res = new_string(total_len, "f_query_multiple_short");
  res[total_len] = '\0';
  memcpy(res, str, total_len);

  FREE_MSTR(str);

  /* Ok, now that we have cleaned up here we have to decide what to do
   with it. If nodollars is 0, then we need to pass it to an object
   for conversion. */

  if (no_dollars) {
    if (command_giver) {
      /* We need to call on this_player(). */
      vm::CallResult v = vm::Call(vm::Value::str_malloced(res))
                             .apply("convert_message", command_giver, ORIGIN_EFUN);

      if (v.is(T_STRING)) {
        return vm::Value::str_make_shared(v->u.string);
      }
      return vm::Value::undefined();
    }
    /* We need to find /global/player. */
    /* Does this work? Seems not to. */
    ob = find_object("/global/player");

    if (ob) {
      vm::CallResult v =
          vm::Call(vm::Value::str_malloced(res)).apply("convert_message", ob, ORIGIN_EFUN);

      /* Return the result! */
      if (v.is(T_STRING)) {
        return vm::Value::str_make_shared(v->u.string);
      }
      return vm::Value::undefined();
    }
    /* res was never handed off -- free it. */
    FREE_MSTR(res);
    return vm::Value::undefined();
  }
  return vm::Value::str_malloced(res);
} /* query_multiple_short() */

vm::Value f_query_multiple_short(vm::EfunArgs&, array_t* arr, std::optional<vm::Arg> type_arg,
                                 std::optional<LPC_INT> no_dollars_arg,
                                 std::optional<LPC_INT> quiet_arg,
                                 std::optional<LPC_INT> dark_arg) {
  const char* type = nullptr;
  size_t type_len = 0;
  int no_dollars = 0, quiet = 0, dark = 0;

  if (dark_arg) {
    dark = *dark_arg;
  }

  if (quiet_arg) {
    quiet = *quiet_arg;
  }

  if (no_dollars_arg) {
    no_dollars = *no_dollars_arg;
  }

  if (type_arg && type_arg->is(T_STRING)) {
    vm::StringArg ts = type_arg->string();
    type = ts.c_str();
    type_len = ts.size();
  }

  return query_multiple_short(arr, type, type_len, no_dollars, quiet, dark);
}

/* Hideous mangling of C code by Taffyd. */

/* varargs int reference_allowed(object referree, mixed referrer) */

#define PLAYTESTER_HANDLER "/obj/handlers/playtesters"
#define PLAYER_HANDLER "/obj/handlers/player_handler"

int _in_reference_allowed = 0;

int reference_allowed(object_t* referee, object_t* referrer_obj, const char* referrer_name) {
  int invis = 0;
  int referee_creator = 0;
  svalue_t* item;
  array_t* vec;
  const char* referee_name = nullptr;
  object_t* playtester_handler = nullptr;
  object_t* player_handler = nullptr;
  int referrer_playtester = 0;
  int referrer_match = 0;
  int playtester_match = 0;
  int i;
  int size;
  int ret = 0;

  /* Check to see whether we're invisible */
  vm::CallResult v = vm::Call().apply("query_invis", referee, ORIGIN_EFUN);

  if (v.is(T_NUMBER)) {
    invis = v->u.number;
  }
  /* And if we're a creator. */
  v = vm::Call().apply("query_creator", referee, ORIGIN_EFUN);

  if (v.is(T_NUMBER)) {
    referee_creator = v->u.number;
  }

  /* If we're not invisible, or not a creator, or currently in a
   reference allowed call, then we can see them. */
  if (!invis || !referee_creator || _in_reference_allowed) {
    return 1;
  }

  _in_reference_allowed = 1;

  if (referrer_obj && referee == referrer_obj) {
    _in_reference_allowed = 0;
    return 1;
  }

  /* Determine the names of these guys.
   * Each name's CallResult owns the string, so the names stay valid
   * across the applies below.
   */

  vm::CallResult referee_name_v = vm::Call().apply("query_name", referee, ORIGIN_EFUN);

  if (referee_name_v.is(T_STRING)) {
    referee_name = referee_name_v->u.string;
  }

  vm::CallResult referrer_name_v(vm::CallResult::kNotFound);
  if (referrer_obj && !referrer_name) {
    referrer_name_v = vm::Call().apply("query_name", referrer_obj, ORIGIN_EFUN);

    if (referrer_name_v.is(T_STRING)) {
      referrer_name = referrer_name_v->u.string;
    }
  }

  if (!referee_name || !referrer_name) {
    _in_reference_allowed = 0;
    return 1;
  }

  playtester_handler = find_object(PLAYTESTER_HANDLER);

  if (playtester_handler) {
    v = vm::Call(vm::Value::str_copy(referrer_name))
            .apply("query_playtester", playtester_handler, ORIGIN_EFUN);

    if (v.is(T_NUMBER)) {
      referrer_playtester = v->u.number;
    }
  }

  v = vm::Call().apply("query_allowed", referee, ORIGIN_EFUN);
  // printf("allow refs:%d\n", v->u.arr->ref);
  if (v.is(T_ARRAY)) {
    vec = v->u.arr;
    size = vec->size;
    /* Iterate through the allowed array. */
    for (i = 0; i < size; i++) {
      item = vec->item + i;

      if (strcmp(referrer_name, item->u.string) == 0) {
        referrer_match = 1;

        /* If they're not a playtester, then bail as soon as
         * we make this match. */
        if (!referrer_playtester) {
          break;
        }
      }

      if (referrer_playtester && strcmp("playtesters", item->u.string) == 0) {
        playtester_match = 1;

        /* If we've already made a referrer match, then time
         * to bail now. */
        if (referrer_match) {
          break;
        }
      }
    }
  }

  /* If we found a match, then they are allowed so bail. */
  if (referrer_match || playtester_match) {
    _in_reference_allowed = 0;
    return 1;
  }

  switch (invis) {
    case 3: {
      /* Check for High Lord Invis. */

      v = vm::Call(vm::Value::str_copy(referrer_name))
              .apply("high_programmer", master_ob, ORIGIN_EFUN);

      if (v.is(T_NUMBER)) {
        ret = v->u.number;
      }
    } break;

    case 2: {
      /* Check for Lord Invis */

      v = vm::Call(vm::Value::str_copy(referrer_name)).apply("query_lord", master_ob, ORIGIN_EFUN);

      if (v.is(T_NUMBER)) {
        ret = v->u.number;
      }
    } break;

    case 1: {
      /* Creator Invis */
      if (referrer_obj) {
        v = vm::Call().apply("query_creator", referrer_obj, ORIGIN_EFUN);

        if (v.is(T_NUMBER)) {
          ret = v->u.number;
        }
      } else {
        player_handler = find_object(PLAYER_HANDLER);
        v = vm::Call(vm::Value::str_copy(referrer_name))
                .apply("test_creator", player_handler, ORIGIN_EFUN);

        if (v.is(T_NUMBER)) {
          ret = v->u.number;
        }
      }
    } break;

    default:
      /* A normal player.
       * Shouldn't we do this somewhere else? */

      ret = 1;
      break;
  }

  _in_reference_allowed = 0;
  return ret;
}

vm::Value f_reference_allowed(vm::EfunArgs&, object_t* referee, std::optional<vm::Arg> referrer) {
  object_t* referrer_obj = command_giver; /* Default to this_player(). */
  const char* referrer_name = nullptr;
  int result = 0;
#ifndef F_FIND_PLAYER
  /* find_player()'s result; holds the object it returned for the call */
  vm::CallResult player(vm::CallResult::kNotFound);
#endif

  if (referrer_obj && (referrer_obj->flags & O_DESTRUCTED)) {
    referrer_obj = nullptr;
  }

  if (referrer) {
    if (referrer->is(T_STRING)) {
/* We've been passed in a string, now we need to call
 * find_player() */
#ifdef F_FIND_PLAYER
      /* If we have a find_player() efun, then we need to sue
       * the following method.  This hasn't been tested!
       */
      referrer_obj = find_living_object(referrer->string().c_str(), 1);
#else
      if (simul_efun_ob) {
        player = vm::Call(referrer->copy()).apply("find_player", simul_efun_ob, ORIGIN_EFUN);

        if (player.is(T_OBJECT)) {
          referrer_obj = player->u.ob;
          referrer_name = referrer->string().c_str();
        } else {
          referrer_obj = NULL;
          referrer_name = referrer->string().c_str();
        }
      }
#endif
    }
    if (referrer->is(T_OBJECT)) {
      referrer_obj = referrer->object();
      referrer_name = nullptr;
    }
  }

  if (referee && (referrer_obj || referrer_name)) {
    result = reference_allowed(referee, referrer_obj, referrer_name);
    return result;
  }
  return vm::Value::undefined();
}

vm::Value f_add_a(vm::EfunArgs& f, vm::StringArg str_arg) {
  auto const max_string_length = CONFIG_INT(__MAX_STRING_LENGTH__);
  const char* const base = str_arg.c_str();
  size_t const total = str_arg.size();
  const char* str = base;
  char* ret;
  char* p;
  char first;
  int len;
  int an;

  while (static_cast<size_t>(str - base) < total && *str == ' ') {
    str++;
  }

  // If *str is 0, it was only spaces.  Return "a ".
  if (static_cast<size_t>(str - base) >= total || *str == 0) {
    return vm::Value::str_copy("a ");
  }

  len = static_cast<int>(total - static_cast<size_t>(str - base));
  // Don't add anything if it already begins with a or an.
  if (!strncasecmp(str, "a ", 2) || !strncasecmp(str, "an ", 3)) {
    return f.take(0);
  }

  first = *str;
  an = 0;

  // Some special cases.
  // If it begins with "us", check the following letter.
  // "a use", "a usurper", "a user", but "an usher".
  if (!strncasecmp(str, "us", 2)) {
    first = str[2];
    an = 1;
  }

  // "hour*" gets "an".
  if (!strncasecmp(str, "hour", 4)) {
    first = 'o';
  }

  switch (first) {
    case 'a':
    case 'e':
    case 'i':
    case 'o':
    case 'u':
    case 'A':
    case 'E':
    case 'I':
    case 'O':
    case 'U':
      an = !an;
      break;
    default:
      break;
  }

  if (an) {  // Add an.
    if (len + 3 > max_string_length) {
      error("add_a() exceeded max string length.\n");
    }
    ret = new_string(len + 3, "f_add_a");
    memcpy(ret, "an ", 3);
    p = ret + 3;
  } else {  // Add a.
    if (len + 2 > max_string_length) {
      error("add_a() exceeded max string length.\n");
    }
    ret = new_string(len + 2, "f_add_a");
    memcpy(ret, "a ", 2);
    p = ret + 2;
  }

  // Add the rest of the string.
  memcpy(p, str, static_cast<size_t>(len));
  p[len] = '\0';
  return vm::Value::str_malloced(ret);
}

// This along with add_a() is the only sfun in /secure/simul_efun/add_a.c
vm::Value f_vowel(vm::EfunArgs&, LPC_INT c) {
  char const v = (char)c;

  if (v == 'a' || v == 'e' || v == 'i' || v == 'o' || v == 'u' || v == 'A' || v == 'E' ||
      v == 'I' || v == 'O' || v == 'U') {
    return 1;
  }
  return 0;
}

/* Both forms run replace_string()'s body directly. It reads only the
 * window's arity (2 or 3 here, so no range) and its typed strings. */
vm::Value f_replace(vm::EfunArgs& f, vm::StringArg /* moved out with f.take(0) */, vm::Arg pattern,
                    std::optional<vm::StringArg> replacement) {
  int num_arg = f.argc();
  if (pattern.is(T_STRING) && num_arg == 3) {
    return replace_string_value(f.take(0), pattern.string(), *replacement);
  }
  if (num_arg == 2 && !pattern.is(T_STRING)) {
    array_t* arr = pattern.array();
    if (arr->size & 1) {
      error("Wrong array size for replace.\n");
    }
    /* Apply each [pattern, replacement] pair in turn to the result so far
     * (owned, so a string nothing else references is edited in place). */
    vm::Value cur = f.take(0);
    for (int i = 0; i < arr->size; i += 2) {
      if (arr->item[i].type == T_STRING && arr->item[i + 1].type == T_STRING) {
        cur = replace_string_value(std::move(cur), vm::StringArg(&arr->item[i]),
                                   vm::StringArg(&arr->item[i + 1]));
        if (cur.type() != T_STRING) {
          break;
        }
      }
    }
    return cur;
  }
  error("Bad arguments for replace.\n");
}

static vm::Value replace_mxp_html(vm::StringArg str, int html, int mxp) {
  auto const max_string_length = CONFIG_INT(__MAX_STRING_LENGTH__);

  char* dst = new_string(max_string_length + 8, "f_replace_mxp_html: 2");
  const char* src = str.c_str();
  const char* const src_end = src + str.size();
  char* dst2 = dst;
  while (src < src_end && dst2 - dst < max_string_length) {
    switch (*src) {
      case '&':
        memcpy(dst2, "&amp;", 5);
        dst2 += 5;
        break;
      case '<':
        memcpy(dst2, "&lt;", 4);
        dst2 += 4;
        break;
      case '>':
        memcpy(dst2, "&gt;", 4);
        dst2 += 4;
        break;
      case '\n':
        if (mxp) {
          memcpy(dst2, "\e[4z<BR>", 8);
          dst2 += 8;
        } else {
          goto def;
        }
        break;
      case '"':
        if (html) {
          memcpy(dst2, "&quot;", 6);
          dst2 += 6;
          break;
        }
        [[fallthrough]];
      default:
      def:
        *dst2++ = *src;
    }
    src++;
  }
  *dst2 = 0;
  return vm::Value::str_malloced(extend_string(dst, dst2 - dst));
}

vm::Value f_replace_html(vm::EfunArgs&, vm::StringArg str) { return replace_mxp_html(str, 1, 0); }

vm::Value f_replace_mxp(vm::EfunArgs&, vm::StringArg str) { return replace_mxp_html(str, 0, 1); }

vm::Value f_roulette_wheel(vm::EfunArgs&, mapping_t* m) {
  long num;
  int j, found;
  mapping_node_t *elt, **a = m->table;
  svalue_t* val;

  // Loop through the mapping, adding up the weights.
  j = m->table_size;
  if (!j || !MAP_COUNT(m)) {
    error("empty mapping in roulette_wheel.\n");
  }
  num = 0;
  do {
    for (elt = a[j]; elt; elt = elt->next) {
      val = elt->values + 1;

      if (val->type != T_NUMBER || val->u.number < 0) {
        error("Weights must be non-negative integers.\n");
      }

      num += val->u.number;
    }
  } while (j--);

  num = 1 + random_number(num);
  found = 0;

  // Loop again, and stop when the sum of the weights comes to num.
  j = m->table_size;
  do {
    for (elt = a[j]; elt; elt = elt->next) {
      val = elt->values + 1;
      num -= val->u.number;

      if (num <= 0) {
        found = 1;
        break;
      }
    }
  } while (j-- && !found);

  if (!found) {  // This shouldn't happen...
    error("Something went wrong!\n");
  }

  // val points to the value that tipped the scale.  val - 1 is the key.
  return vm::Value::copy(val - 1);
}

// Appends up to (bufsize - 1 - *len) bytes of `text` to buf[*len..], always
// NUL-terminating and never writing past bufsize. `text` need not be
// NUL-terminated at `textlen` (used for LPC strings, which may embed NULs).
static void append_bounded(char* buf, size_t bufsize, size_t* len, const char* text,
                            size_t textlen) {
  size_t avail = (*len < bufsize) ? bufsize - 1 - *len : 0;
  if (textlen > avail) {
    textlen = avail;
  }
  memcpy(buf + *len, text, textlen);
  *len += textlen;
  buf[*len] = '\0';
}

svalue_t replace_tmp = {T_NUMBER};
svalue_t* replace_objects(svalue_t* thing) {
  int i;
  switch (thing->type) {
    case T_OBJECT: {
      // thing->u.ob->obname and, especially, the master's object_name()
      // apply below are arbitrary-length and mudlib/master-controlled --
      // bound every copy into this fixed buffer instead of strcpy/strcat.
      char buf[2000];
      size_t len = 0;
      append_bounded(buf, sizeof(buf), &len, thing->u.ob->obname,
                     strnlen(thing->u.ob->obname, sizeof(buf) - 1));
      vm::CallResult tmp(vm::CallResult::kNotFound);
      if (!(thing->u.ob->flags & O_DESTRUCTED)) {
        tmp = vm::Call(vm::Value::object(thing->u.ob)).safe_apply_master(APPLY_OBJECT_NAME);
      } else {
        static constexpr char kDestructed[] = " (destructed)";
        append_bounded(buf, sizeof(buf), &len, kDestructed, sizeof(kDestructed) - 1);
      }
      if (tmp.is(T_STRING)) {
        append_bounded(buf, sizeof(buf), &len, " (\"", 3);
        append_bounded(buf, sizeof(buf), &len, tmp->u.string, SVALUE_STRLEN(tmp.get()));
        append_bounded(buf, sizeof(buf), &len, "\")", 2);
      }
      free_svalue(&replace_tmp, "replace_objects");
      replace_tmp.type = T_STRING;
      replace_tmp.subtype = STRING_MALLOC;
      replace_tmp.u.string = string_copy(buf, "replace_objects");
      return &replace_tmp;
    }
    case T_ARRAY:
    case T_CLASS: {
      array_t* ar = thing->u.arr;
      vm::Value nar_owner = vm::Value::refed(allocate_array(ar->size)); /* freed if we error */
      array_t* nar = nar_owner->u.arr;
      for (i = 0; i < ar->size; i++) {
        assign_svalue(&nar->item[i], replace_objects(&ar->item[i]));
      }
      assign_svalue(&replace_tmp, nar_owner.get());
      replace_tmp.type = thing->type;
      return &replace_tmp;
    }
    case T_MAPPING: {
      /* keys, result and key are owned here, so an error frees them */
      vm::Value keys_owner = vm::Value::refed(mapping_indices(thing->u.map));
      vm::Value out_owner = vm::Value::refed(allocate_mapping(thing->u.map->table_size));
      vm::Value key_owner;
      array_t* keys = keys_owner->u.arr;
      mapping_t* out = out_owner->u.map;
      for (i = 0; i < keys->size; i++) {
        svalue_t* key = key_owner.get();
        svalue_t* tmp = find_in_mapping(thing->u.map, keys->item[i]);
        assign_svalue(key, replace_objects(&keys->item[i]));
        assign_svalue_no_free(find_for_insert(out, key, 1), replace_objects(tmp));
        if (out->count - i != 1) {
          printf("guilty party:%s\n", key->u.string);
        }
      }
      assign_svalue(&replace_tmp, out_owner.get());
      return &replace_tmp;
    }
    default:
      assign_svalue(&replace_tmp, thing);
      return &replace_tmp;
  }
}

vm::Value f_replace_objects(vm::EfunArgs&, vm::Arg thing) {
  // replace_tmp is a module-global scratch slot: move the result out of it
  // (leaving 0) so no reference lingers there between calls (flagged by the
  // Debug leak checker).
  return vm::Value::take(replace_objects(thing.sv()));
}

vm::Value f_replace_dollars(vm::EfunArgs& f, vm::StringArg str, array_t* arr) {
  auto const max_string_length = CONFIG_INT(__MAX_STRING_LENGTH__);

  char* newstr;
  const char* oldstr = str.c_str();
  char* currentnew;
  const char* currentold = oldstr;
  int i;
  if (arr->size & 1) {
    error("wrong array length for replace_dollars()");
  }
  for (i = 0; i < arr->size; i++)
    if (arr->item[i].type != T_STRING) {
      error("replace array should only contain strings");
    }
  newstr = new_string(max_string_length, "replace_dollars");
  currentnew = newstr;
  for (i = 0; i < COUNTED_STRLEN(oldstr); i++) {
    if (oldstr[i] == '$') {
      int j;
      for (j = 0; j < arr->size; j += 2) {
        const char* one = arr->item[j].u.string;
        if (!strncmp(one, oldstr + i, COUNTED_STRLEN(one))) {
          const char* two = arr->item[j + 1].u.string;
          int const len = i - (currentold - oldstr);
          if (currentnew + len - newstr > max_string_length) {
            FREE_MSTR(newstr);
            error("string too long");
          }
          memcpy(currentnew, currentold, static_cast<size_t>(len));
          currentnew += len;
          currentold += len;
          currentold += COUNTED_STRLEN(one);
          // Check against the replacement length actually written below
          // ('two'), not the matched pattern ('one'): a replacement longer
          // than its pattern would otherwise overrun newstr.
          if (currentnew + COUNTED_STRLEN(two) - newstr > max_string_length) {
            FREE_MSTR(newstr);
            error("string too long");
          }
          {
            size_t const twolen = COUNTED_STRLEN(two);
            memcpy(currentnew, two, twolen);
            currentnew += twolen;
          }
          break;
        }
      }
    }
  }
  if (newstr == currentnew) {
    // nothing happened!
    FREE_MSTR(newstr);
    return f.take(0);
  }

  {
    size_t const rest = static_cast<size_t>(COUNTED_STRLEN(oldstr) - (currentold - oldstr));
    if (currentnew + rest - newstr > static_cast<size_t>(max_string_length)) {
      FREE_MSTR(newstr);
      error("string too long");
    }

    memcpy(currentnew, currentold, rest);
    currentnew[rest] = '\0';
    return vm::Value::str_malloced(extend_string(newstr, currentnew - newstr + rest));
  }
}
