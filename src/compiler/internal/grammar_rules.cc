#include "base/std.h"

#include "vm/vm.h"
#include "vm/internal/base/machine.h"
#include "compiler/internal/compiler.h"
#include "compiler/internal/lex.h"
#include "compiler/internal/scratchpad.h"
#include "compiler/internal/generate.h"
#include "compiler/internal/grammar_rules.h"

#include <fmt/format.h>

extern int context;       // FIXME
extern int func_present;  // FIXME

void rule_program(parse_node_t *$$) { comp_trees[TREE_MAIN] = $$; }

bool rule_inheritence(parse_node_t **$$, int $1, char *$3) {
  object_t *ob;
  inherit_t inherit;
  int initializer;
#ifdef SENSIBLE_MODIFIERS
  int acc_mod;
#endif

  $1 |= global_modifiers;

#ifdef SENSIBLE_MODIFIERS
  acc_mod = ($1 & DECL_ACCESS) & ~global_modifiers;
  if (acc_mod & (acc_mod - 1)) {
    char buf[256];
    char *end = EndOf(buf);
    char *p;

    p = strput(buf, end, "Multiple access modifiers (");
    p = get_type_modifiers(p, end, acc_mod);
    p = strput(p, end, ") for inheritance");
    yyerror(buf);
  }
#endif

  if (!($1 & DECL_ACCESS)) $1 |= DECL_PUBLIC;
  if (var_defined) {
    yyerror("Illegal to inherit after defining global variables.");
    inherit_file = 0;
    return true;
  }
  ob = find_object2($3);
  if (ob == 0) {
    inherit_file = alloc_cstring($3, "inherit");
    /* Return back to load_object() */
    return true;
  }
  scratch_free($3);
  inherit.prog = ob->prog;

  if (mem_block[A_INHERITS].current_size) {
    inherit_t *prev_inherit = INHERIT(NUM_INHERITS - 1);

    inherit.function_index_offset = prev_inherit->function_index_offset +
                                    prev_inherit->prog->num_functions_defined +
                                    prev_inherit->prog->last_inherited;
    if (prev_inherit->prog->num_functions_defined &&
        prev_inherit->prog->function_table[prev_inherit->prog->num_functions_defined - 1]
                .funcname[0] == APPLY___INIT_SPECIAL_CHAR)
      inherit.function_index_offset--;
  } else
    inherit.function_index_offset = 0;

  inherit.variable_index_offset = mem_block[A_VAR_TEMP].current_size / sizeof(variable_t);
  inherit.type_mod = $1;
  add_to_mem_block(A_INHERITS, (char *)&inherit, sizeof inherit);

  /* The following has to come before copy_vars - Sym */
  copy_structures(ob->prog);
  copy_variables(ob->prog, $1);
  initializer = copy_functions(ob->prog, $1);
  if (initializer >= 0) {
    parse_node_t *node, *newnode;
    /* initializer is an index into the object we're
       inheriting's function table; this finds the
       appropriate entry in our table and generates
       a call to it */
    node = new_node_no_line();
    node->kind = NODE_CALL_2;
    node->r.expr = 0;
    node->v.number = F_CALL_INHERITED;
    node->l.number = initializer | ((NUM_INHERITS - 1) << 16);
    node->type = TYPE_ANY;

    /* The following illustrates a distinction between */
    /* macros and funcs...newnode is needed here - Sym */
    newnode = comp_trees[TREE_INIT];
    CREATE_TWO_VALUES(comp_trees[TREE_INIT], 0, newnode, node);
    comp_trees[TREE_INIT] = pop_value(comp_trees[TREE_INIT]);
  }
  *$$ = 0;

  return false;
}

