#ifndef LPC_TO_C_H
#define LPC_TO_C_H

#include "lpc_incl.h"
#include "eoperators.h"
#include "efun_protos.h"

#ifdef LPC_TO_C

#include "cfuns.h"

#define C_STRING(x) push_string(current_prog->strings[x], STRING_SHARED)
#define C_AGGREGATE(x) lpc_arr = allocate_empty_array(x); for (lpc_int=x; lpc_int--; ) lpc_arr->item[lpc_int] = *sp--; (++sp)->type = T_ARRAY; sp->u.arr = lpc_arr
#define C_AGGREGATE_ASSOC(x) lpc_map = load_mapping_from_aggregate(sp -= x, x); (++sp)->type = T_MAPPING; sp->u.map = lpc_map
#define C_WHILE_DEC(x) if (fp[x].type == T_NUMBER) { lpc_int = fp[x].u.number--; } else if (fp[x].type == T_REAL) { lpc_int = fp[x].u.real--; } else error("-- of non-numeric argument\n"); 
#define C_LOOP_COND_LV(x, y) lpc_int = c_loop_cond_compare(&fp[x], &fp[y])
#define C_LOOP_COND_NUM(x, y) if (fp[x].type == T_NUMBER) { lpc_int = fp[x].u.number < y; } else if (fp[x].type == T_REAL) { lpc_int = fp[x].u.real < y; } else error("Right side of < is a number, left side is not.\n");
#define C_LOOP_INCR(x) if (fp[x].type == T_NUMBER) { fp[x].u.number++; } else if (fp[x].type == T_REAL) { fp[x].u.real++; } else { error("++ of non-numeric argument\n"); }
#define C_GLOBAL(x) lpc_svp = &current_object->variables[variable_index_offset + x]; if ((lpc_svp->type == T_OBJECT) && (lpc_svp->u.ob->flags & O_DESTRUCTED)) { *++sp = const0; assign_svalue(lpc_svp, &const0); } else assign_svalue_no_free(++sp, lpc_svp);
#define C_LOCAL(x) if ((fp[x].type == T_OBJECT) && (fp[x].u.ob->flags & O_DESTRUCTED)) { *++sp = const0; assign_svalue(fp + x, &const0); } else assign_svalue_no_free(++sp, fp + x);
#define C_LVALUE(x) (++sp)->type = T_LVALUE; sp->u.lvalue = x
#define C_LAND(x) if (sp->type == T_NUMBER) { if (!sp->u.number) x;  sp--; } else pop_stack()
#define C_LOR(x) if (sp->type == T_NUMBER && !sp->u.number) sp--; else x
#define C_BRANCH_WHEN_ZERO(x) if (sp->type == T_NUMBER) { if (!((sp--)->u.number)) x; } else pop_stack()
#define C_BRANCH_WHEN_NON_ZERO(x) if (sp->type != T_NUMBER || sp->u.number) { pop_stack(); x; } else sp--
#define C_BBRANCH_LT(x) f_lt(); if ((sp--)->u.number) x
#define C_CHECK_TRUE() if (sp->type != T_NUMBER) { lpc_int = 1; pop_stack(); } else lpc_int = (sp--)->u.number
#define BRANCH_LINK(x, y) if (x(sp->type == T_NUMBER && !sp->u.number)) { y; } pop_stack()
#define CHECK_SWITCHES if (switches_need_fixing) { fix_switches(string_switch_tables, NUM_STRING_SWITCHES); switches_need_fixing = 0; }

void c_efun_return PROT((int));

#endif

#endif

