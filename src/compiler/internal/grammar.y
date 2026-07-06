/* This is to make emacs edit this in C mode: -*-C-*- */

%define lr.type ielr
%define parse.error detailed
%define parse.assert true
%define parse.lac full
%define api.pure full
%define api.push-pull push
/* 8.3: token/rule source spans. ADDITIVE -- primary line attribution
 * stays current_line-at-reduce; yylloc feeds operand-range diagnostics
 * (see rule_set_operand_ranges) and future callers. */
%locations
%lex-param { void* yyscanner }
%parse-param { void* yyscanner }
/* 3rd conflict (added alongside string_like/template_literal): shift/reduce
 * on L_STRING between extending string_literal (string_literal: string_literal
 * L_STRING) and reducing to close off a string_like chain (string_like:
 * string_like string). Default shift-preference is exactly what's wanted --
 * it keeps folding adjacent plain string literals into one string_literal at
 * compile time, the same as before string_like existed, rather than ever
 * splitting them into a runtime string_like L_STRING chain. */
%expect 3

%start all

%{
#include "base/std.h"
#include "compiler/internal/grammar_rules.h"

extern char *outp;

/*
 * LPC grammar — parse-tree generator.
 *
 * Rules are ordered top-down: the entry point and high-level structure
 * appear first; details and primitives are defined further below.
 * All bison rules may forward-reference nonterminals defined later.
 *
 * `context` tracks nesting state used for semantic error checking:
 *   SWITCH_CONTEXT, LOOP_CONTEXT, SPECIAL_CONTEXT, SWITCH_STRINGS,
 *   SWITCH_NUMBERS, SWITCH_RANGES, SWITCH_DEFAULT.
 */
int context;
int num_refs;
int func_present;

void yyerror(void *yyscanner, const char *msg);
%}

/* =========================================================================
 * Tokens
 * =========================================================================
 *
 * Single-character operator tokens are used as character literals in rules:
 *   '+'  '-'  '/'  '*'  '%'  '&'  '|'  '<'  '>'  '^'  '~'  '?'
 *   '{'  '}'  ','  ';'  ':'  '('  ')'  '['  ']'  '$'
 */

/* Literals */
%token <string> L_STRING
%token <number> L_NUMBER
%token <real>   L_REAL

/* Template literal fragments: `head${`, `}middle${`, `}tail` */
%token <string> L_TEMPLATE_HEAD L_TEMPLATE_MIDDLE L_TEMPLATE_TAIL

/* Type keywords */
%token <number> L_BASIC_TYPE L_TYPE_MODIFIER

/* Names: L_DEFINED_NAME is already in the symbol table; L_IDENTIFIER is new */
%token <ihe> L_DEFINED_NAME
%token <string> L_IDENTIFIER

/* efun:: prefix for bypassing simul-efun overrides */
%token <string> L_EFUN

/* Postfix / prefix increment-decrement */
%token L_INC L_DEC

/* Compound assignment:  +=  -=  |=  ... */
%token <number> L_ASSIGN

/* Logical operators */
%token L_LAND L_LOR

/* Nullish coalescing:  ?? */
%token L_QUESTION_QUESTION

/* Bit-shift operators */
%token L_LSH L_RSH

/* Relational operators  >  >=  <=  (< is a literal character) */
%token <number> L_ORDER

/* Logical not:  ! */
%token L_NOT

/* Control-flow keywords */
%token L_IF L_ELSE
%token L_SWITCH L_CASE L_DEFAULT L_RANGE L_DOT_DOT_DOT
%token L_WHILE L_DO L_FOR L_FOREACH L_IN
%token L_BREAK L_CONTINUE
%token L_RETURN

/* Member access and scope */
%token L_ARROW L_DOT L_INHERIT L_COLON_COLON

/* Optional chaining:  ?.name  ?.[idx]  .?[idx]  (mappings only) */
%token L_OPTIONAL_DOT L_DOT_OPTIONAL

/* Compound literal openers:  ({  ([  (:  (: with new-syntax */
%token L_ARRAY_OPEN L_MAPPING_OPEN L_FUNCTION_OPEN
%token <number> L_NEW_FUNCTION_OPEN

/* Built-in special forms */
%token L_SSCANF L_CATCH
%token L_ARRAY
%token L_REF
%token L_PARSE_COMMAND L_TIME_EXPRESSION
%token L_CLASS L_NEW
%token <number> L_PARAMETER   /* anonymous-function parameter placeholder: $1, $2, ... */

/* Debug-only introspection token (compiled out in release) */
%token L_TREE

/* Used only by lpcfmt (the LPC formatter), never by the driver */
%token L_PREPROCESSOR_COMMAND

/* =========================================================================
 * Operator precedence (low to high; last entry binds tightest)
 * Mirrors C's table with LPC extensions (?? nullish coalescing).
 * =========================================================================
 */
%precedence LOWER_THAN_ELSE   /* resolves the dangling-else shift/reduce conflict */
%precedence L_ELSE

%precedence L_ASSIGN          /* =  +=  -=  *=  /=  %=  &=  |=  ^=  <<=  >>= */
%right '?'                    /* ternary  a ? b : c */
%left L_QUESTION_QUESTION     /* nullish coalescing  a ?? b */
%left L_LOR                   /* || */
%left L_LAND                  /* && */
%left '|'                     /* bitwise or */
%left '^'                     /* bitwise xor */
%left '&'                     /* bitwise and */
%left L_EQ L_NE               /* ==  != */
%left L_ORDER '<'             /* >  >=  <= and < */
%left L_LSH L_RSH             /* <<  >> */
%left '+' '-'
%left '*' '%' '/'
%precedence L_NOT '~'         /* !  ~  (unary prefix) */


/* =========================================================================
 * Semantic value union
 * =========================================================================
 */
