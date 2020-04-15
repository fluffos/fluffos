#include "base/std.h"

#include "vm/internal/base/machine.h"

#include "thirdparty/json/single_include/nlohmann/json.hpp"
using json = nlohmann::json;

// FIXME: move init from main() to compile time;
svalue_t const0{T_NUMBER, 0, {0}};
svalue_t const1{T_NUMBER, 0, {1}};
svalue_t const0u{T_NUMBER, T_UNDEFINED, {0}};

/*
 * Assign to a svalue.
 * This is done either when element in array, or when to an identifier
 * (as all identifiers are kept in a array pointed to by the object).
 */

void assign_svalue_no_free(svalue_t *to, svalue_t *from) {
  DEBUG_CHECK(from == 0, "Attempt to assign_svalue() from a null ptr.\n");
  DEBUG_CHECK(to == 0, "Attempt to assign_svalue() to a null ptr.\n");
  DEBUG_CHECK((from->type & (from->type - 1)) & ~T_FREED, "from->type is corrupt; >1 bit set.\n");

  if (from->type == T_OBJECT && (!from->u.ob || (from->u.ob->flags & O_DESTRUCTED))) {
    *to = const0u;
    return;
  }

  *to = *from;

  if ((to->type & T_FREED) && to->type != T_FREED) {
    to->type &= ~T_FREED;
  }

  if (from->type == T_STRING) {
    if (from->subtype & STRING_COUNTED) {
      INC_COUNTED_REF(to->u.string);
      ADD_STRING(MSTR_SIZE(to->u.string));
      NDBG(BLOCK(to->u.string));
    }
  } else if (from->type & T_REFED) {
    if (from->type == T_OBJECT) {
      add_ref(from->u.ob, "assign_svalue_no_free");
    } else {
      from->u.refed->ref++;
    }
  }
}

void assign_svalue(svalue_t *dest, svalue_t *v) {
  /* First deallocate the previous value. */
  free_svalue(dest, "assign_svalue");
  assign_svalue_no_free(dest, v);
}

/*
 * Copies an array of svalues to another location, which should be
 * free space.
 */
void copy_some_svalues(svalue_t *dest, svalue_t *v, int num) {
  while (num--) {
    assign_svalue_no_free(dest + num, v + num);
  }
}

/*
 * Free the data that an svalue is pointing to. Not the svalue
 * itself.
 * Use the free_svalue() define to call this
 */
#ifdef DEBUG
void int_free_svalue(svalue_t *v, const char *tag)
#else
void int_free_svalue(svalue_t *v)
#endif
{
  if (v->type == T_STRING) {
    const char *str = v->u.string;

    if (v->subtype & STRING_COUNTED) {
      int size = MSTR_SIZE(str);
      if (DEC_COUNTED_REF(str)) {
        SUB_STRING(size);
        NDBG(BLOCK(str));
        if (v->subtype & STRING_HASHED) {
          SUB_NEW_STRING(size, sizeof(block_t));
          deallocate_string(const_cast<char *>(str));
          CHECK_STRING_STATS;
        } else {
          SUB_NEW_STRING(size, sizeof(malloc_block_t));
          FREE(MSTR_BLOCK(str));
          CHECK_STRING_STATS;
        }
      } else {
        SUB_STRING(size);
        NDBG(BLOCK(str));
      }
    }
  } else if ((v->type & T_REFED) && !(v->type & T_FREED)) {
#ifdef DEBUG
    if (v->type == T_OBJECT) {
      debug(d_flag, "Free_svalue %s (%d) from %s\n", v->u.ob->obname, v->u.ob->ref - 1, tag);
    }
#endif
    /* TODO: Set to 0 on condition that REF overflow to negative. */
    if (v->u.refed->ref > 0) {
      v->u.refed->ref--;
    }
    if (v->u.refed->ref == 0) {
      switch (v->type) {
        case T_OBJECT:
          dealloc_object(v->u.ob, "free_svalue");
          break;
        case T_CLASS:
          dealloc_class(v->u.arr);
          break;
        case T_ARRAY:
          if (v->u.arr != &the_null_array) {
            dealloc_array(v->u.arr);
          }
          break;
        case T_BUFFER:
          if (v->u.buf != &null_buf) {
            FREE((char *)v->u.buf);
          }
          break;
        case T_MAPPING:
          dealloc_mapping(v->u.map);
          break;
        case T_FUNCTION:
          dealloc_funp(v->u.fp);
          break;
        case T_REF:
          if (!v->u.ref->lvalue) {
            kill_ref(v->u.ref);
          }
          break;
      }
    }
  } else if (v->type == T_ERROR_HANDLER) {
    (*v->u.error_handler)();
  }
#ifdef DEBUG
  else if (v->type == T_FREED) {
    fatal("T_FREED svalue freed.  Previously freed by %s.\n", v->u.string);
  }
  v->type = T_FREED;
  v->u.string = tag;
#endif
}

