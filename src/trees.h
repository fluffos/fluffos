/*
 * Defines and types for the compiler parse trees.
 */

#ifndef _TREES_H
#define _TREES_H
#include "std.h"
#define NODES_PER_BLOCK         256

enum node_type {
    NODE_RETURN, NODE_TWO_VALUES, NODE_OPCODE, NODE_OPCODE_1, NODE_OPCODE_2,
    NODE_UNARY_OP, NODE_UNARY_OP_1, NODE_BINARY_OP, NODE_BINARY_OP_1,
    NODE_TERNARY_OP, NODE_TERNARY_OP_1, NODE_CONTROL_JUMP, NODE_LOOP,
    NODE_CALL, NODE_CALL_1, NODE_CALL_2, NODE_LAND_LOR, NODE_FOREACH,
    NODE_LVALUE_EFUN, NODE_SWITCH_RANGES, NODE_SWITCH_STRINGS, 
    NODE_SWITCH_DIRECT, NODE_SWITCH_NUMBERS, NODE_CASE_NUMBER,
    NODE_CASE_STRING, NODE_DEFAULT, NODE_IF, NODE_BRANCH_LINK, NODE_PARAMETER,
    NODE_PARAMETER_LVALUE, NODE_EFUN, NODE_ANON_FUNC, NODE_REAL, NODE_NUMBER,
    NODE_STRING, NODE_FUNCTION_CONSTRUCTOR, NODE_CATCH, NODE_TIME_EXPRESSION,
    NODE_FUNCTION
};

enum control_jump_type {
    CJ_BREAK_SWITCH = 2, CJ_BREAK = 0, CJ_CONTINUE = 1
};

union parse_value {
    long number;
    double real;
    struct parse_node_s *expr;
};

typedef struct parse_node_s {
    short kind;
#if !defined(USE_32BIT_ADDRESSES) 
    short line;
#else
    unsigned int line;
#endif
    unsigned short type;
    union parse_value v, l, r; /* left, right, and value */
} parse_node_t;

typedef struct parse_node_block_s {
    struct parse_node_block_s *next;
    parse_node_t nodes[NODES_PER_BLOCK];
} parse_node_block_t;

#define IS_NODE(vn, nt, op) ((vn)->kind == nt && (vn)->v.number == op)

#define CREATE_TERNARY_OP(vn, op, t, x, y, z) SAFE(\
                                         (vn) = new_node();\
                                         (vn)->kind = NODE_TERNARY_OP;\
                                         INT_CREATE_TERNARY_OP(vn, op, t,\
                                                               x, y, z);\
                                                   )
#define INT_CREATE_TERNARY_OP(vn, op, t, x, y, z) SAFE(\
                                                       (vn)->l.expr = (x);\
                                                       (vn)->type = t;\
                                                       CREATE_BINARY_OP((vn)->r.expr, op, t, y, z);\
                                                       )
#define CREATE_BINARY_OP(vn, op, t, x, y) SAFE(\
                                               (vn) = new_node();\
                                               (vn)->kind = NODE_BINARY_OP;\
                                               INT_CREATE_BINARY_OP(vn, op,\
                                                                    t, x, y);\
                                               )
#define INT_CREATE_BINARY_OP(vn, op, t, x, y) SAFE(\
                                                   INT_CREATE_UNARY_OP(vn, op,\
                                                                       t, y);\
                                                   (vn)->l.expr = x;\
                                                   )
#define CREATE_UNARY_OP(vn, op, t, x) SAFE(\
                                           (vn) = new_node();\
                                           (vn)->kind = NODE_UNARY_OP;\
                                           INT_CREATE_UNARY_OP(vn, op, t, x);\
                                           )
#define INT_CREATE_UNARY_OP(vn, op, t, x) SAFE(\
                                               INT_CREATE_OPCODE(vn, op, t);\
                                               (vn)->r.expr = x;\
                                               )
#define CREATE_OPCODE(vn, op, t) SAFE(\
                                      (vn) = new_node();\
                                      (vn)->kind = NODE_OPCODE;\
                                      INT_CREATE_OPCODE(vn, op, t);\
                                      )
#define INT_CREATE_OPCODE(vn, op, t) SAFE(\
                                          (vn)->v.number = op;\
                                          (vn)->type = t;\
                                          )

