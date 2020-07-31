
#include "base/package_api.h"
#include "config.h"
#include "thirdparty/json/single_include/nlohmann/json.hpp"
#include <cstdint>
using json = nlohmann::json;
using value_t  = nlohmann::detail::value_t;

static array_t* decode_array(json);
static mapping_t* decode_object(json);
static svalue_t json_to_svalue(json);

constexpr int _max_depth = 10;
static json svalue_to_json(const svalue_t *obj, int depth) {
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
      return obj->u.string;
    }
    case T_CLASS:
      /* fall through */
    case T_ARRAY: {
      json res = json::array();
      for (int i = 0; i < obj->u.arr->size; i++) {
        res.push_back(svalue_to_json(&(obj->u.arr->item[i]), depth + 1));
      }
      return res;
    }
    case T_BUFFER: {
      json res = json::array();
      for (int i = 0; i < obj->u.buf->size; i++) {
        res.push_back((int)obj->u.buf->item[i]);
      }
      return res;
    }
    case T_MAPPING: {
      json res = json::object();
      for (int i = 0; i < obj->u.map->table_size; i++) {
        mapping_node_t *elm;
        for (elm = obj->u.map->table[i]; elm; elm = elm->next) {
          auto key = &(elm->values[0]);
          auto val = &(elm->values[1]);
          if (key->type == T_STRING) {
            res[key->u.string] = svalue_to_json(val, depth + 1);
          } else {
            res[std::to_string((intptr_t)key)] = svalue_to_json(val, depth + 1);
          }
        }
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
} /* end of svalue_to_json() */
void f_json_decode() {
  if (sp->type != T_STRING) {
    sp--;
    push_constant_string("只能输入字符串");
    return;
  }
  auto j = json::parse(sp->u.string);
  sp--;
  svalue_t sv = json_to_svalue(j);
  push_some_svalues(&sv,1);
  return;
}

svalue_t json_to_svalue(json j){
  debug_message("json type:%s\n",j.type_name());
  svalue_t sv;
  if(j.is_structured()){
    if (j.is_array()){
      sv.type=T_ARRAY;
      sv.u.arr = decode_array(j);
    } else if (j.is_object()){
      sv.type = T_MAPPING;
      sv.u.map = decode_object(j);
    }
  } else if(j.is_primitive()){
    if(j.is_null()){
      sv.type = T_UNDEFINED;
    } else if (j.is_string()){
      sv.type=T_STRING;
      sv.u.string =make_shared_string(j.get<std::string>().c_str());

    }else if (j.is_boolean()){
      sv.type = T_NUMBER;
      sv.u.number = (j.get<bool>() ? 1 : 0);

    }else if (j.is_number()){
      if (j.is_number_integer()){
        sv.type = T_NUMBER;
        sv.u.number = j.get<std::int64_t>();
      } else if(j.is_number_float()){
        sv.type=T_REAL;
        sv.u.real = j.get<double>();
      }
    // for discarded, null
    } else{
      sv.type = T_UNDEFINED;
    }
  }
  return sv;
}
array_t * decode_array(json j){
  array_t *arr = allocate_empty_array(j.size());
  if (j.size() < 1){
    return arr;
  }
  for(int i=0; i< j.size(); i++){
    svalue_t sv = json_to_svalue(j[i]);
    assign_svalue_no_free(&arr->item[i],&sv);
  }
  return arr;
}

mapping_t * decode_object(json j){
  mapping_t *m = allocate_mapping(0);
  if(j.size() < 1){
    return m;
  }
  svalue_t lv;
  svalue_t *rv;
  for(auto &el : j.items()){
        lv.type = T_STRING;
        lv.subtype = STRING_CONSTANT;
        lv.u.string = el.key().c_str();
        rv = find_for_insert(m,&lv,0);
        svalue_t r = json_to_svalue(el.value());
        assign_svalue(rv,&r);
  }
  return m;
}

void f_json_encode() {
  json j = svalue_to_json(sp, 0);
#ifdef DEBUG
  debug_message(j.dump(4).c_str());
#endif
  sp--;
  copy_and_push_string(j.dump().c_str());
  return;
}