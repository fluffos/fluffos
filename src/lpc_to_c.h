#ifndef CFUNS_H
#define CFUNS_H

#include "lpc_incl.h"

#ifdef LPC_TO_C

typedef struct {
    short index1, index2;
} range_switch_entry_t;

typedef struct {
    char *string;
    int index;
} string_switch_entry_t;

#define C_STRING(x) push_string(current_prog->strings[x], STRING_SHARED)
#define C_AGGREGATE(x) vec = allocate_empty_array(x); for (i=x; i--; ) vec->item[i] = *sp--; (++sp)->type = T_ARRAY; sp->u.arr = vec
#define C_AGGREGATE_ASSOC(x) map = load_mapping_from_aggregate(sp -= x, x); (++sp)->type = T_MAPPING; sp->u.map = map
#define C_WHILE_DEC(x) if (fp[x].type & T_NUMBER) { i = fp[x].u.number--; } else if (fp[x].type & T_REAL) { i = fp[x].u.real--; } else error("-- of non-numeric argument\n"); 
#define C_LOOP_COND_LV(x, y) i = c_loop_cond_compare(&fp[x], &fp[y])
#define C_LOOP_COND_NUM(x, y) if (fp[x].type == T_NUMBER) { i = fp[x].u.number < y; } else if (fp[x].type == T_REAL) { i = fp[x].u.real < y; } else error("Right side of < is a number, left side is not.\n");
#define C_LOOP_INCR(x) if (fp[x].type & T_NUMBER) { fp[x].u.number++; } else if (fp[x].type & T_REAL) { fp[x].u.real++; } else { error("++ of non-numeric argument\n"); }
#define C_GLOBAL(x) svp = &current_object->variables[variable_index_offset + x]; if ((svp->type & T_OBJECT) && (svp->u.ob->flags & O_DESTRUCTED)) { *++sp = const0; assign_svalue(svp, &const0); } else assign_svalue_no_free(++sp, svp);
#define C_LOCAL(x) if ((fp[x].type & T_OBJECT) && (fp[x].u.ob->flags & O_DESTRUCTED)) { *++sp = const0; assign_svalue(fp + x, &const0); } else assign_svalue_no_free(++sp, fp + x);
#define C_LVALUE(x) (++sp)->type = T_LVALUE; sp->u.lvalue = x
#define C_LAND(x) if (sp->type & T_NUMBER) { if (!sp->u.number) x;  sp--; } else pop_stack()
#define C_LOR(x) if (sp->type & T_NUMBER && !sp->u.number) sp--; else x
#define C_BRANCH_WHEN_ZERO(x) if (sp->type & T_NUMBER) { if (!((sp--)->u.number)) x; } else pop_stack()
#define C_BRANCH_WHEN_NON_ZERO(x) if (sp->type != T_NUMBER || sp->u.number) { pop_stack(); x; } else sp--
#define C_CHECK_TRUE() if (sp->type != T_NUMBER) { i = 1; pop_stack(); } else i = (sp--)->u.number
#define BRANCH_LINK(x, y) if (x(sp->type & T_NUMBER && !sp->u.number)) { y; } pop_stack()
#define CHECK_SWITCHES if (switches_need_fixing) { fix_switches(string_switch_tables, NUM_STRING_SWITCHES); switches_need_fixing = 0; }

void c_efun_return PROT((int));

#endif

/* these should really all be in one place */
static int i;
static array_t *vec;
static mapping_t *map;
static svalue_t *svp;

#endif