%union
{
  LPC_INT number;              /* integers, opcodes, type flags */
  LPC_FLOAT real;              /* floating-point literals */
  char *string;                /* scratch-allocated string */
  argument_t argument;         /* function parameter list metadata */
  ident_hash_elem_t *ihe;      /* symbol-table entry for a known name */
  parse_node_t *node;          /* parse-tree node (most non-terminals) */
  function_context_t *contextp; /* saved function-context pointer */
  decl_t decl;                 /* block/declaration info (node + local count) */
  func_block_t func_block;     /* saved state for anonymous function body */
  struct {
    struct ident_hash_elem_t *ihe;
    LPC_INT classname_index;
  } class_header;              /* class name and index during class declaration */
  struct {
    parse_node_t *expr;
    LPC_INT saved_context;
    LPC_INT saved_cases_size;
  } switch_header;             /* switch discriminant and saved case-table state */
  struct {
    LPC_INT saved_context;
    LPC_INT saved_refs;
  } call_open;                 /* context/refs captured at the '(' of a call */
}

/* =========================================================================
 * Non-terminal type annotations
 * =========================================================================
 */

%type <number> efun_override
%type <number> single_new_local_def
%type <number> constant
%type <string> string_const string_literal
%type <argument> argument_list argument
%type <number> type optional_star type_modifier_list
%type <number> opt_basic_type basic_type atomic_type
%type <number> cast arg_type opt_atomic_type param_decl
%type <number> functional_open
%type <string> function_name identifier new_local_name
%type <node> optional_default_arg_value
%type <node> number real string string_like template_literal template_parts
%type <node> expr comma_expr for_expr sscanf catch
%type <node> parse_command time_expression opt_arg_list arg_list opt_pair_list
%type <node> pair_list assoc_pair primary_expr lvalue function_call lvalue_list
%type <node> new_local_def statement stmt_while stmt_cond stmt_do stmt_switch case
%type <node> stmt_return optional_else_part block_or_semi
%type <node> case_label switch_block
%type <node> spread_expr expr_or_block
%type <node> single_new_local_def_with_init
%type <node> class_init opt_class_init all def function
%type <node> program modifier_change inheritance type_decl
%type <node> tree
%type <decl> local_declarations local_name_list block
%type <decl> foreach_var foreach_vars for_init stmt_foreach stmt_for
%type <decl> local_declaration_statement statements
%type <class_header> class_header
%type <switch_header> switch_header
%type <call_open> call_open
%type <number> local_decl_header local_decl_statement_header
%type <func_block> lambda_return_type
%type <number> loop_start foreach_start block_start special_context_start
%type <contextp> dollar_start

%%

/* =========================================================================
 * 0. Named helper rules for mid-rule action type safety
 * =========================================================================
 */
loop_start: %empty { $$ = rule_loop_open(); };
foreach_start: %empty { $$ = rule_foreach_open(); };
block_start: %empty { $$ = rule_block_open(); };
special_context_start: %empty { $$ = rule_special_context_open(); };
dollar_start: %empty { $$ = rule_dollar_open(); };

/* =========================================================================
 * 1. Entry point and top-level structure
 * =========================================================================
 */

/* The entire compilation unit. */
all:
  program  { rule_program($program); }
;

/* A sequence of top-level definitions, separated by optional stray semicolons. */
program:
  program[prog] def opt_semicolon  { rule_program_append(&$$, $prog, $def); }
  | %empty                         { $$ = 0; }
;

/* A stray ';' after a top-level definition is legal; warn, don't error. */
opt_semicolon:
  %empty
  | ';'  { rule_opt_semicolon(); }
;

/* A single top-level definition:
 *   function or forward prototype
 *   global variable declaration
 *   inherit statement
 *   class declaration
 *   access modifier change (private:  public:  etc.)
 */
def:
  function
  | type name_list ';'  { rule_def_global_var($type); $$ = 0; }
  | inheritance
  | type_decl
  | modifier_change
;

/* =========================================================================
 * 2. Function definitions
 * =========================================================================
 */

/* A complete function definition or forward prototype.
 *
 * Mid-rule positions:
 *   4: after  type optional_star identifier  -- establish function type
 *   8: after  '(' argument ')'              -- register prototype
 *   9: block_or_semi                        -- the body (or ';' prototype)
 */
function:
  type optional_star identifier[id]
    { $<number>$ = rule_func_type($type, $optional_star, $id); } [f_type]
  '(' argument ')'
    { $<number>$ = rule_func_proto($<number>f_type, $optional_star, &$id, $argument); } [f_proto]
  block_or_semi
    { rule_func(&$$, $<number>f_type, $optional_star, $id, $argument, &$<number>f_proto, &$block_or_semi); }
;

/* The body of a function: a {block} or ';' for forward prototypes. */
block_or_semi:
  block   { $$ = rule_block_or_semi($block.node); }
  | ';'   { $$ = 0; }
  | error { $$ = 0; }
;

/* =========================================================================
 * 3. Statements
 * =========================================================================
 */

/* A statement: expression, control flow, compound form, or jump. */
statement:
  comma_expr ';'         { $$ = rule_statement_expr($comma_expr); }
  | stmt_cond
  | stmt_while
  | stmt_do
  | stmt_for             { $$ = rule_statement_compound_stmt($stmt_for); }
  | stmt_foreach         { $$ = rule_statement_compound_stmt($stmt_foreach); }
  | stmt_switch
  | stmt_return
  | block                { $$ = rule_statement_compound_stmt($block); }
  | ';'                  { $$ = 0; }
  | L_BREAK    ';'       { $$ = rule_statement_break(); }
  | L_CONTINUE ';'       { $$ = rule_statement_continue(); }
;


/* return;  or  return expr; */
stmt_return:
  L_RETURN ';'               { rule_return_void(&$$); }
  | L_RETURN comma_expr ';'  { rule_return_expr(&$$, $comma_expr); }
;

/* if-else.  Dangling-else resolved by LOWER_THAN_ELSE / L_ELSE precedence. */
stmt_cond:
  L_IF '(' comma_expr ')' statement optional_else_part
    { rule_cond(&$$, $comma_expr, $statement, $optional_else_part); }
