#ifndef LPC_TO_C_H
#define LPC_TO_C_H

#include "lpc_incl.h"
#include "eoperators.h"
#include "efun_protos.h"

#ifdef LPC_TO_C

#include "cfuns.h"

#define C_STRING(x) SAFE(\
			 (++sp)->type = T_STRING;\
			 sp->subtype = STRING_SHARED;\
			 sp->u.string = ref_string(current_prog->strings[x]);\
		     )
#define C_AGGREGATE(x) SAFE(\
			    lpc_arr = allocate_empty_array(x + num_varargs);\
			    num_varargs = 0;\
			    for (lpc_int=x + num_varargs; lpc_int--; )\
			    lpc_arr->item[lpc_int] = *sp--;\
			    (++sp)->type = T_ARRAY;\
			    sp->u.arr = lpc_arr;\
			    )
#define C_AGGREGATE_ASSOC(x) SAFE(\
				  lpc_map = load_mapping_from_aggregate(sp -= (x + num_varargs), (x+num_varargs));\
				  num_varargs = 0;\
				  (++sp)->type = T_MAPPING;\
				  sp->u.map = lpc_map;\
				  )
#define C_WHILE_DEC(x) SAFE(\
			    if (fp[x].type == T_NUMBER) {\
			        lpc_int = fp[x].u.number--;\
			    } else if (fp[x].type == T_REAL) {\
				lpc_int = fp[x].u.real--;\
			    } else error("-- of non-numeric argument\n");\
			    )
#define C_LOOP_COND_LV(x, y) lpc_int = c_loop_cond_compare(&fp[x], &fp[y])
#define C_LOOP_COND_NUM(x, y) SAFE(\
				   if (fp[x].type == T_NUMBER) {\
				       lpc_int = fp[x].u.number < y;\
				   } else if (fp[x].type == T_REAL) {\
				       lpc_int = fp[x].u.real < y;\
				   } else error("Right side of < is a number, left side is not.\n");\
				   )
#define C_LOOP_INCR(x) SAFE(\
			    if (fp[x].type == T_NUMBER) { fp[x].u.number++; }\
			    else if (fp[x].type == T_REAL) { fp[x].u.real++; }\
			    else { error("++ of non-numeric argument\n"); }\
			    )
#define C_GLOBAL(x) SAFE(\
			 lpc_svp = &current_object->variables[variable_index_offset + x];\
			 if ((lpc_svp->type == T_OBJECT) && (lpc_svp->u.ob->flags & O_DESTRUCTED)) {\
													*++sp = const0; assign_svalue(lpc_svp, &const0);\
			 } else assign_svalue_no_free(++sp, lpc_svp);\
			 )
#define C_LOCAL(x) SAFE(\
			if ((fp[x].type == T_OBJECT) &&\
			    (fp[x].u.ob->flags & O_DESTRUCTED)) {\
			    *++sp = const0;\
			    assign_svalue(fp + x, &const0);\
			} else assign_svalue_no_free(++sp, fp + x);\
			)
#define C_LVALUE(x) SAFE((++sp)->type = T_LVALUE; sp->u.lvalue = x;)
#define C_LAND(x) SAFE(\
		       if (sp->type == T_NUMBER) {\
			   if (!sp->u.number) x;  sp--;\
		       } else pop_stack();\
		       )
#define C_LOR(x) SAFE(if (sp->type == T_NUMBER && !sp->u.number) sp--; else x;)
#define C_BRANCH_WHEN_ZERO(x) SAFE(\
				   if (sp->type == T_NUMBER) {\
				       if (!((sp--)->u.number)) x;\
				   } else pop_stack();\
				   )
#define C_BRANCH_WHEN_NON_ZERO(x) SAFE(\
				       if (sp->type != T_NUMBER \
					   || sp->u.number) { pop_stack(); x;\
				       } else sp--;\
				       )
#define C_BBRANCH_LT(x) SAFE(f_lt(); if ((sp--)->u.number) x;)
#define BRANCH_LINK(x, y) SAFE(\
			       if (x(sp->type == T_NUMBER && !sp->u.number)) {\
				  y; } pop_stack();\
			       )

void c_efun_return PROT((int));

#endif

#endif

