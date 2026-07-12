#include "base/std.h"

#include "vm/vm.h"
#include "vm/internal/base/machine.h"
#include "compiler/internal/compiler.h"
#include "compiler/internal/lexer.h"
#include "compiler/internal/scratchpad.h"
#include "compiler/internal/generate.h"
#include "compiler/internal/grammar_rules.h"

#include <fmt/format.h>

extern int context;
extern int func_present;
extern int num_refs;

ScratchString* rule_identifier_defined_name(ident_hash_elem_t* ihe) {
  return scratch_new_string(ihe->name);
}

parse_node_t* rule_modifier_change(LPC_INT modifiers) {
  if (!modifiers) {
    yyerror("modifier list may not be empty.");
  }
  if (modifiers & FUNC_VARARGS) {
    yyerror("Illegal modifier 'varargs' in global modifier list.");
    modifiers &= ~FUNC_VARARGS;
  }
  if (!(modifiers & DECL_ACCESS)) {
    modifiers |= DECL_PUBLIC;
  }
  global_modifiers = modifiers;
  return nullptr;
}

void rule_member_name(LPC_INT star_modifier, const ScratchString* identifier) {
  if (current_type == TYPE_VOID) {
    yyerror("Illegal to declare class member of type void.");
  }
  add_local_name(identifier, current_type | star_modifier);
}

void rule_member_list_set_type(LPC_INT basic_type) { current_type = basic_type; }

parse_node_t* rule_default_arg_value(parse_node_t* expr) {
  if (current_function_context->num_locals) {
    yyerror("Illegal to use local variable in functional.");
  }
  if (current_function_context->values_list->r.expr) {
    current_function_context->values_list->r.expr->kind =
        current_function_context->values_list->kind;
  }

  parse_node_t* node = new_node();
  node->kind = NODE_FUNCTION_CONSTRUCTOR;
  node->type = TYPE_FUNCTION;
  node->l.expr = expr;
  node->r.expr = nullptr;  // no arguments
  node->v.number = FP_FUNCTIONAL + 0 /* args */;
  pop_function_context();
  return node;
}

LPC_INT rule_type_modifier_list(LPC_INT modifier, LPC_INT list) {
  LPC_INT res = modifier | list;
  int acc_mod = res & DECL_ACCESS;
#ifdef SENSIBLE_MODIFIERS
  if (acc_mod & (acc_mod - 1)) {
    char buf[256];
    char* end = EndOf(buf);
    get_type_modifiers(buf, end, acc_mod);
    yyerror("Multiple access modifiers (%s)", buf);
    res = DECL_PUBLIC;
  }
#endif
  return res;
}

LPC_INT rule_type(LPC_INT modifiers, LPC_INT basic_type) {
  LPC_INT res = (modifiers << 16) | basic_type;
  current_type = res;
  return res;
}

LPC_INT rule_atomic_type_class(ident_hash_elem_t* ihe) {
  if (ihe->dn.class_num == -1) {
    yyerror("Undefined class '%s'", ihe->name);
    return TYPE_ANY;
  } else {
    return ihe->dn.class_num | TYPE_MOD_CLASS;
  }
}

LPC_INT rule_atomic_type_class_identifier(const ScratchString* identifier) {
  yyerror("Undefined class '%s'", identifier->c_str());
  return TYPE_ANY;
}

LPC_INT rule_param_decl_typed(LPC_INT type_star) {
  if (type_star != TYPE_VOID) {
    add_local_name("", type_star);
  }
  return type_star;
}

LPC_INT rule_param_decl_typed_name(LPC_INT type_star, const ScratchString* name,
                                   parse_node_t* default_val) {
  if (type_star == TYPE_VOID) {
    yyerror("Illegal to declare argument of type void.");
  }
  add_local_name(name, type_star, default_val);
  return type_star;
}

LPC_INT rule_param_decl_untyped_name(const ScratchString* name) {
  if (exact_types) {
    yyerror("Missing type for argument");
  }
  add_local_name(name, TYPE_ANY);
  return TYPE_ANY;
}

void rule_argument_varargs(argument_t* result, argument_t* arg_list) {
  *result = *arg_list;
  result->flags |= ARG_IS_VARARGS;

  // '...' needs a preceding named parameter to hold the remaining args; with
  // none (e.g. `foo(void ...)`) max_num_locals is 0 and the type lookup below
  // would read type_of_locals_ptr[-1].
  if (max_num_locals <= 0) {
    yyerror("'...' requires a preceding parameter to hold the remaining arguments.");
    return;
  }

  int x = type_of_locals_ptr[max_num_locals - 1];
  int lt = x & ~LOCAL_MODS;

  if (x & LOCAL_MOD_REF) {
    yyerror("Variable to hold remainder of args may not be a reference");
  }
  if (lt != TYPE_ANY && !(lt & TYPE_MOD_ARRAY)) {
    yywarn("Variable to hold remainder of arguments should be an array.");
  }
}

void rule_argument_list_single(argument_t* result, LPC_INT new_arg_val) {
  if ((new_arg_val & TYPE_MASK) == TYPE_VOID && !(new_arg_val & TYPE_MOD_CLASS)) {
    if (new_arg_val & ~TYPE_MASK) {
      yyerror("Illegal to declare argument of type void.");
    }
    result->num_arg = 0;
  } else {
    result->num_arg = 1;
  }
  result->flags = 0;
}

void rule_argument_list_multi(argument_t* result, argument_t* list, LPC_INT new_arg_val) {
  if (!list->num_arg) {
    yyerror("argument of type void must be the only argument.");
  }
  if ((new_arg_val & TYPE_MASK) == TYPE_VOID && !(new_arg_val & TYPE_MOD_CLASS)) {
    yyerror("Illegal to declare argument of type void.");
  }

  *result = *list;
  result->num_arg++;
}

LPC_INT rule_cast(LPC_INT basic_type, LPC_INT optional_star) { return basic_type | optional_star; }

LPC_INT rule_opt_basic_type_empty() { return TYPE_UNKNOWN; }

void rule_single_new_local_def(LPC_INT* result, LPC_INT type, const ScratchString* name) {
  if (type == TYPE_VOID) {
    yyerror("Illegal to declare local variable of type void.");
  }
  *result = add_local_name(name, type);
}