;

optional_else_part:
  %empty %prec LOWER_THAN_ELSE  { $$ = 0; }
  | L_ELSE statement             { $$ = $statement; }
;

/* while loop. */
stmt_while:
  L_WHILE '(' comma_expr ')' loop_start statement
    { rule_while(&$$, $comma_expr, $statement, $loop_start); }
;

/* do-while loop. */
stmt_do:
  L_DO loop_start statement L_WHILE '(' comma_expr ')' ';'
    { rule_do(&$$, $statement, $comma_expr, $loop_start); }
;

/* for loop. */
stmt_for:
  L_FOR '(' for_init ';' for_expr[cond] ';' for_expr[incr] ')' loop_start statement
    { rule_for(&$$, &$for_init, $cond, $incr, $statement, $loop_start); }
;

/* foreach loop. */
stmt_foreach:
  L_FOREACH '(' foreach_vars L_IN expr ')' foreach_start statement
    { rule_foreach(&$$, &$foreach_vars, $expr, $statement, $foreach_start); }
;

/* switch.  Local declarations before the first case are allowed (C89-style). */
stmt_switch:
  switch_header '{' local_declarations case switch_block '}'
    { rule_switch(&$$, $switch_header.expr, &$local_declarations, $case, $switch_block, $switch_header.saved_context, $switch_header.saved_cases_size); }
;

/* =========================================================================
 * 4. Blocks and local variable declarations
 * =========================================================================
 */

/* A brace-enclosed block.  Position 2 is the mid-rule that saves the
 * current local-variable count so it can be restored when the block exits. */
block:
  '{' block_start statements '}'
    { rule_block(&$$, $statements.node, $block_start); }
;

/* Sequence of statements and interleaved local declarations inside a block. */
statements:
  %empty
    { rule_block_statements_empty(&$$); }
  | statement statements[stmts]
    { rule_block_statements_stmt(&$$, $statement, &$stmts); }
  | local_declaration_statement statements[stmts]
    { rule_block_statements_decl(&$$, &$local_declaration_statement, &$stmts); }
  | error ';' statements[stmts]
    { rule_block_statements_error(&$$, &$stmts); }
;

/* A typed local variable declaration used as a statement. */
local_decl_statement_header:
  basic_type  { rule_local_declaration_statement_set_type($basic_type); $$ = $basic_type; }
;

local_declaration_statement:
  local_decl_statement_header local_name_list ';'
    { rule_local_declaration_statement(&$$, &$local_name_list); }
;

/* Local declarations before the first case label in a switch body. */
local_decl_header:
  basic_type  { rule_local_declarations_set_type($basic_type); $$ = $basic_type; }
;

local_declarations:
  %empty
    { $$.node = 0; $$.num = 0; }
  | local_declarations[decls] local_decl_header local_name_list ';'
    { rule_local_declarations(&$$, &$decls, &$local_name_list); }
;

/* =========================================================================
 * 5. Expressions
 *
 * Hierarchy (tightest to loosest):
 *   primary_expr  -- atoms and postfix operations
 *   expr          -- unary, binary, assignment, ternary
 *   comma_expr    -- comma (sequence) operator
 *
 * Operator precedence is resolved by %left/%right declarations above,
 * so expr is a single flat rule with no precedence-climbing chains.
 * =========================================================================
 */

/* The sequence (comma) operator. */
comma_expr:
  expr
  | comma_expr[list] ',' expr   { rule_comma_expr(&$$, $list, $expr); }
;

expr:
  ref lvalue                          { rule_expr_ref(&$$, $lvalue); }
  | lvalue L_ASSIGN expr[val]         { rule_expr_assign(&$$, $lvalue, $L_ASSIGN, $val); }
  | error L_ASSIGN expr[val]          { rule_expr_assign_error(&$$, $val); }

  | expr[cond] '?' expr[then_b] ':' expr[else_b] %prec '?'  { rule_expr_ternary(&$$, $cond, $then_b, $else_b); }
  | expr[lhs] L_QUESTION_QUESTION expr[rhs]     { rule_expr_nullish(&$$, $lhs, $rhs); }
  | expr[lhs] L_LOR  expr[rhs]                  { rule_expr_lor(&$$, $lhs, $rhs); }
  | expr[lhs] L_LAND expr[rhs]                  { rule_expr_land(&$$, $lhs, $rhs); }

  | expr[lhs] '|' expr[rhs]   { rule_expr_or(&$$, $lhs, $rhs); }
  | expr[lhs] '^' expr[rhs]   { rule_expr_xor(&$$, $lhs, $rhs); }
  | expr[lhs] '&' expr[rhs]   { rule_expr_and(&$$, $lhs, $rhs); }

  | expr[lhs] L_EQ    expr[rhs]   { rule_expr_eq(&$$, $lhs, $rhs); }
  | expr[lhs] L_NE    expr[rhs]   { rule_expr_ne(&$$, $lhs, $rhs); }
  | expr[lhs] L_ORDER expr[rhs]   { rule_expr_order(&$$, $lhs, $L_ORDER, $rhs); }
  | expr[lhs] '<'     expr[rhs]   { rule_expr_lt(&$$, $lhs, $rhs); }

  | expr[lhs] L_LSH expr[rhs]  { rule_expr_lsh(&$$, $lhs, $rhs); }
  | expr[lhs] L_RSH expr[rhs]  { rule_expr_rsh(&$$, $lhs, $rhs); }

  | expr[lhs] '+' expr[rhs]  {
        rule_set_operand_ranges(@lhs.first_line, @lhs.first_column, @lhs.last_column,
                                @2.first_line, @2.first_column,
                                @rhs.first_line, @rhs.first_column, @rhs.last_column);
        rule_expr_add(&$$, $lhs, $rhs);
        rule_clear_operand_ranges();
    }
  | expr[lhs] '-' expr[rhs]  { rule_expr_sub(&$$, $lhs, $rhs); }
  | expr[lhs] '*' expr[rhs]  { rule_expr_mul(&$$, $lhs, $rhs); }
  | expr[lhs] '%' expr[rhs]  { rule_expr_mod(&$$, $lhs, $rhs); }
  | expr[lhs] '/' expr[rhs]  { rule_expr_div(&$$, $lhs, $rhs); }

  | cast expr[val]  %prec L_NOT     { rule_expr_cast(&$$, $cast, $val); }
  | L_INC lvalue  %prec L_NOT  { rule_expr_pre_inc(&$$, $lvalue); }
  | L_DEC lvalue  %prec L_NOT  { rule_expr_pre_dec(&$$, $lvalue); }
  | L_NOT expr[val]                 { rule_expr_not(&$$, $val); }
  | '~'   expr[val]                 { rule_expr_compl(&$$, $val); }
  | '-'   expr[val]  %prec L_NOT    { rule_expr_neg(&$$, $val); }

  | lvalue L_INC  { rule_expr_post_inc(&$$, $lvalue); }
  | lvalue L_DEC  { rule_expr_post_dec(&$$, $lvalue); }

  | primary_expr
  | sscanf
  | parse_command
  | time_expression
  | number
  | real