#define CREATE_OPCODE_1(vn, op, t, p) SAFE(\
                                           (vn) = new_node();\
                                           (vn)->kind = NODE_OPCODE_1;\
                                           INT_CREATE_OPCODE(vn, op, t);\
                                           (vn)->l.number = p;\
                                           )
#define CREATE_OPCODE_2(vn, op, t, p1, p2) SAFE(\
                                                (vn) = new_node();\
                                                (vn)->kind = NODE_OPCODE_2;\
                                                INT_CREATE_OPCODE(vn, op, t);\
                                                (vn)->l.number = p1;\
                                                (vn)->r.number = p2;\
                                                )
#define CREATE_UNARY_OP_1(vn, op, t, x, p) SAFE(\
                                                (vn) = new_node();\
                                                (vn)->kind = NODE_UNARY_OP_1;\
                                                INT_CREATE_UNARY_OP(vn,\
                                                                    op, t, x);\
                                                (vn)->l.number = p;\
                                                )
#define CREATE_BINARY_OP_1(vn, op, t, x, y, p) SAFE(\
                                            (vn) = new_node();\
                                            (vn)->kind = NODE_BINARY_OP_1;\
                                            INT_CREATE_BINARY_OP(vn, op,\
                                                                 t, x, y);\
                                            (vn)->type = p;\
                                                    )
#define CREATE_TERNARY_OP_1(vn, op, t, x, y, z, p) SAFE(\
                                             (vn) = new_node();\
                                             (vn)->kind = NODE_TERNARY_OP_1;\
                                             INT_CREATE_TERNARY_OP(vn, op, t,\
                                                                   x, y, z);\
                                             (vn)->r.expr->type = p;\
                                             )
#define CREATE_RETURN(vn, val) SAFE(\
                                    (vn) = new_node_no_line();\
                                    (vn)->kind = NODE_RETURN;\
                                    (vn)->r.expr = val;\
                                    )
#define CREATE_LAND_LOR(vn, op, x, y) SAFE(\
                                           (vn) = new_node();\
                                           (vn)->kind = NODE_LAND_LOR;\
                                           (vn)->v.number = op;\
                                           (vn)->l.expr = x;\
                                           (vn)->r.expr = y;\
                                           (vn)->type = ((x->type == y->type)\
                                                         ? x->type : TYPE_ANY);\
                                           )
#define CREATE_CALL(vn, op, t, el) SAFE(\
                                        (vn) = el;\
                                        (vn)->kind = NODE_CALL;\
                                        (vn)->l.number = (vn)->v.number;\
                                        (vn)->v.number = op;\
                                        (vn)->type = t;\
                                        )
#define CREATE_STATEMENTS(vn, ln, rn) SAFE(\
                                           (vn) = new_node_no_line();\
                                           (vn)->kind = NODE_TWO_VALUES;\
                                           (vn)->l.expr = ln;\
                                           (vn)->r.expr = rn;\
                                           )
#define CREATE_TWO_VALUES(vn, t, ln, rn) SAFE(\
                                              CREATE_STATEMENTS(vn, ln, rn);\
                                              (vn)->type = t;\
                                              )
#define CREATE_CONTROL_JUMP(vn, op) SAFE(\
                                         (vn) = new_node_no_line();\
                                         (vn)->kind = NODE_CONTROL_JUMP;\
                                         (vn)->v.number = op;\
                                         )
#define CREATE_PARAMETER(vn, t, p) SAFE(\
                                        (vn) = new_node_no_line();\
                                        (vn)->kind = NODE_PARAMETER;\
                                        (vn)->type = t;\
                                        (vn)->v.number = p;\
                                        )
#define CREATE_IF(vn, c, s, e) SAFE(\
                                    (vn) = new_node_no_line();\
                                    (vn)->kind = NODE_IF;\
                                    (vn)->v.expr = c;\
                                    (vn)->l.expr = s;\
                                    (vn)->r.expr = (e);\
                                    )

#define CREATE_IGNORE(vn, c,s,e) SAFE(\
                                      (vn) = new_node_no_line();\
                                      (vn)->kind = NODE_IGNORE;\
                                      (vn)->v.expr = c;\
                                      (vn)->l.expr = s;\
                                      (vn)->r.expr = (e);\
                                      )

