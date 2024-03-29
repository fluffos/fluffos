---
layout: doc
title: general / parse_tree
---
# parse_tree

## NODE_TERNARY_OP

    l.expr: first arg
    r.expr: NODE_BINARY_OP

## NODE_BINARY_OP

    v.number: opcode
    l.expr, r.expr: arguments

## NODE_UNARY_OP

    v.number: opcode
    r.expr: argument

## NODE_OPCODE

    v.number: opcode

## NODE_OPCODE_1

    v.number: opcode
    l.number: param

## NODE_OPCODE_2

    v.number: opcode
    l.number: param1
    r.number: param2

## NODE_UNARY_OP_1

    r.expr: argument
    v.number: opcode
    l.number: param

## NODE_BINARY_OP_1

    r.expr, l.expr: args
    v.number: opcode
    type: param

## NODE_TERNARY_OP_1

    l.expr: first arg
    r.expr: NODE_BINARY_OP
    type: param

## NODE_RETURN

    r.expr: return value (may be zero)

## NODE_LAND_LOR

    l.expr, r.expr: expressions
    v.number: F_LAND, F_LOR

## NODE_BRANCH_LINK

    l.expr, r.expr: expressions

## NODE_CALL, NODE_CALL_1

    r.expr: expr_list
    v.number: opcode
    l.number: param (short)

## NODE_CALL_2

    r.expr: expr_list
    v.number: opcode
    l.number: param2 << 16 + param (short)

## NODE_TWO_VALUES

    l.expr, r.expr: two values ;)

## NODE_CONTROL_JUMP

    v.number: (CJ_BREAK, CJ_BREAK_SWITCH, CJ_CONTINUE)

## NODE_PARAMETER

    v.number: index

## NODE_PARAMETER_LVALUE

    v.number: index

## NODE_IF

    v.expr: condition
    l.expr: statement
    r.expr: else statement

## NODE_LOOP

    type: if non-zero, do test at top of loop
    v.expr: block
    l.expr: inc
    r.expr: test

## NODE_FOREACH

    l.expr: first var
    r.expr: second var, or zero
    v.expr: the expression

## NODE_LVALUE_EFUN

    l.expr: binary/ternary expression with builtin args
    r.expr: lvalue list

## NODE_EFUN

    r.expr: expr_list
    v.number: opcode ( + 1024 if void valued efun is used)
    l.number: num args