;

/* lvalue: syntactically a primary_expr, verified to be an assignable location. */
lvalue:
  primary_expr  { $$ = rule_lvalue($primary_expr); }
;

/* =========================================================================
 * 6. Primary expressions -- atoms and postfix operations
 * =========================================================================
 */

primary_expr:
  function_call
  | L_DEFINED_NAME                   { rule_primary_expr_defined_name(&$$, $L_DEFINED_NAME); }
  | L_IDENTIFIER                     { rule_primary_expr_identifier(&$$, $L_IDENTIFIER); }
  | L_PARAMETER                      { rule_primary_expr_parameter(&$$, $L_PARAMETER); }
  | string_like
  | '(' comma_expr ')'               { $$ = $comma_expr; }
  | catch
  | tree

  /* Dollar-expression: $(expr) -- evaluates expr in the enclosing context. */
  | '$' '(' dollar_start comma_expr ')'
    { rule_primary_expr_dollar_expr(&$$, $dollar_start, $comma_expr); }

  /* Member access */
  | primary_expr[expr_node] L_ARROW identifier   { rule_primary_expr_member_arrow(&$$, $expr_node, $identifier); }
  | primary_expr[expr_node] L_DOT   identifier   { rule_primary_expr_member_dot(&$$, $expr_node, $identifier); }

  /* Optional chaining (mappings only): expr?.name, expr?.[idx], expr.?[idx].
   * Short-circuits to 0 at runtime instead of erroring when expr isn't a
   * mapping; see rule_primary_expr_member_optional/rule_primary_expr_index_optional. */
  | primary_expr[expr_node] L_OPTIONAL_DOT identifier
    { rule_primary_expr_member_optional(&$$, $expr_node, $identifier); }
  | primary_expr[expr_node] L_OPTIONAL_DOT '[' comma_expr[idx] ']'
    { rule_primary_expr_index_optional(&$$, $expr_node, $idx); }
  | primary_expr[expr_node] L_DOT_OPTIONAL '[' comma_expr[idx] ']'
    { rule_primary_expr_index_optional(&$$, $expr_node, $idx); }

  /* Subscript and slice indexing -- eight range forms combining forward (n)
   * and reverse-from-end (<n) endpoints, plus open-ended variants. */
  | primary_expr[expr_node] '[' comma_expr ']'
    { rule_primary_expr_index(&$$, $expr_node, $comma_expr); }
  | primary_expr[expr_node] '[' '<' comma_expr ']'
    { rule_primary_expr_index_r(&$$, $expr_node, $comma_expr); }
  | primary_expr[expr_node] '[' comma_expr[start] L_RANGE comma_expr[end] ']'
    { rule_primary_expr_range_nn(&$$, $expr_node, $start, $end); }
  | primary_expr[expr_node] '[' '<' comma_expr[start] L_RANGE comma_expr[end] ']'
    { rule_primary_expr_range_rn(&$$, $expr_node, $start, $end); }
  | primary_expr[expr_node] '[' '<' comma_expr[start] L_RANGE '<' comma_expr[end] ']'
    { rule_primary_expr_range_rr(&$$, $expr_node, $start, $end); }
  | primary_expr[expr_node] '[' comma_expr[start] L_RANGE '<' comma_expr[end] ']'
    { rule_primary_expr_range_nr(&$$, $expr_node, $start, $end); }
  | primary_expr[expr_node] '[' comma_expr[start] L_RANGE ']'
    { rule_primary_expr_range_ne(&$$, $expr_node, $start); }
  | primary_expr[expr_node] '[' '<' comma_expr[start] L_RANGE ']'
    { rule_primary_expr_range_re(&$$, $expr_node, $start); }
  | primary_expr[expr_node] '[' L_RANGE '<' comma_expr[end] ']'
    { rule_primary_expr_range_nr(&$$, $expr_node, 0, $end); }
  /* Two extra alternatives to avoid a reduce/reduce conflict on ':': */
  | primary_expr[expr_node] '[' '<' comma_expr[start] L_RANGE '<' expr[end] ']'
    { rule_primary_expr_range_rr(&$$, $expr_node, $start, $end); }
  | primary_expr[expr_node] '[' '<' comma_expr[start] L_RANGE expr[end] ']'
    { rule_primary_expr_range_rn(&$$, $expr_node, $start, $end); }

  /* Anonymous function / lambda:  int (int x) { return x + 1; }
   * lambda_return_type is at position 1, argument at 3, block at 5. */
  | lambda_return_type '(' argument ')' block
    { rule_primary_expr_anon_func(&$$, &$lambda_return_type, &$argument, &$block); }

  /* First-class function constructors:
   *   (:)               -- zero-arg functional
   *   (: , a, b :)      -- functional with bound arguments
   *   (: expr :)        -- expression wrapped as a functional */
  | functional_open ':' ')'
    { rule_primary_expr_functional_1(&$$, $functional_open); }
  | functional_open ',' arg_list ':' ')'
    { rule_primary_expr_functional_2(&$$, $functional_open, $arg_list); }
  | L_FUNCTION_OPEN comma_expr ':' ')'
    { rule_primary_expr_functional_3(&$$, $comma_expr); }

  /* Compound literals */
  | L_MAPPING_OPEN opt_pair_list ']' ')'   { rule_primary_expr_mapping(&$$, $opt_pair_list); }
  | L_ARRAY_OPEN opt_arg_list '}' ')'      { rule_primary_expr_array(&$$, $opt_arg_list); }