LPC_INT rule_func_type(LPC_INT type, LPC_INT optional_star, char *identifier) {
  int flags;
#ifdef SENSIBLE_MODIFIERS
  int acc_mod;
#endif
  func_present = 1;
  flags = (type >> 16);

  flags |= global_modifiers;

#ifdef SENSIBLE_MODIFIERS
  acc_mod = (flags & DECL_ACCESS) & ~global_modifiers;
  if (acc_mod & (acc_mod - 1)) {
    char buf[256];
    char *end = EndOf(buf);
    char *p;

    p = strput(buf, end, "Multiple access modifiers (");
    p = get_type_modifiers(p, end, flags);
    p = strput(p, end, ") for function");
    yyerror(buf);
  }
#endif

  if (!(flags & DECL_ACCESS)) flags |= DECL_PUBLIC;
#ifdef SENSIBLE_MODIFIERS
  if (flags & DECL_NOSAVE) {
    yywarn("Illegal to declare nosave function.");
    flags &= ~DECL_NOSAVE;
  }
#endif
  type = (flags << 16) | (type & 0xffff);
  /* Handle type checking here so we know whether to typecheck
     'argument' */
  if (type & 0xffff) {
    if (CONFIG_INT(__RC_OLD_TYPE_BEHAVIOR__)) {
      exact_types = 0;
    } else {
      exact_types = (type & 0xffff) | optional_star;
    }
  } else {
    if (pragmas & PRAGMA_STRICT_TYPES) {
      if (strcmp(identifier, "create") != 0)
        yyerror("\"#pragma strict_types\" requires type of function");
      else
        exact_types = TYPE_VOID; /* default for create() */
    } else
      exact_types = 0;
  }
  return type;
}

LPC_INT rule_func_proto(LPC_INT type, LPC_INT optional_star, char **identifier, argument_t argument) {
  char *p = *identifier;
  *identifier = (char *)make_shared_string(*identifier);
  scratch_free(p);

  /* If we had nested functions, we would need to check */
  /* here if we have enough space for locals */

  /*
   * Define a prototype. If it is a real function, then the
   * prototype will be replaced below.
   */

  LPC_INT func_types = FUNC_PROTOTYPE;
  if (argument.flags & ARG_IS_VARARGS) {
    func_types |= (FUNC_TRUE_VARARGS | FUNC_VARARGS);
  }
  func_types |= (type >> 16);

  define_new_function(*identifier, argument.num_arg, 0, func_types, (type & 0xffff) | optional_star);
  /* This is safe since it is guaranteed to be in the
     function table, so it can't be dangling */
  free_string(*identifier);
  context = 0;

  return func_types;
}

void rule_func(parse_node_t **function, LPC_INT type, LPC_INT optional_star, char *identifier, argument_t argument, LPC_INT *func_types,
               parse_node_t **block_or_semi) {
  /* Either a prototype or a block */
  if (*block_or_semi) {
    int fun;

    *func_types &= ~FUNC_PROTOTYPE;
    if ((*block_or_semi)->kind != NODE_RETURN &&
        ((*block_or_semi)->kind != NODE_TWO_VALUES || (*block_or_semi)->r.expr->kind != NODE_RETURN)) {
      parse_node_t *replacement;
      CREATE_STATEMENTS(replacement, *block_or_semi, 0);
      CREATE_RETURN(replacement->r.expr, 0);
      *block_or_semi = replacement;
    }

    // Creating functions for argument defaults
    fun = define_new_function(identifier, argument.num_arg, max_num_locals - argument.num_arg, *func_types, (type & 0xffff) | optional_star);
    if (fun != -1) {
      *function = new_node_no_line();
      (*function)->kind = NODE_FUNCTION;
      (*function)->v.number = fun;
      (*function)->l.number = max_num_locals;
      (*function)->r.expr = *block_or_semi;

      if (!(*func_types & FUNC_TRUE_VARARGS) && argument.num_arg) {
        bool have_default_args = false;
        auto default_args_limit = sizeof(FUNCTION_DEF(fun)->default_args_findex) / sizeof(FUNCTION_DEF(fun)->default_args_findex[0]);
        for (int i = 0; i < argument.num_arg; i++) {
          auto local = locals_ptr[i];
          if (local.funcptr_default) {
            have_default_args = true;
            if (i > default_args_limit || argument.num_arg > default_args_limit) {
              yyerror("Functions with default arguments can only have %d args", default_args_limit);
              return ;
            }
            FUNCTION_DEF(fun)->min_arg--;
            // TODO: generate a unique name for the function
            auto funcname = fmt::format(FMT_STRING("#__{}_{}_{}"), get_current_time(), identifier, local.ihe->name, local.ihe->name);
            // the funcnum here will change in epilog().
            auto funcnum = define_new_function(funcname.c_str(), 0, 0,
                                               (*func_types & DECL_ACCESS) | DECL_NOMASK, // same access as origin function
                                               type_of_locals_ptr[locals_ptr[i].runtime_index]);
            FUNCTION_DEF(fun)->default_args_findex[i] = FUNCTION_TEMP(funcnum)->u.index;
            // debug_message("function %s (%d), new def arg function %s (%d)\n", identifier, fun, funcname.c_str(), funcnum);

            parse_node_t *node_return;
            CREATE_RETURN(node_return,  local.funcptr_default);

            auto *node_func = new_node_no_line();
            node_func->kind = NODE_FUNCTION;
            node_func->v.number = funcnum;
            node_func->l.number = 0;
            node_func->r.expr = node_return;

            auto *newnode = *function;
            CREATE_TWO_VALUES(*function, 0, newnode, node_func);
          } else {
            if (i > 0) {
              if (i > default_args_limit) continue;
              auto prev = FUNCTION_DEF(fun)->default_args_findex[i - 1];
              if (prev != 0) {
                yyerror("Function arguments with default value closure must be specified continuously.");
                return ;
              }
            }
          }
        }
      }
    } else
      *function = 0;
  } else
    *function = 0;
  free_all_local_names(!!(*block_or_semi));
}

