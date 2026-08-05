#include "base/std.h"

#include "vm/vm.h"
#include "vm/internal/base/machine.h"
#include "compiler/internal/compiler.h"
#include "compiler/internal/lexer.h"
#include "compiler/internal/scratchpad.h"
#include "compiler/internal/generate.h"
#include "compiler/internal/grammar_rules.h"
#include "debugger/debug_hook.h"

#include <fmt/format.h>

extern int context;       // defined in grammar.autogen.cc (via grammar.y preamble)
extern int func_present;  // defined in grammar.autogen.cc (via grammar.y preamble)
extern int num_refs;      // defined in grammar.autogen.cc (via grammar.y preamble)

// Snapshot of the function declaration's own line, taken in rule_func_type()
// (fired right after `type optional_star identifier` -- current_line is
// still on the function header, in the right file) and consumed by
// rule_func() once the whole body has been parsed. rule_func() can't just
// capture current_line itself: by the time it fires, parsing has moved on
// past the closing '}' (LALR needs a lookahead token to reduce), which can
// mean a completely different file if the function was the last thing in
// an #included header. Function declarations don't nest (LPC has no named
// function inside a function), so a single non-reentrant slot is safe --
// anonymous/lambda functions go through rule_primary_expr_anon_func()
// instead and never touch this.
static int pending_func_decl_line;

void rule_program(parse_node_t* program_node) { comp_trees[TREE_MAIN] = program_node; }

bool rule_inheritence(parse_node_t** result_node, int type_mod,
                      const ScratchString* inherit_file_name) {
  object_t* ob;
  inherit_t inherit;
  int initializer;
#ifdef SENSIBLE_MODIFIERS
  int acc_mod;
#endif

  type_mod |= global_modifiers;

#ifdef SENSIBLE_MODIFIERS
  acc_mod = (type_mod & DECL_ACCESS) & ~global_modifiers;
  if (acc_mod & (acc_mod - 1)) {
    char buf[256];
    char* end = EndOf(buf);
    char* p;

    p = strput(buf, end, "Multiple access modifiers (");
    p = get_type_modifiers(p, end, acc_mod);
    p = strput(p, end, ") for inheritance");
    yyerror(buf);
  }
#endif

  if (!(type_mod & DECL_ACCESS)) type_mod |= DECL_PUBLIC;
  if (var_defined) {
    yyerror("Illegal to inherit after defining global variables.");
    inherit_file = 0;
    inherit_file_source.clear();
    return true;
  }

  /* master::inherit_program(from, path, priv) -- the master may redirect
     the inheritance to another file (string return), supply the inherited
     program's source itself (array-of-strings return), or deny it (any
     other return). No return value / a missing apply / no master keeps
     the default behavior. Same mid-compile master-apply precedent as
     valid_override and get_include_path: the master's implementation
     must not trigger a compile (the compiler is non-reentrant). */
  ScratchString inherit_path(*inherit_file_name);
  std::string inline_source;
  bool have_inline_source = false;
  if (compiler_vm_context && master_ob) {
    push_malloced_string(add_slash(main_file_name()));
    push_malloced_string(string_copy(inherit_path.c_str(), "inherit_program"));
    push_number((type_mod & DECL_PRIVATE) ? 1 : 0);
    svalue_t* ret = safe_apply_master_ob(APPLY_INHERIT_PROGRAM, 3);
    if (ret && ret != reinterpret_cast<svalue_t*>(-1)) {
      if (ret->type == T_STRING) {
        inherit_path = ScratchString(ret->u.string);
      } else if (ret->type == T_ARRAY) {
        array_t* arr = ret->u.arr;
        for (int i = 0; i < arr->size; i++) {
          if (arr->item[i].type != T_STRING) {
            yyerror("master::%s: source for '%s' must be an array of strings",
                    applies_table[APPLY_INHERIT_PROGRAM], inherit_path.c_str());
            inherit_file = 0;
            inherit_file_source.clear();
            return true;
          }
          inline_source += arr->item[i].u.string;
          inline_source += '\n';
        }
        have_inline_source = true;
      } else {
        yyerror("Inheritance of '%s' denied by master::%s", inherit_path.c_str(),
                applies_table[APPLY_INHERIT_PROGRAM]);
        inherit_file = 0;
        inherit_file_source.clear();
        return true;
      }
    }
  }

  ob = find_object2(inherit_path.c_str());
  if (ob == 0) {
    inherit_file = alloc_cstring(inherit_path.c_str(), "inherit");
    inherit_file_source = have_inline_source ? std::move(inline_source) : std::string();
    /* Return back to load_object() */
    return true;
  }
  inherit.prog = ob->prog;

  if (mem_block[A_INHERITS].current_size) {
    inherit_t* prev_inherit = INHERIT(NUM_INHERITS - 1);

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
  inherit.type_mod = type_mod;
  add_to_mem_block(A_INHERITS, (char*)&inherit, sizeof inherit);

  /* The following has to come before copy_vars - Sym */
  copy_structures(ob->prog);
  copy_variables(ob->prog, type_mod);
  initializer = copy_functions(ob->prog, type_mod);
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
  *result_node = 0;

  return false;
}

LPC_INT rule_func_type(LPC_INT type, LPC_INT optional_star, const ScratchString* identifier) {
  int flags;
#ifdef SENSIBLE_MODIFIERS
  int acc_mod;
#endif
  pending_func_decl_line = current_line_base + current_line;
  func_present = 1;
  flags = (type >> 16);

  flags |= global_modifiers;

#ifdef SENSIBLE_MODIFIERS
  acc_mod = (flags & DECL_ACCESS) & ~global_modifiers;
  if (acc_mod & (acc_mod - 1)) {
    char buf[256];
    char* end = EndOf(buf);
    char* p;

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
      if (*identifier != "create")
        yyerror("\"#pragma strict_types\" requires type of function");
      else
        exact_types = TYPE_VOID; /* default for create() */
    } else
      exact_types = 0;
  }
  return type;
}