;

/* =========================================================================
 * 7. Function calls
 * =========================================================================
 */

/* call_open captures the '(' and saves the simul-efun ref count / context. */
call_open:
  '('  { rule_call_open(&$$.saved_context, &$$.saved_refs); }
;

/* All the ways to call a function in LPC:
 *   efun::name(...)            -- bypass simul-efun, call driver efun
 *   new(...)                   -- clone and initialize an object
 *   new(class Foo, f:v, ...)   -- allocate a class instance
 *   name(...)                  -- known or bare identifier call
 *   expr[idx](...)             -- call via indexed function pointer
 *   expr->method(...)          -- call a method on an object
 *   (*fp)(...)                 -- call via function pointer
 */
function_call:
  efun_override call_open opt_arg_list ')'
    { rule_function_call_efun(&$$, $efun_override, $opt_arg_list, $call_open.saved_context, $call_open.saved_refs); }
  | L_NEW call_open opt_arg_list ')'
    { rule_function_call_new(&$$, $opt_arg_list, $call_open.saved_context, $call_open.saved_refs); }
  | L_NEW '(' L_CLASS L_DEFINED_NAME opt_class_init ')'
    { rule_function_call_new_class(&$$, $L_DEFINED_NAME, $opt_class_init); }
  | L_NEW '(' L_CLASS L_IDENTIFIER opt_class_init ')'
    { rule_function_call_new_class_undef(&$$, $L_IDENTIFIER, $opt_class_init); }
  | L_DEFINED_NAME call_open opt_arg_list ')'
    { rule_function_call_defined_name(&$$, $L_DEFINED_NAME, $opt_arg_list, $call_open.saved_context, $call_open.saved_refs); }
  | function_name call_open opt_arg_list ')'
    { rule_function_call_name(&$$, $function_name, $opt_arg_list, $call_open.saved_context, $call_open.saved_refs); }
  | primary_expr '[' comma_expr ']' call_open opt_arg_list ')'
    { rule_function_call_indexed(&$$, $primary_expr, $comma_expr, $opt_arg_list, $call_open.saved_context, $call_open.saved_refs); }
  | primary_expr L_ARROW identifier call_open opt_arg_list ')'
    { rule_function_call_arrow(&$$, $primary_expr, $identifier, $opt_arg_list, $call_open.saved_context, $call_open.saved_refs); }
  | '(' '*' comma_expr ')' call_open opt_arg_list ')'
    { rule_function_call_star(&$$, $comma_expr, $opt_arg_list, $call_open.saved_context, $call_open.saved_refs); }
;

/* =========================================================================
 * 8. Special built-in forms
 * =========================================================================
 */

/* sscanf(str, fmt, lvalue, ...) */
sscanf:
  L_SSCANF '(' expr[expr1] ',' expr[expr2] lvalue_list ')'
    { rule_sscanf(&$$, $expr1, $expr2, $lvalue_list); }
;

/* parse_command(cmd, env, fmt, lvalue, ...) */
parse_command:
  L_PARSE_COMMAND '(' expr[expr1] ',' expr[expr2] ',' expr[expr3] lvalue_list ')'
    { rule_parse_command(&$$, $expr1, $expr2, $expr3, $lvalue_list); }
;

/* catch(expr) or catch { stmts } -- evaluates body; returns 0 or an error string. */
catch:
  L_CATCH special_context_start expr_or_block
    { rule_catch(&$$, $expr_or_block, $special_context_start); }
;

/* time_expression(expr) or time_expression { stmts } -- returns tick count. */
time_expression:
  L_TIME_EXPRESSION special_context_start expr_or_block
    { rule_time_expression(&$$, $expr_or_block, $special_context_start); }
;

/* tree: debug-only parse-tree pretty-printer. */
tree:
  L_TREE block
    { rule_tree_block(&$$, $block.node); }
  | L_TREE '(' comma_expr ')'
    { rule_tree_expr(&$$, $comma_expr); }
;

/* =========================================================================
 * 9. Switch internals
 * =========================================================================
 */

/* Switch discriminant: captures the test expression and saves case-table state. */
switch_header:
  L_SWITCH '(' comma_expr ')'
    { $$.expr = $comma_expr; rule_switch_pre(&$$.saved_context, &$$.saved_cases_size); }
;

/* The body after the first case label: interleaved case labels and statements. */
switch_block:
  case switch_block[block]         { rule_switch_block_case(&$$, $case, $block); }
  | statement switch_block[block]  { rule_switch_block_stmt(&$$, $statement, $block); }
  | %empty                  { rule_switch_block_empty(&$$); }
;

/* A case or default label.  LPC supports integer ranges:
 *   case 1..5:   case 1..:   case ..5:  */
case:
  L_CASE case_label ':'
    { rule_case_single(&$$, $case_label); }
  | L_CASE case_label[start] L_RANGE case_label[end] ':'
    { rule_case_range(&$$, $start, $end); }
  | L_CASE case_label L_RANGE ':'
    { rule_case_range_from(&$$, $case_label); }
  | L_CASE L_RANGE case_label ':'
    { rule_case_range_to(&$$, $case_label); }
  | L_DEFAULT ':'
    { rule_case_default(&$$); }