#define CREATE_LOOP(vn, tf, b, i, t) SAFE(\
                                          (vn) = new_node_no_line();\
                                          (vn)->kind = NODE_LOOP;\
                                          (vn)->type = tf;\
                                          (vn)->v.expr = b;\
                                          (vn)->l.expr = i;\
                                          (vn)->r.expr = t;\
                                      )
#define CREATE_LVALUE_EFUN(vn, t, lvl) SAFE(\
                                            (vn) = new_node();\
                                            (vn)->kind = NODE_LVALUE_EFUN;\
                                            (vn)->r.expr = lvl;\
                                            (vn)->type = t;\
                                            )
#define CREATE_FOREACH(vn, ln, rn) SAFE(\
                                        (vn) = new_node();\
                                        (vn)->kind = NODE_FOREACH;\
                                        (vn)->l.expr = ln;\
                                        (vn)->r.expr = rn;\
                                        )
#define CREATE_ERROR(vn) SAFE(\
                              (vn) = new_node_no_line();\
                              (vn)->kind = NODE_NUMBER;\
                              (vn)->type = TYPE_ANY;\
                              )
#define CREATE_REAL(vn, val) SAFE(\
                                  (vn) = new_node_no_line();\
                                  (vn)->kind = NODE_REAL;\
                                  (vn)->type = TYPE_REAL;\
                                  (vn)->v.real = val;\
                                  )
#define CREATE_NUMBER(vn, val) SAFE(\
                                    (vn) = new_node_no_line();\
                                    (vn)->kind = NODE_NUMBER;\
                                    (vn)->type = (val ? TYPE_NUMBER : TYPE_ANY);\
                                    (vn)->v.number = val;\
 )
#define CREATE_STRING(vn, val) SAFE(\
                                    (vn) = new_node_no_line();\
                                    (vn)->kind = NODE_STRING;\
                                    (vn)->type = TYPE_STRING;\
                                    (vn)->v.number = store_prog_string(val);\
                                    )
#define CREATE_EXPR_LIST(vn, pn) SAFE(\
                                      (vn) = new_node();\
                                      (vn)->v.number = (pn ? ((parse_node_t *)pn)->kind : 0);\
                                      (vn)->l.expr = (pn ? ((parse_node_t *)pn)->l.expr : (vn));\
                                      (vn)->r.expr = pn;\
                                      )
#define CREATE_EXPR_NODE(vn, pn, f) SAFE(\
                                         (vn) = new_node_no_line();\
                                         (vn)->v.expr = pn;\
                                         (vn)->l.expr = vn;\
                                         (vn)->r.expr = 0;\
                                         (vn)->type = f;\
                                         )
#define CREATE_CATCH(vn, pn) SAFE(\
                                  (vn) = new_node();\
                                  (vn)->kind = NODE_CATCH;\
                                  (vn)->type = TYPE_ANY;\
                                  (vn)->r.expr = pn;\
                                  )
#define CREATE_TIME_EXPRESSION(vn, pn) SAFE(\
                                          (vn) = new_node();\
                                          (vn)->kind = NODE_TIME_EXPRESSION;\
                                          (vn)->type = TYPE_ANY;\
                                          (vn)->r.expr = pn;\
                                          )

#define NODE_NO_LINE(x,y) SAFE(\
                               (x) = new_node_no_line();\
                               (x)->kind = y;\
                               )

/* tree functions */
void free_tree (void);
void release_tree (void);
void lock_expressions (void);
void unlock_expressions (void);
/* node functions */
parse_node_t *new_node (void);
parse_node_t *new_node_no_line (void);
parse_node_t *make_branched_node (short, char, 
                                       parse_node_t *, parse_node_t *);
/* parser grammar functions */
parse_node_t *binary_int_op (parse_node_t *, parse_node_t *, 
                                  char, const char *);
parse_node_t *make_range_node (int, parse_node_t *,
                                         parse_node_t *,
                                         parse_node_t *);
parse_node_t *insert_pop_value (parse_node_t *);
parse_node_t *pop_value (parse_node_t *);
parse_node_t *optimize_loop_test (parse_node_t *);
int is_boolean (parse_node_t *);

#endif