ident_hash_elem_t *rule_define_class(LPC_INT *$$, char *$3) {
  ident_hash_elem_t *ihe;

  ihe = find_or_add_ident(PROG_STRING(*$$ = store_prog_string($3)), FOA_GLOBAL_SCOPE);
  if (ihe->dn.class_num == -1) {
    ihe->sem_value++;
    ihe->dn.class_num = mem_block[A_CLASS_DEF].current_size / sizeof(class_def_t);
    if (ihe->dn.class_num > CLASS_NUM_MASK) {
      yyerror("Too many classes, max is %d.\n", CLASS_NUM_MASK + 1);
    }

    scratch_free($3);
    return nullptr;
  } else {
    return ihe;
  }
}

void rule_define_class_members(struct ident_hash_elem_t *$2, LPC_INT $5) {
  class_def_t *sd;
  class_member_entry_t *sme;
  int i, raise_error = 0;

  /* check for a redefinition */
  if ($2 != 0) {
    sd = CLASS($2->dn.class_num);
    if (sd->size != current_number_of_locals) {
      raise_error = 1;
    } else {
      i = sd->size;
      sme = (class_member_entry_t *)mem_block[A_CLASS_MEMBER].block + sd->index;
      while (i--) {
        /* check for matching names and types */
        if (strcmp(PROG_STRING(sme[i].membername), locals_ptr[i].ihe->name) != 0 ||
            sme[i].type != (type_of_locals_ptr[i] & ~LOCAL_MODS)) {
          raise_error = 1;
          break;
        }
      }
    }
  }

  if (raise_error) {
    yyerror("Illegal to redefine class '%s',", PROG_STRING($5));
  } else {
    sd = (class_def_t *)allocate_in_mem_block(A_CLASS_DEF, sizeof(class_def_t));
    i = sd->size = current_number_of_locals;
    sd->index = mem_block[A_CLASS_MEMBER].current_size / sizeof(class_member_entry_t);
    sd->classname = $5;

    sme = (class_member_entry_t *)allocate_in_mem_block(
        A_CLASS_MEMBER, sizeof(class_member_entry_t) * current_number_of_locals);

    while (i--) {
      sme[i].membername = store_prog_string(locals_ptr[i].ihe->name);
      sme[i].type = type_of_locals_ptr[i] & ~LOCAL_MODS;
    }
  }
  free_all_local_names(0);
}