;

/* A case label value: compile-time integer or string literal.
 * Note: a switch cannot mix integer and string cases. */
case_label:
  constant        { rule_case_label_constant(&$$, $constant); }
  | string_const  { rule_case_label_string(&$$, $string_const); }
;

/* =========================================================================
 * 10. For / foreach internals
 * =========================================================================
 */

/* The optional condition or step clause of a for loop. */
for_expr:
  %empty      { $$ = 0; }
  | comma_expr
;

/* The initialization clause: an expression or a typed local scoped to the loop. */
for_init:
  for_expr
    { rule_for_init_expr(&$$, $for_expr); }
  | single_new_local_def_with_init
    { rule_for_init_local(&$$, $single_new_local_def_with_init); }
;

/* A single foreach iteration variable. */
foreach_var:
  L_DEFINED_NAME          { rule_foreach_var_defined(&$$, $L_DEFINED_NAME); }
  | single_new_local_def  { rule_foreach_var_new_local(&$$, $single_new_local_def); }
  | L_IDENTIFIER          { rule_foreach_var_identifier(&$$, $L_IDENTIFIER); }
;

/* foreach supports one or two iteration variables. */
foreach_vars:
  foreach_var
    { rule_foreach_vars_single(&$$, &$foreach_var); }
  | foreach_var[var1] ',' foreach_var[var2]
    { rule_foreach_vars_double(&$$, &$var1, &$var2); }
;

/* =========================================================================
 * 11. Class declarations
 * =========================================================================
 */

/* A complete class declaration:  class Foo { int x; string name; } */
type_decl:
  class_header member_list '}'
    { rule_define_class_members($class_header.ihe, $class_header.classname_index); $$ = 0; }
;

/* The opening of a class definition: registers the name and saves its index. */
class_header:
  type_modifier_list L_CLASS identifier '{'
    { $$.ihe = rule_define_class(&$$.classname_index, $identifier); }
;

/* The body of a class: zero or more  type field, field;  lines. */
member_list:
  %empty
  | member_list basic_type { rule_member_list_set_type($basic_type); }
      member_name_list ';'
;

/* Comma-separated list of member field names sharing one type. */
member_name_list:
  member_name
  | member_name ',' member_name_list
;

/* A single class member field declaration. */
member_name:
  optional_star identifier  { rule_member_name($optional_star, $identifier); }
;

/* A single class member initializer in  new(class Foo, field: value, ...) */
class_init:
  identifier ':' expr  { $$ = rule_class_init($identifier, $expr); }
;

/* Zero or more  , field: value  initializers after the class name. */
opt_class_init:
  %empty                           { $$ = rule_opt_class_init_empty(); }
  | opt_class_init[list] ',' class_init  { $$ = rule_opt_class_init($list, $class_init); }
;

/* =========================================================================
 * 12. Types and modifiers
 * =========================================================================
 */

/* A full type: modifiers + optional base type.
 *   static int     private     (modifiers only, type inferred)
 */
type:
  type_modifier_list opt_basic_type  { $$ = rule_type($type_modifier_list, $opt_basic_type); }
;

/* Zero or more type modifiers (private, public, static, ...) as a bitmask. */
type_modifier_list:
  %empty                                 { $$ = 0; }
  | L_TYPE_MODIFIER type_modifier_list[list]   { $$ = rule_type_modifier_list($L_TYPE_MODIFIER, $list); }
;

/* Optional base type; defaults to TYPE_ANY. */
opt_basic_type:
  basic_type
  | %empty  { $$ = rule_opt_basic_type_empty(); }
;

/* A type that may carry the array modifier:  int  or  int *  or  int array */
basic_type:
  atomic_type
  | opt_atomic_type L_ARRAY  { $$ = $opt_atomic_type | TYPE_MOD_ARRAY; }
;

/* Like atomic_type but defaults to TYPE_ANY when omitted. */
opt_atomic_type:
  atomic_type
  | %empty  { $$ = rule_opt_basic_type_empty(); }
;

/* A base type without the array modifier:  int, string, object, class Foo, ... */
atomic_type:
  L_BASIC_TYPE
  | L_CLASS L_DEFINED_NAME  { $$ = rule_atomic_type_class($L_DEFINED_NAME); }
  | L_CLASS L_IDENTIFIER    { $$ = rule_atomic_type_class_identifier($L_IDENTIFIER); }
;

/* A parameter type that may also be passed by reference:  int & */
arg_type:
  basic_type
  | basic_type ref  { $$ = $basic_type | LOCAL_MOD_REF; }
;

/* An explicit type cast:  (int)  or  (string *) */
cast:
  '(' basic_type optional_star ')'  { $$ = rule_cast($basic_type, $optional_star); }
;

/* The '*' array-pointer modifier. */
optional_star:
  %empty  { $$ = 0; }
  | '*'   { $$ = TYPE_MOD_ARRAY; }
;

/* =========================================================================
 * 13. Identifiers and name qualification
 * =========================================================================
 */

/* An identifier, possibly already in the symbol table. */
identifier:
  L_DEFINED_NAME  { $$ = rule_identifier_defined_name($L_DEFINED_NAME); }
  | L_IDENTIFIER
;

/* A name being introduced as a new local or parameter. */
new_local_name:
  L_IDENTIFIER
  | L_DEFINED_NAME  { $$ = rule_new_local_name_redefine($L_DEFINED_NAME); }
;

/* efun::name -- bypass any simul-efun override and call the driver efun. */
efun_override:
  L_EFUN L_COLON_COLON identifier  { $$ = rule_efun_override($identifier); }
  | L_EFUN L_COLON_COLON L_NEW     { $$ = rule_efun_override_new(); }
;

/* A potentially qualified function name.
 *   foo              -- local or global function
 *   ::foo            -- this object, bypassing inheritance
 *   ClassName::foo   -- specific inherited class
 *   int::to_string   -- built-in type method
 */