/*
 * Converts any LPC datatype into json format, only value types are supported.
 */
constexpr int _max_depth = 10;
json svalue_to_json_summary(const svalue_t *obj, int depth) {
  /* prevent an infinite recursion on self-referential structures */
  if (depth >= _max_depth) {
    return "truncated";
  }
  switch ((obj->type & ~T_FREED)) {
    case T_INVALID:
      return "T_INVALID";
    case T_LVALUE:
      return {{"lvalue", (intptr_t)obj->u.lvalue}};
    case T_REF:
      return {{"ref", (intptr_t)obj->u.ref->lvalue}};
    case T_FUNCTION:
      return "function";
    case T_NUMBER:
      return obj->u.number;
    case T_REAL:
      return obj->u.real;
    case T_STRING: {
      auto len = SVALUE_STRLEN(obj);
      if (len < 32) {
        return obj->u.string;
      }
      return std::string(obj->u.string,
                         u8_truncate(reinterpret_cast<const uint8_t *>(obj->u.string), 32)) +
             "...(len:" + std::to_string(len) + ")";
    }
    case T_CLASS:
      /* fall through */
    case T_ARRAY: {
      json res = json::array();
      for (int i = 0; i < std::min(3, obj->u.arr->size); i++) {
        res.push_back(svalue_to_json_summary(&(obj->u.arr->item[i]), depth + 1));
      }
      if (obj->u.arr->size > 10) {
        res.push_back("...(len:" + std::to_string(obj->u.arr->size) + ")");
      }
      return res;
    }
    case T_BUFFER: {
      json res = json::array();
      for (int i = 0; i < std::min(10u, obj->u.buf->size); i++) {
        res.push_back((int)obj->u.buf->item[i]);
      }
      if (obj->u.buf->size > 10) {
        res.push_back("...(len:" + std::to_string(obj->u.buf->size) + ")");
      }
      return res;
    }
    case T_MAPPING: {
      json res = json::object();
      auto limit = std::min(5u, obj->u.map->count);
      for (int i = 0; i < obj->u.map->table_size; i++) {
        mapping_node_t *elm;
        for (elm = obj->u.map->table[i]; elm; elm = elm->next) {
          auto key = &(elm->values[0]);
          auto val = &(elm->values[1]);
          if (key->type == T_STRING) {
            res[key->u.string] = svalue_to_json_summary(val, depth + 1);
          } else {
            res[std::to_string((intptr_t)key)] = svalue_to_json_summary(val, depth + 1);
          }
          if (limit-- == 0) {
            break;
          }
        }
      }
      if (obj->u.map->count > 4) {
        res["_sizeof"] = std::to_string(obj->u.map->count);
      }
      return res;
    }
    case T_OBJECT: {
      if (obj->u.ob->flags & O_DESTRUCTED) {
        return {0};
      }
      return std::string("/") + obj->u.ob->obname;
    }
    default: {
      return "unknown svalue";
    }
  }
} /* end of svalue_to_json_summary() */