LPC_INT rule_func_proto(LPC_INT type, LPC_INT optional_star, const ScratchString* identifier,
                        const char** shared_name_out, argument_t argument) {
  /* The name changes representation here: the arena string the lexer
     produced becomes a SHARED string, registered in the function table.
     *shared_name_out (the value-stack slot's shared_string member) is
     what rule_func consumes -- the arena original is simply left to the
     arena's bulk free. */
  const char* shared = make_shared_string(identifier->c_str());

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

  define_new_function(shared, argument.num_arg, 0, func_types, (type & 0xffff) | optional_star);
  /* Dropping our ref is safe: the function table's ref keeps the shared
     string alive, so *shared_name_out can't be dangling. */
  free_string(shared);
  *shared_name_out = shared;
  context = 0;

  return func_types;
}

void rule_func(parse_node_t** function, LPC_INT type, LPC_INT optional_star, const char* identifier,
               argument_t argument, LPC_INT* func_types, parse_node_t** block_or_semi) {
  /* Either a prototype or a block */
  if (*block_or_semi) {
    int fun;

    *func_types &= ~FUNC_PROTOTYPE;
    if ((*block_or_semi)->kind != NODE_RETURN && ((*block_or_semi)->kind != NODE_TWO_VALUES ||
                                                  (*block_or_semi)->r.expr->kind != NODE_RETURN)) {
      parse_node_t* replacement;
      CREATE_STATEMENTS(replacement, *block_or_semi, 0);
      CREATE_RETURN(replacement->r.expr, 0);
      *block_or_semi = replacement;
    }

    // Creating functions for argument defaults
    fun = define_new_function(identifier, argument.num_arg, max_num_locals - argument.num_arg,
                              *func_types, (type & 0xffff) | optional_star);
    if (fun != -1) {
      // Snapshot local/argument names for the debugger's variable inspector
      // (DESIGN.md §9) before free_all_local_names() discards them below.
      // Captured here (top of the block, before the default-argument-closure
      // loop) rather than right before free_all_local_names() so an early
      // `return` from that loop on a compile error can't skip it.
      //
      // Interned via store_prog_string(), same as class member names in
      // rule_define_class_members() above -- the strings ride on
      // prog->strings[]'s own generic per-program free loop, so no per-entry
      // free is needed, only a single FREE() of the backing short[] (see
      // deallocate_program()).
      //
      // Indexed by runtime_index, NOT array position: a local declared
      // inside a for()/switch() block that already closed uses
      // pop_n_locals(), which drops it from locals_ptr[] (so it can't be
      // captured here) without giving its slot back to max_num_locals -- the
      // slot number stays permanently reserved for this function. Those
      // slots are left as -1 (falls back to argN/localN in the debugger).
      if (max_num_locals > 0 && lpc_debugger_wants_local_names()) {
        function_t* debug_def = FUNCTION_DEF(fun);
        int total = max_num_locals;
        auto* names = reinterpret_cast<short*>(
            DMALLOC(total * sizeof(short), TAG_LOCAL_NAMES, "rule_func: local_names"));
        for (int li = 0; li < total; li++) {
          names[li] = -1;
        }
        for (int li = 0; li < current_number_of_locals; li++) {
          int slot = locals_ptr[li].runtime_index;
          if (slot >= 0 && slot < total) {
            names[slot] = store_prog_string(locals_ptr[li].ihe->name);
          }
        }
        debug_def->local_names = names;
      }

      *function = new_node_no_line();
      (*function)->kind = NODE_FUNCTION;
      (*function)->v.number = fun;
      (*function)->l.number = max_num_locals;
      (*function)->r.expr = *block_or_semi;
      // Safety net alongside the explicit-return line fix in
      // rule_return_void()/rule_return_expr() (grammar_rules_loops.cc): a
      // function whose body has NO explicit return (an implicit
      // `return 0;`/`return;` gets synthesized above by rule_func() itself,
      // via CREATE_RETURN's default new_node_no_line()) can still reach
      // codegen with every one of its statements' lines equal to 0 -- e.g.
      // an empty body, or one whose only statement the optimizer discards
      // entirely (a side-effect-free expression statement). Stamping the
      // function's own declaration line here (captured early, in
      // rule_func_type(), before parsing could drift into a different
      // file -- see pending_func_decl_line's comment) makes
      // i_generate_node()'s existing `if (expr->line && ...)` guard call
      // switch_to_line() at function entry regardless of what's inside.
      (*function)->line = pending_func_decl_line;

      if (!(*func_types & FUNC_TRUE_VARARGS) && argument.num_arg) {
        bool have_default_args = false;
        auto default_args_limit = sizeof(FUNCTION_DEF(fun)->default_args_findex) /
                                  sizeof(FUNCTION_DEF(fun)->default_args_findex[0]);
        for (int i = 0; i < argument.num_arg; i++) {
          auto local = locals_ptr[i];
          if (local.funcptr_default) {
            have_default_args = true;
            if (i > default_args_limit || argument.num_arg > default_args_limit) {
              yyerror("Functions with default arguments can only have %d args",
                      static_cast<int>(default_args_limit));
              return;
            }
            FUNCTION_DEF(fun)->min_arg--;
            // The helper's name must be unique across the whole inherit
            // chain: it is defined DECL_NOMASK, so a parent and child both
            // defining foo(int a: (: ... :)) with colliding helper names is
            // an "Illegal to redefine 'nomask' function" compile error.
            // A wall-clock timestamp only disambiguated compiles that
            // happened in DIFFERENT seconds -- overriding an inherited
            // default-arg function failed whenever parent and child
            // compiled within the same second (the normal case), and
            // compiles were unreproducible byte-wise. A process-global
            // counter is unique for every compile in the process and
            // deterministic given compile order.
            static uint64_t default_arg_seq = 0;
            auto funcname = fmt::format(FMT_STRING("#__{}_{}_{}"), ++default_arg_seq, identifier,
                                        local.ihe->name);
            // the funcnum here will change in epilog().
            auto funcnum = define_new_function(
                funcname.c_str(), 0, 0,
                (*func_types & DECL_ACCESS) | DECL_NOMASK,  // same access as origin function
                type_of_locals_ptr[locals_ptr[i].runtime_index]);
            FUNCTION_DEF(fun)->default_args_findex[i] = FUNCTION_TEMP(funcnum)->u.index;
            // debug_message("function %s (%d), new def arg function %s (%d)\n", identifier, fun,
            // funcname.c_str(), funcnum);

            parse_node_t* node_return;
            CREATE_RETURN(node_return, local.funcptr_default);

            auto* node_func = new_node_no_line();
            node_func->kind = NODE_FUNCTION;
            node_func->v.number = funcnum;
            node_func->l.number = 0;
            node_func->r.expr = node_return;

            auto* newnode = *function;
            CREATE_TWO_VALUES(*function, 0, newnode, node_func);
          } else {
            if (i > 0) {
              if (i > default_args_limit) continue;
              auto prev = FUNCTION_DEF(fun)->default_args_findex[i - 1];
              if (prev != 0) {
                yyerror(
                    "Function arguments with default value closure must be specified "
                    "continuously.");
                return;
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

ident_hash_elem_t* rule_define_class(LPC_INT* classname_idx_out, const ScratchString* class_name) {
  ident_hash_elem_t* ihe;

  ihe = find_or_add_ident(PROG_STRING(*classname_idx_out = store_prog_string(class_name)),
                          FOA_GLOBAL_SCOPE);
  if (ihe->dn.class_num == -1) {
    ihe->sem_value++;
    ihe->dn.class_num = mem_block[A_CLASS_DEF].current_size / sizeof(class_def_t);
    if (ihe->dn.class_num > CLASS_NUM_MASK) {
      yyerror("Too many classes, max is %d.\n", CLASS_NUM_MASK + 1);
    }

    return nullptr;
  } else {
    return ihe;
  }
}

void rule_define_class_members(struct ident_hash_elem_t* class_ihe, LPC_INT classname_idx) {
  class_def_t* sd;
  class_member_entry_t* sme;
  int i, raise_error = 0;

  /* check for a redefinition */
  if (class_ihe != 0) {
    sd = CLASS(class_ihe->dn.class_num);
    if (sd->size != current_number_of_locals) {
      raise_error = 1;
    } else {
      i = sd->size;
      sme = (class_member_entry_t*)mem_block[A_CLASS_MEMBER].block + sd->index;
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
    yyerror("Illegal to redefine class '%s',", PROG_STRING(classname_idx));
  } else {
    sd = (class_def_t*)allocate_in_mem_block(A_CLASS_DEF, sizeof(class_def_t));
    i = sd->size = current_number_of_locals;
    sd->index = mem_block[A_CLASS_MEMBER].current_size / sizeof(class_member_entry_t);
    sd->classname = classname_idx;

    sme = (class_member_entry_t*)allocate_in_mem_block(
        A_CLASS_MEMBER, sizeof(class_member_entry_t) * current_number_of_locals);

    while (i--) {
      sme[i].membername = store_prog_string(locals_ptr[i].ihe->name);
      sme[i].type = type_of_locals_ptr[i] & ~LOCAL_MODS;
    }
  }
  free_all_local_names(0);
  /* Reads 1 while the very next top-level def is parsed -- see
     rule_def_global_var()'s combined-declaration diagnostic. */
  g_compile.class_def_cooldown = 2;
}

// ============================================================================
// Thin wrappers that keep grammar.y free of direct macro / global-var access
// ============================================================================

LPC_INT rule_loop_open() {
  LPC_INT saved = context;
  context = LOOP_CONTEXT;
  return saved;
}

LPC_INT rule_special_context_open() {
  LPC_INT saved = context;
  context = SPECIAL_CONTEXT;
  return saved;
}

LPC_INT rule_block_open() { return (LPC_INT)current_number_of_locals; }

void rule_number(parse_node_t** result, LPC_INT val) { CREATE_NUMBER(*result, val); }

void rule_real(parse_node_t** result, LPC_FLOAT val) { CREATE_REAL(*result, val); }

void rule_primary_expr_parameter(parse_node_t** result, LPC_INT n) {
  CREATE_PARAMETER(*result, TYPE_ANY, n);
}

void rule_program_append(parse_node_t** result, parse_node_t* prog, parse_node_t* def) {
  if (g_compile.class_def_cooldown) {
    g_compile.class_def_cooldown--;
  }
  CREATE_TWO_VALUES(*result, 0, prog, def);
}

void rule_tree_block(parse_node_t** result, parse_node_t* block_node) {
#ifdef DEBUG
  *result = new_node_no_line();
  lpc_tree_form(block_node, *result);
#else
  (void)block_node;
  *result = nullptr;
#endif
}

void rule_tree_expr(parse_node_t** result, parse_node_t* expr) {
#ifdef DEBUG
  *result = new_node_no_line();
  lpc_tree_form(expr, *result);
#else
  (void)expr;
  *result = nullptr;
#endif
}

void rule_opt_semicolon() { yywarn("Extra ';'. Ignored."); }

ScratchString* rule_string_literal_concat(ScratchString* s1, ScratchString* s2) {
  /* Both operands are arena strings consumed by this reduction; append in
     place and hand s1 back as the merged literal. */
  *s1 += *s2;
  return s1;
}