function_name:
  L_IDENTIFIER
  | L_COLON_COLON identifier               { $$ = rule_function_name_colon_colon($identifier); }
  | L_BASIC_TYPE L_COLON_COLON identifier  { $$ = rule_function_name_type($L_BASIC_TYPE, $identifier); }
  | identifier[obj] L_COLON_COLON identifier[id]    { $$ = rule_function_name_obj($obj, $id); }
;

/* inherit "/path/to/file"; */
inheritance:
  type_modifier_list L_INHERIT string_const ';'
    { if (rule_inheritence(&$$, $type_modifier_list, $string_const)) { YYACCEPT; } }
;

/* private:  or  public:  -- switches the default access for subsequent defs. */
modifier_change:
  type_modifier_list ':'  { $$ = rule_modifier_change($type_modifier_list); }
;

/* =========================================================================
 * 14. Function parameters and local variable definitions
 * =========================================================================
 */

/* A complete formal parameter list: possibly empty, possibly variadic. */
argument:
  %empty
    { $$.num_arg = 0; $$.flags = 0; }
  | argument_list
  | argument_list L_DOT_DOT_DOT
    { rule_argument_varargs(&$$, &$argument_list); }
;

/* One or more comma-separated param_decl entries. */
argument_list:
  param_decl
    { rule_argument_list_single(&$$, $param_decl); }
  | argument_list[list] ',' param_decl
    { rule_argument_list_multi(&$$, &$list, $param_decl); }
;

/* A single formal parameter:
 *   int        -- typed, anonymous (prototype-only)
 *   int x      -- typed and named
 *   x          -- untyped and named (legacy, treated as mixed)
 */
param_decl:
  arg_type optional_star
    { $$ = rule_param_decl_typed($arg_type | $optional_star); }
  | arg_type optional_star new_local_name optional_default_arg_value
    { $$ = rule_param_decl_typed_name($arg_type | $optional_star, $new_local_name, $optional_default_arg_value); }
  | new_local_name
    { $$ = rule_param_decl_untyped_name($new_local_name); }
;

/* Default argument value:  : (: expr :) */
optional_default_arg_value:
  %empty                                  { $$ = 0; }
  | ':' L_FUNCTION_OPEN comma_expr ':' ')'  { $$ = rule_default_arg_value($comma_expr); }
;

/* One or more comma-separated local definitions sharing a type. */
local_name_list:
  new_local_def
    { rule_local_name_list_single(&$$, $new_local_def); }
  | new_local_def ',' local_name_list[list]
    { rule_local_name_list_multi(&$$, $new_local_def, &$list); }
;

/* A single local variable: name with optional array modifier and initializer. */
new_local_def:
  optional_star new_local_name
    { $$ = rule_new_local_def($new_local_name, $optional_star); }
  | optional_star new_local_name L_ASSIGN expr
    { $$ = rule_new_local_def_with_init($new_local_name, $optional_star, $L_ASSIGN, $expr); }
;

/* A single typed local (for for-init and foreach). */
single_new_local_def:
  arg_type optional_star new_local_name
    { rule_single_new_local_def(&$$, $arg_type | $optional_star, $new_local_name); }
;

/* A typed local with mandatory initializer (for-init only). */
single_new_local_def_with_init:
  single_new_local_def L_ASSIGN expr
    { $$ = rule_single_new_local_def_with_init($single_new_local_def, $L_ASSIGN, $expr); }
;

/* =========================================================================
 * 15. Global variable declarations
 * =========================================================================
 */

/* One or more global variable names sharing a type:  int x, *y; */
name_list:
  new_name
  | new_name ',' name_list
;

/* A single global variable name, optionally with an initializer. */
new_name:
  optional_star identifier               { rule_new_name($optional_star, $identifier); }
  | optional_star identifier L_ASSIGN expr { rule_new_name_with_init($optional_star, $identifier, $L_ASSIGN, $expr); }
;

/* =========================================================================
 * 16. First-class function openers and lambda
 * =========================================================================
 */

/* Return-type specifier of an anonymous function (lambda).
 * Saves function-context state before the parameter list is parsed. */
lambda_return_type:
  L_BASIC_TYPE  { rule_lambda_return_type(&$$, $L_BASIC_TYPE); }
;

/* The opening token of a first-class function constructor.
 *   (:           -- canonical new-style opener
 *   efun::(      -- functional wrapping a driver efun
 */
functional_open:
  L_NEW_FUNCTION_OPEN
  | L_FUNCTION_OPEN efun_override  { $$ = rule_functional_open($efun_override); }
;

/* =========================================================================
 * 17. Expression list building blocks
 * =========================================================================
 */

/* A key:value pair inside a mapping literal:  key : value */
assoc_pair:
  expr[key] ':' expr[value]  { rule_assoc_pair(&$$, $key, $value); }
;

/* A single argument, optionally spread:  arr... */
spread_expr:
  expr                  { rule_spread_expr_normal(&$$, $expr); }
  | expr L_DOT_DOT_DOT  { rule_spread_expr_dots(&$$, $expr); }
;

/* A non-empty comma-separated list of spread_expr arguments. */
arg_list:
  spread_expr
    { rule_arg_list_single(&$$, $spread_expr); }
  | arg_list[list] ',' spread_expr
    { rule_arg_list_multi(&$$, $list, $spread_expr); }
;

/* A possibly-empty argument list with optional trailing comma.
 * Used for function call arguments and ({...}) array literals. */
opt_arg_list:
  %empty          { rule_opt_arg_list_empty(&$$); }
  | arg_list      { rule_opt_arg_list(&$$, $arg_list); }
  | arg_list ','  { rule_opt_arg_list(&$$, $arg_list); }
;

/* A non-empty list of key:value pairs. */
pair_list:
  assoc_pair
    { rule_pair_list_single(&$$, $assoc_pair); }
  | pair_list[list] ',' assoc_pair
    { rule_pair_list_multi(&$$, $list, $assoc_pair); }
;

/* A possibly-empty mapping pair list with optional trailing comma.
 * Used for ([...]) mapping literals. */
opt_pair_list:
  %empty           { rule_opt_pair_list_empty(&$$); }
  | pair_list      { rule_opt_pair_list(&$$, $pair_list); }
  | pair_list ','  { rule_opt_pair_list(&$$, $pair_list); }
;

/* An lvalue list for sscanf/parse_command out-parameters:  , &var, &var */
lvalue_list:
  %empty                    { $$ = rule_lvalue_list_empty(); }
  | ',' lvalue lvalue_list[list]  { $$ = rule_lvalue_list($lvalue, $list); }
;

/* The body of catch and time_expression: a block or a parenthesized expr. */
expr_or_block:
  block                  { $$ = rule_expr_or_block_block($block); }
  | '(' comma_expr ')'   { $$ = rule_expr_or_block_expr($comma_expr); }
;

/* The ref keyword -- pass a variable by reference. */
ref:
  L_REF
;

/* =========================================================================
 * 18. Literals and compile-time constants
 * =========================================================================
 */

/* A string wrapped as a parse-tree node. */
string:
  string_literal  { rule_string(&$$, $string_literal); }
;

/* Template literal: a backtick string with `${expr}` interpolation.
 * L_TEMPLATE_HEAD/L_TEMPLATE_MIDDLE/L_TEMPLATE_TAIL are the literal text
 * fragments between interpolations (lexed natively, see lex.l); each
 * interpolated `expr` is coerced to a string at runtime (F_TEMPLATE_COERCE)
 * and the whole thing folds into a chain of runtime string concatenations.
 * A backtick string with no interpolation at all never reaches this rule --
 * the lexer returns a plain L_STRING for it instead, so it participates in
 * ordinary string_literal concatenation like a "..." literal would. */
template_literal:
  L_TEMPLATE_HEAD expr[e] template_parts[rest]  { rule_template_literal(&$$, $L_TEMPLATE_HEAD, $e, $rest); }
;

template_parts:
  L_TEMPLATE_TAIL                                     { rule_template_parts_tail(&$$, $L_TEMPLATE_TAIL); }
  | L_TEMPLATE_MIDDLE expr[e] template_parts[rest]     { rule_template_parts_middle(&$$, $L_TEMPLATE_MIDDLE, $e, $rest); }
;

/* A string-valued primary expression: any mix of adjacent plain strings and
 * template literals, concatenated left to right. Pure adjacent "..." chains
 * still fold at compile time via string_literal above; anything touching a
 * template literal folds into runtime F_ADD nodes instead, since a template
 * literal's value isn't known until its interpolated expressions run. */
string_like:
  string
  | template_literal
  | string_like[left] string[right]            { rule_string_like_concat(&$$, $left, $right); }
  | string_like[left] template_literal[right]   { rule_string_like_concat(&$$, $left, $right); }
;

/* Adjacent string literal tokens are implicitly concatenated:
 *   "hello" " " "world"  ->  "hello world"
 */
string_literal:
  L_STRING
  | string_literal[left] L_STRING  { $$ = rule_string_literal_concat($left, $L_STRING); }
;

/* A compile-time string expression used where a raw string value is needed
 * (inherit paths, case labels). May be parenthesized or joined with +. */
string_const:
  string_literal
  | '(' string_const[str] ')'           { $$ = $str; }
  | string_const[left] '+' string_const[right]  { $$ = rule_string_literal_concat($left, $right); }
;

number:
  L_NUMBER  { rule_number(&$$, $L_NUMBER); }
;

real:
  L_REAL  { rule_real(&$$, $L_REAL); }
;

/* A compile-time constant integer expression folded at parse time.
 * Used exclusively for switch case labels. */
constant:
  constant[lhs] '|' constant[rhs]    { rule_constant_or(&$$, $lhs, $rhs); }
  | constant[lhs] '^' constant[rhs]  { rule_constant_xor(&$$, $lhs, $rhs); }
  | constant[lhs] '&' constant[rhs]  { rule_constant_and(&$$, $lhs, $rhs); }
  | constant[lhs] L_EQ constant[rhs]   { rule_constant_eq(&$$, $lhs, $rhs); }
  | constant[lhs] L_NE constant[rhs]   { rule_constant_ne(&$$, $lhs, $rhs); }
  | constant[lhs] L_ORDER constant[rhs]  { rule_constant_order(&$$, $lhs, $L_ORDER, $rhs); }
  | constant[lhs] '<' constant[rhs]    { rule_constant_lt(&$$, $lhs, $rhs); }
  | constant[lhs] L_LSH constant[rhs]  { rule_constant_lsh(&$$, $lhs, $rhs); }
  | constant[lhs] L_RSH constant[rhs]  { rule_constant_rsh(&$$, $lhs, $rhs); }
  | constant[lhs] '+' constant[rhs]    { rule_constant_add(&$$, $lhs, $rhs); }
  | constant[lhs] '-' constant[rhs]    { rule_constant_sub(&$$, $lhs, $rhs); }
  | constant[lhs] '*' constant[rhs]    { rule_constant_mul(&$$, $lhs, $rhs); }
  | constant[lhs] '%' constant[rhs]    { rule_constant_mod(&$$, $lhs, $rhs); }
  | constant[lhs] '/' constant[rhs]    { rule_constant_div(&$$, $lhs, $rhs); }
  | '(' constant[val] ')'         { $$ = $val; }
  | L_NUMBER                 { $$ = $L_NUMBER; }
  | '-' L_NUMBER             { rule_constant_neg(&$$, $L_NUMBER); }
  | L_NOT L_NUMBER           { rule_constant_not(&$$, $L_NUMBER); }
  | '~' L_NUMBER             { rule_constant_compl(&$$, $L_NUMBER); }
;

%%
