#include "std.h"

#ifdef LPC_TO_C
#include "lpc_incl.h"
#include "backend.h"
#include "lpc_to_c.h"
#include "stralloc.h"
#include "eoperators.h"
#include "parse.h"
#include "qsort.h"

IF_DEBUG(extern int foreach_in_progress);

/* temporaries for LPC->C code */
int lpc_int;
svalue_t *lpc_svp;
array_t *lpc_arr;
mapping_t *lpc_map;

static svalue_t *lval;

void c_new_class P1(int, which) {
    array_t *cl;
    
    cl = allocate_class(&current_prog->classes[which]);
    push_refed_class(cl);
}

void c_member P1(int, idx) {
    array_t *arr;

    if (sp->type != T_CLASS)
        error("Tried to take a member of something that isn't a class.\n");
    arr = sp->u.arr;
    if (idx >= arr->size) error("Class has no corresponding member.\n");
    assign_svalue_no_free(sp, &arr->item[idx]);
    free_array(arr);
}

void c_member_lvalue P1(int, idx) {
    array_t *arr;

    if (sp->type != T_CLASS)
        error("Tried to take a member of something that isn't a class.\n");
    arr = sp->u.arr;
    if (idx >= arr->size) error("Class has no corresponding member.\n");
    sp->type = T_LVALUE;
    sp->u.lvalue = arr->item + idx;
    free_array(arr);
}

void c_return() {
    svalue_t sv;

    sv = *sp--;
    pop_n_elems(csp->num_local_variables);
    sp++;
    DEBUG_CHECK(sp != fp, "Bad stack at c_return\n");
    *sp =sv;
    pop_control_stack();
}

void c_return_zero() {
    pop_n_elems(csp->num_local_variables);
    sp++;
    DEBUG_CHECK(sp != fp, "Bad stack at c_return\n");
    *sp = const0;
    pop_control_stack();
}

void c_foreach P3(int, flags, int, idx1, int, idx2) {
    IF_DEBUG(foreach_in_progress++);
    
    if (flags & 4) {
	CHECK_TYPES(sp, T_MAPPING, 2, F_FOREACH);
	
	push_refed_array(mapping_indices(sp->u.map));
	(++sp)->type = T_NUMBER;
	sp->u.lvalue = (sp-1)->u.arr->item;
	sp->subtype = (sp-1)->u.arr->size;
		    
	(++sp)->type = T_LVALUE;
	if (flags & 2)
	    sp->u.lvalue = &current_object->variables[idx1 + variable_index_offset];
	else
	    sp->u.lvalue = fp + idx1;
    } else 
    if (sp->type == T_STRING) {
	(++sp)->type = T_NUMBER;
	sp->u.lvalue_byte = (unsigned char *)((sp-1)->u.string);
	sp->subtype = SVALUE_STRLEN(sp - 1);
    } else {
	CHECK_TYPES(sp, T_ARRAY, 2, F_FOREACH);

	(++sp)->type = T_NUMBER;
	sp->u.lvalue = (sp-1)->u.arr->item;
	sp->subtype = (sp-1)->u.arr->size;
    }

    (++sp)->type = T_LVALUE;
    if (flags & 1)
	sp->u.lvalue = &current_object->variables[idx2 + variable_index_offset];
    else
	sp->u.lvalue = fp + idx2;
}

void c_expand_varargs P1(int, where) {
    svalue_t *s, *t;
    array_t *arr;
    int n;
    
    s = sp - where;
    
    if (s->type != T_ARRAY)
	error("Item being expanded with ... is not an array\n");
		
    arr = s->u.arr;
    n = arr->size;
    num_varargs += n - 1;
    if (!n) {
	t = s;
	while (t < sp) {
	    *t = *(t + 1);
	    t++;
	}
	sp--;
    } else if (n == 1) {
	assign_svalue_no_free(s, &arr->item[0]);
    } else {
	t = sp;
	sp += n - 1;
	while (t > s) {
	    *(t + n - 1) = *t;
	    t--;
	}
	t = s + n - 1;
	if (arr->ref == 1) {
	    memcpy(s, arr->item, n * sizeof(svalue_t));
	    free_empty_array(arr);
	    return;
	} else {
	    while (n--)
		assign_svalue_no_free(t--, &arr->item[n]);
	}
    }
    free_array(arr);
}

void c_exit_foreach PROT((void)) {
    IF_DEBUG(foreach_in_progress--);
    if ((sp-1)->type == T_LVALUE) {
	/* mapping */
	sp -= 3;
	free_array((sp--)->u.arr);
	free_mapping((sp--)->u.map);
    } else {
	/* array or string */
	sp -= 2;
	if (sp->type == T_STRING)
	    free_string_svalue(sp--);
	else
	    free_array((sp--)->u.arr);
    }
}

int c_next_foreach PROT((void)) {
    if ((sp-1)->type == T_LVALUE) {
	/* mapping */
	if ((sp-2)->subtype--) {
	    svalue_t *key = (sp-2)->u.lvalue++;
	    svalue_t *value = find_in_mapping((sp-4)->u.map, key);
		    
	    assign_svalue((sp-1)->u.lvalue, key);
	    assign_svalue(sp->u.lvalue, value);
	    return 1;
	}
    } else {
	/* array or string */
	if ((sp-1)->subtype--) {
	    if ((sp-2)->type == T_STRING) {
		free_svalue(sp->u.lvalue, "string foreach");
		sp->u.lvalue->type = T_NUMBER;
		sp->u.lvalue->u.number = *((sp-1)->u.lvalue_byte)++;
	    } else {
		assign_svalue(sp->u.lvalue, (sp-1)->u.lvalue++);
	    }
	    return 1;
	}
    }
    c_exit_foreach();
    return 0;
}

void c_call_inherited P3(int, inh, int, func, int, num_arg) {
    inherit_t *ip = current_prog->inherit + inh;
    program_t *temp_prog = ip->prog;
    function_t *funp;
		
    funp = &temp_prog->functions[func];
		
    push_control_stack(FRAME_FUNCTION, funp);

    caller_type = ORIGIN_LOCAL;
    current_prog = temp_prog;
		
    csp->num_local_variables = num_arg + num_varargs;
    num_varargs = 0;
    		
    function_index_offset += ip->function_index_offset;
    variable_index_offset += ip->variable_index_offset;
    
    funp = setup_inherited_frame(funp);
    csp->pc = pc;

    call_program(current_prog, funp->offset);
}

void c_call P2(int, func, int, num_arg) {
    function_t *funp;

    func += function_index_offset;
    /*
     * Find the function in the function table. As the
     * function may have been redefined by inheritance, we
     * must look in the last table, which is pointed to by
     * current_object.
     */
    DEBUG_CHECK(func >= current_object->prog->num_functions,
		"Illegal function index\n");
    
    funp = &current_object->prog->functions[func];
    
    if (funp->flags & NAME_UNDEFINED)
	error("Undefined function: %s\n", funp->name);
    /* Save all important global stack machine registers */
    push_control_stack(FRAME_FUNCTION, funp);
    
    caller_type = ORIGIN_LOCAL;
    /* This assigment must be done after push_control_stack() */
    current_prog = current_object->prog;
    /*
     * If it is an inherited function, search for the real
     * definition.
     */
    csp->num_local_variables = num_arg + num_varargs;
    num_varargs = 0;
    function_index_offset = variable_index_offset = 0;
    funp = setup_new_frame(funp);
    csp->pc = pc;	/* The corrected return address */
    call_program(current_prog, funp->offset);
}

void c_efun_return P1(int, args) {
    svalue_t sv;
		
    sv = *sp--;
    pop_n_elems(args);
    *++sp = sv;
}

void c_void_assign() {
#ifdef DEBUG
    if (sp->type != T_LVALUE) fatal("Bad argument to F_VOID_ASSIGN\n");
#endif
    lval = (sp--)->u.lvalue;
    if (sp->type != T_INVALID){
	switch(lval->type){
	case T_LVALUE_BYTE:
	    {
		if (sp->type != T_NUMBER){
		    error("Illegal rhs to char lvalue\n");
		} else {
		    *global_lvalue_byte.u.lvalue_byte = (sp--)->u.number & 0xff;
		}
		break;
	    }
	    
	case T_LVALUE_RANGE:
	    {
		copy_lvalue_range(sp--);
		break;
	    }
	    
	default:
	    {
		free_svalue(lval, "F_VOID_ASSIGN : 3");
		*lval = *sp--;
	    }
	}
    } else sp--;
}

void c_post_dec() {
    DEBUG_CHECK(sp->type != T_LVALUE,
		"non-lvalue argument to --\n");
    lval = sp->u.lvalue;
    switch(lval->type) {
    case T_NUMBER:
	sp->type = T_NUMBER;
	sp->u.number = lval->u.number--;
	break;
    case T_REAL:
	sp->type = T_REAL;
	sp->u.real = lval->u.real--;
	break;
    case T_LVALUE_BYTE:
	sp->type = T_NUMBER;
	sp->u.number = (*global_lvalue_byte.u.lvalue_byte)--;
	break;
    default:
	error("-- of non-numeric argument\n");
    }
}

void c_post_inc() {
    DEBUG_CHECK(sp->type != T_LVALUE,
		"non-lvalue argument to ++\n");
    lval = sp->u.lvalue;
    switch (lval->type) {
    case T_NUMBER:
	sp->type = T_NUMBER;
	sp->u.number = lval->u.number++;
	break;
    case T_REAL:
	sp->type = T_REAL;
	sp->u.real = lval->u.real++;
	break;
    case T_LVALUE_BYTE:
	sp->type = T_NUMBER;
	sp->u.number = (*global_lvalue_byte.u.lvalue_byte)++;
	break;
    default:
	error("++ of non-numeric argument\n");
    }
}

void c_pre_dec() {
    svalue_t *lval;

    DEBUG_CHECK(sp->type != T_LVALUE, 
		"non-lvalue argument to --\n");
    lval = sp->u.lvalue;
    switch (lval->type) {
    case T_NUMBER:
	sp->type = T_NUMBER;
	sp->u.number = --(lval->u.number);
	break;
    case T_REAL:
	sp->type = T_REAL;
	sp->u.real = --(lval->u.real);
	break;
    case T_LVALUE_BYTE:
	sp->type = T_NUMBER;
	sp->u.number = --(*global_lvalue_byte.u.lvalue_byte);
	break;
    default:
	error("-- of non-numeric argument\n");
    }
}

void c_pre_inc() {
    svalue_t *lval;

    DEBUG_CHECK(sp->type != T_LVALUE,
		"non-lvalue argument to ++\n");
    lval = sp->u.lvalue;
    switch (lval->type) {
    case T_NUMBER:
	sp->type = T_NUMBER;
	sp->u.number = ++lval->u.number;
	break;
    case T_REAL:
	sp->type = T_REAL;
	sp->u.real = ++lval->u.number;
	break;
    case T_LVALUE_BYTE:
	sp->type = T_NUMBER;
	sp->u.number = ++*global_lvalue_byte.u.lvalue_byte;
	break;
    default:
	error("++ of non-numeric argument\n");
    }
}

void c_assign() {
#ifdef DEBUG
    if (sp->type != T_LVALUE) fatal("Bad argument to F_ASSIGN\n");
#endif
    switch(sp->u.lvalue->type){
    case T_LVALUE_BYTE:
	if ((sp - 1)->type != T_NUMBER) {
	    error("Illegal rhs to char lvalue\n");
	} else {
	    *global_lvalue_byte.u.lvalue_byte = ((sp - 1)->u.number & 0xff);
	}
	break;
    default:
	assign_svalue(sp->u.lvalue, sp - 1);
	break;
    case T_LVALUE_RANGE:
	assign_lvalue_range(sp - 1);
	break;
    }
    sp--;              /* ignore lvalue */
    /* rvalue is already in the correct place */
}

void c_void_assign_local P1(svalue_t *, var) {
    if (sp->type == T_INVALID) {
	sp--;
	return;
    }
    free_svalue(var, "c_void_assign_local");
    *var = *sp--;
}

void c_index() {
    int i;
    
    switch (sp->type) {
    case T_MAPPING:
	{
	    svalue_t *v;
	    mapping_t *m;
	    
	    v = find_in_mapping(m = sp->u.map, sp - 1);
	    assign_svalue(--sp, v);    /* v will always have a
					* value */
	    free_mapping(m);
	    break;
	}
    case T_BUFFER:
	{
	    if ((sp-1)->type != T_NUMBER)
		error("Indexing a buffer with an illegal type.\n");
	    
	    i = (sp - 1)->u.number;
	    if ((i > sp->u.buf->size) || (i < 0))
		error("Buffer index out of bounds.\n");
	    i = sp->u.buf->item[i];
	    free_buffer(sp->u.buf);
	    (--sp)->u.number = i;
	    break;
	}
    case T_STRING:
	{
	    if ((sp-1)->type != T_NUMBER) {
		error("Indexing a string with an illegal type.\n");
	    }
	    i = (sp - 1)->u.number;
	    if ((i > SVALUE_STRLEN(sp)) || (i < 0))
		error("String index out of bounds.\n");
	    i = (unsigned char) sp->u.string[i];
	    free_string_svalue(sp);
	    (--sp)->u.number = i;
	    break;
	}
    case T_ARRAY:
	{
	    array_t *arr;
	    
	    if ((sp-1)->type != T_NUMBER)
		error("Indexing an array with an illegal type\n");
	    i = (sp - 1)->u.number;
	    if (i<0) error("Negative index passed to array.\n");
	    arr = sp->u.arr;
	    if (i >= arr->size) error("Array index out of bounds.\n");
	    assign_svalue_no_free(--sp, &arr->item[i]);
	    free_array(arr);
	    break;
	}
    default:
	error("Indexing on illegal type.\n");
    }
    
    /*
     * Fetch value of a variable. It is possible that it is a
     * variable that points to a destructed object. In that case,
     * it has to be replaced by 0.
     */
    if (sp->type == T_OBJECT && (sp->u.ob->flags & O_DESTRUCTED)) {
	free_object(sp->u.ob, "F_INDEX");
	sp->type = T_NUMBER;
	sp->u.number = 0;
    }
}

void c_rindex() {
    int i;

    switch (sp->type) {
    case T_BUFFER:
	{
	    if ((sp-1)->type != T_NUMBER)
		error("Indexing a buffer with an illegal type.\n");
	    
	    i = sp->u.buf->size - (sp - 1)->u.number;
	    if ((i > sp->u.buf->size) || (i < 0))
		error("Buffer index out of bounds.\n");

	    i = sp->u.buf->item[i];
	    free_buffer(sp->u.buf);
	    (--sp)->u.number = i;
	    break;
	}
    case T_STRING:
	{
	    int len = SVALUE_STRLEN(sp);
	    if ((sp-1)->type != T_NUMBER) {
		error("Indexing a string with an illegal type.\n");
	    }
	    i = len - (sp - 1)->u.number;
	    if ((i > len) || (i < 0))
		error("String index out of bounds.\n");
	    i = (unsigned char) sp->u.string[i];
	    free_string_svalue(sp);
	    (--sp)->u.number = i;
	    break;
	}
    case T_ARRAY:
	{
	    array_t *vec = sp->u.arr;
	    
	    if ((sp-1)->type != T_NUMBER)
		error("Indexing an array with an illegal type\n");
	    i = vec->size - (sp - 1)->u.number;
	    if (i < 0 || i >= vec->size) error("Array index out of bounds.\n");
	    assign_svalue_no_free(--sp, &vec->item[i]);
	    free_array(vec);
	    break;
	}
    default:
	error("Indexing from the right on illegal type.\n");
    }
    
    /*
     * Fetch value of a variable. It is possible that it is a
     * variable that points to a destructed object. In that case,
     * it has to be replaced by 0.
     */
    if (sp->type == T_OBJECT && (sp->u.ob->flags & O_DESTRUCTED)) {
	free_object(sp->u.ob, "F_RINDEX");
	sp->type = T_NUMBER;
	sp->u.number = 0;
    }
}

void
c_functional P3(int, kind, int, num_arg, POINTER_INT, func) {
    funptr_t *fp;
    
    fp = (funptr_t *)DXALLOC(sizeof(funptr_hdr_t) + sizeof(functional_t),
			     TAG_FUNP, "c_functional");
    fp->hdr.owner = current_object;
    add_ref( current_object, "c_functional" );
    fp->hdr.type = kind;
    
    current_prog->func_ref++;
    
    fp->f.functional.prog = current_prog;
    fp->f.functional.offset = func;
    fp->f.functional.num_arg = num_arg;
    fp->f.functional.num_local = 0;
    fp->f.functional.fio = function_index_offset;
    fp->f.functional.vio = variable_index_offset;

    if (sp->type == T_ARRAY) {
	fp->hdr.args = sp->u.arr;
	fp->f.functional.num_arg += sp->u.arr->size;
    } else
	fp->hdr.args = 0;
    
    fp->hdr.ref = 1;

    sp->type = T_FUNCTION;
    sp->u.fp = fp;
}

void
c_anonymous P3(int, num_arg, int, num_local, POINTER_INT, func) {
    funptr_t *fp;
    
    fp = (funptr_t *)DXALLOC(sizeof(funptr_hdr_t) + sizeof(functional_t),
			     TAG_FUNP, "c_functional");
    fp->hdr.owner = current_object;
    add_ref( current_object, "c_functional" );
    fp->hdr.type = FP_FUNCTIONAL | FP_NOT_BINDABLE;
    
    current_prog->func_ref++;
    
    fp->f.functional.prog = current_prog;
    fp->f.functional.offset = func;
    fp->f.functional.num_arg = num_arg;
    fp->f.functional.num_local = num_local;
    fp->f.functional.fio = function_index_offset;
    fp->f.functional.vio = variable_index_offset;

    fp->hdr.args = 0;

    fp->hdr.ref = 1;

    (++sp)->type = T_FUNCTION;
    sp->u.fp = fp;
}

void
c_function_constructor P2(int, kind, int, arg)
{
    funptr_t *fp;

    switch (kind) {
    case FP_EFUN:
	fp = make_efun_funp(arg, sp);
	pop_stack();
	break;
    case FP_LOCAL:
	fp = make_lfun_funp(arg, sp); 
	pop_stack();
	break;
    case FP_SIMUL:
	fp = make_simul_funp(arg, sp); 
	pop_stack();
	break;
    case FP_FUNCTIONAL:
    case FP_FUNCTIONAL | FP_NOT_BINDABLE:
    case FP_ANONYMOUS:
	fatal("Wrong constructor called for LPC->C functional.\n");
    default:
	fatal("Tried to make unknown type of function pointer.\n");
    }
    push_refed_funp(fp);
}

void c_not() {
    if (sp->type == T_NUMBER)
	sp->u.number = !sp->u.number;
    else
	assign_svalue(sp, &const0);
}

void c_mod() {
    CHECK_TYPES(sp - 1, T_NUMBER, 1, F_MOD);
    CHECK_TYPES(sp, T_NUMBER, 2, F_MOD);
    if ((sp--)->u.number == 0)
	error("Modulus by zero.\n");
    sp->u.number %= (sp+1)->u.number;
}

void c_add_eq P1(int, is_void) {
    DEBUG_CHECK(sp->type != T_LVALUE,
		"non-lvalue argument to +=\n");
    lval = sp->u.lvalue;
    sp--; /* points to the RHS */
    switch (lval->type) {
    case T_STRING:
	if (sp->type == T_STRING) {
	    SVALUE_STRING_JOIN(lval, sp, "f_add_eq: 1");
	} else if (sp->type == T_NUMBER) {
	    char buff[20];
	    
	    sprintf(buff, "%d", sp->u.number);
	    EXTEND_SVALUE_STRING(lval, buff, "f_add_eq: 2");
	} else if (sp->type == T_REAL) {
	    char buff[40];
	    
	    sprintf(buff, "%f", sp->u.real);
	    EXTEND_SVALUE_STRING(lval, buff, "f_add_eq: 2");
	} else {
	    bad_argument(sp, T_STRING | T_NUMBER | T_REAL, 2,
			 (is_void ? F_VOID_ADD_EQ : F_ADD_EQ));
	}
	break;
    case T_NUMBER:
	if (sp->type == T_NUMBER) {
	    lval->u.number += sp->u.number;
	    /* both sides are numbers, no freeing required */
	} else if (sp->type == T_REAL) {
	    lval->u.number += sp->u.real;
	    /* both sides are numbers, no freeing required */
	} else {
	    error("Left hand side of += is a number (or zero); right side is not a number.\n");
	}
	break;
    case T_REAL:
	if (sp->type == T_NUMBER) {
	    lval->u.real += sp->u.number;
	    /* both sides are numerics, no freeing required */
	}
	if (sp->type == T_REAL) {
	    lval->u.real += sp->u.real;
	    /* both sides are numerics, no freeing required */
	} else {
	    error("Left hand side of += is a number (or zero); right side is not a number.\n");
	}
	break;
    case T_BUFFER:
	if (sp->type != T_BUFFER) {
	    bad_argument(sp, T_BUFFER, 2, (is_void ? F_VOID_ADD_EQ : F_ADD_EQ));
	} else {
	    buffer_t *b;
	    
	    b = allocate_buffer(lval->u.buf->size + sp->u.buf->size);
	    memcpy(b->item, lval->u.buf->item, lval->u.buf->size);
	    memcpy(b->item + lval->u.buf->size, sp->u.buf->item,
		   sp->u.buf->size);
	    free_buffer(sp->u.buf);
	    free_buffer(lval->u.buf);
	    lval->u.buf = b;
	}
	break;
    case T_ARRAY:
	if (sp->type != T_ARRAY)
	    bad_argument(sp, T_ARRAY, 2, (is_void ? F_VOID_ADD_EQ : F_ADD_EQ));
	else {
	    /* add_array now frees the arrays */
	    lval->u.arr = add_array(lval->u.arr, sp->u.arr);
	}
	break;
    case T_MAPPING:
	if (sp->type != T_MAPPING)
	    bad_argument(sp, T_MAPPING, 2, (is_void ? F_VOID_ADD_EQ : F_ADD_EQ));
	else {
	    absorb_mapping(lval->u.map, sp->u.map);
	    free_mapping(sp->u.map);	/* free RHS */
	    /* LHS not freed because its being reused */
	}
	break;
    case T_LVALUE_BYTE:
	if (sp->type != T_NUMBER)
	    error("Bad right type to += of char lvalue.\n");
	else *global_lvalue_byte.u.lvalue_byte += sp->u.number;
	break;
    default:
	bad_arg(1, (is_void ? F_VOID_ADD_EQ : F_ADD_EQ));
    }
    
    if (!is_void) {	/* not void add_eq */
	assign_svalue_no_free(sp, lval);
    } else {
	/*
	 * but if (void)add_eq then no need to produce an
	 * rvalue
	 */
	sp--;
    }
}

void c_divide() {
    switch((sp-1)->type|sp->type){
    case T_NUMBER:
	{
	    if (!(sp--)->u.number) error("Division by zero\n");
	    sp->u.number /= (sp+1)->u.number;
	    break;
	}
	
    case T_REAL:
	{
	    if ((sp--)->u.real == 0.0) error("Division by zero\n");
	    sp->u.real /= (sp+1)->u.real;
	    break;
	}
	
    case T_NUMBER|T_REAL:
	{
	    if ((sp--)->type == T_NUMBER){
		if (!((sp+1)->u.number)) error("Division by zero\n");
		sp->u.real /= (sp+1)->u.number;
	    } else {
		if ((sp+1)->u.real == 0.0) error("Division by 0.0\n");
		sp->type = T_REAL;
		sp->u.real = sp->u.number / (sp+1)->u.real;
	    }
	    break;
	}
	
    default:
	{
	    if (!((sp-1)->type & (T_NUMBER|T_REAL)))
		bad_argument(sp-1,T_NUMBER|T_REAL,1, F_DIVIDE);
	    if (!(sp->type & (T_NUMBER|T_REAL)))
		bad_argument(sp, T_NUMBER|T_REAL,2, F_DIVIDE);
	}
    }
}

void c_multiply() {
    switch((sp-1)->type|sp->type){
    case T_NUMBER:
	{
	    sp--;
	    sp->u.number *= (sp+1)->u.number;
	    break;
	}
	
    case T_REAL:
	{
	    sp--;
	    sp->u.real *= (sp+1)->u.real;
	    break;
	}
	
    case T_NUMBER|T_REAL:
	{
	    if ((--sp)->type == T_NUMBER){
		sp->type = T_REAL;
		sp->u.real = sp->u.number * (sp+1)->u.real;
	    }
	    else sp->u.real *= (sp+1)->u.number;
	    break;
	}
	
    case T_MAPPING:
	{
	    mapping_t *m;
	    m = compose_mapping((sp-1)->u.map, sp->u.map, 1);
	    pop_2_elems();
	    (++sp)->type = T_MAPPING;
	    sp->u.map = m;
	    break;
	}
	
    default:
	{
	    if (!((sp-1)->type & (T_NUMBER|T_REAL|T_MAPPING)))
		bad_argument(sp-1, T_NUMBER|T_REAL|T_MAPPING,1, F_MULTIPLY);
	    if (!(sp->type & (T_NUMBER|T_REAL|T_MAPPING)))
		bad_argument(sp, T_NUMBER|T_REAL|T_MAPPING,2, F_MULTIPLY);
	    error("Args to * are not compatible.\n");
	}
    }
    
}

void c_inc() {
    DEBUG_CHECK(sp->type != T_LVALUE,
		"non-lvalue argument to ++\n");
    lval = (sp--)->u.lvalue;
    switch (lval->type) {
    case T_NUMBER:
	lval->u.number++;
	break;
    case T_REAL:
	lval->u.real++;
	break;
    case T_LVALUE_BYTE:
	++*global_lvalue_byte.u.lvalue_byte;
	break;
    default:
	error("++ of non-numeric argument\n");
    }
}

void c_dec() {
    svalue_t *lval;

    DEBUG_CHECK(sp->type != T_LVALUE,
		"non-lvalue argument to --\n");
    lval = (sp--)->u.lvalue;
    switch (lval->type) {
    case T_NUMBER:
	lval->u.number--;
	break;
    case T_REAL:
	lval->u.real--;
	break;
    case T_LVALUE_BYTE:
	--(*global_lvalue_byte.u.lvalue_byte);
	break;
    default:
	error("-- of non-numeric argument\n");
    }
}

void c_le() {
    int i = sp->type;

    switch((--sp)->type|i){
    case T_NUMBER:
	sp->u.number = sp->u.number <= (sp+1)->u.number;
	break;
	
    case T_REAL:
	sp->u.number = sp->u.real <= (sp+1)->u.real;
	sp->type = T_NUMBER;
	break;
	
    case T_NUMBER|T_REAL:
	if (i == T_NUMBER){
	    sp->type = T_NUMBER;
	    sp->u.number = sp->u.real <= (sp+1)->u.number;
	} else sp->u.number = sp->u.number <= (sp+1)->u.real;
	break;
	
    case T_STRING:
	i = strcmp(sp->u.string, (sp+1)->u.string) <= 0;
	free_string_svalue(sp+1);
	free_string_svalue(sp);
	sp->type = T_NUMBER;
	sp->u.number = i;
	break;
	
    default:
	{
	    switch((sp++)->type){
	    case T_NUMBER:
	    case T_REAL:
		bad_argument(sp, T_NUMBER | T_REAL, 2, F_LE);
		
	    case T_STRING:
		bad_argument(sp, T_STRING, 2, F_LE);
		
	    default:
		bad_argument(sp - 1, T_NUMBER | T_STRING | T_REAL, 1, F_LE);
	    }
	}
    }
}

void c_lt() {
    int i = sp->type;
    switch (i | (--sp)->type) {
    case T_NUMBER:
	sp->u.number = sp->u.number < (sp+1)->u.number;
	break;
    case T_REAL:
	sp->u.number = sp->u.real < (sp+1)->u.real;
	sp->type = T_NUMBER;
	break;
    case T_NUMBER|T_REAL:
	if (i == T_NUMBER) {
	    sp->type = T_NUMBER;
	    sp->u.number = sp->u.real < (sp+1)->u.number;
	} else sp->u.number = sp->u.number < (sp+1)->u.real;
	break;
    case T_STRING:
	i = (strcmp((sp - 1)->u.string, sp->u.string) < 0);
	free_string_svalue(sp+1);
	free_string_svalue(sp);
	sp->type = T_NUMBER;
	sp->u.number = i;
	break;
    default:
	switch ((sp++)->type) {
	case T_NUMBER:
	case T_REAL:
	    bad_argument(sp, T_NUMBER | T_REAL, 2, F_LT);
	case T_STRING:
	    bad_argument(sp, T_STRING, 2, F_LT);
	default:
	    bad_argument(sp-1, T_NUMBER | T_STRING | T_REAL, 1, F_LT);
	}
    }
}

void c_gt() {
    int i = sp->type;
    switch ((--sp)->type | i) {
    case T_NUMBER:
	sp->u.number = sp->u.number > (sp+1)->u.number;
	break;
    case T_REAL:
	sp->u.number = sp->u.real > (sp+1)->u.real;
	sp->type = T_NUMBER;
	break;
    case T_NUMBER | T_REAL:
	if (i == T_NUMBER) {
	    sp->type = T_NUMBER;
	    sp->u.number = sp->u.real > (sp+1)->u.number;
	} else sp->u.number = sp->u.number > (sp+1)->u.real;
	break;
    case T_STRING:
	i = strcmp(sp->u.string, (sp+1)->u.string) > 0;
	free_string_svalue(sp+1);
	free_string_svalue(sp);
	sp->type = T_NUMBER;
	sp->u.number = i;
	break;
    default:
	{
	    switch ((sp++)->type) {
	    case T_NUMBER:
	    case T_REAL:
		bad_argument(sp, T_NUMBER | T_REAL, 2, F_GT);
	    case T_STRING:
		bad_argument(sp, T_STRING, 2, F_GT);
	    default:
		bad_argument(sp-1, T_NUMBER | T_REAL | T_STRING, 1, F_GT);
	    }
	}
    }
}

void c_ge() {
    int i = sp->type;
    switch ((--sp)->type | i) {
    case T_NUMBER:
	sp->u.number = sp->u.number >= (sp+1)->u.number;
	break;
    case T_REAL:
	sp->u.number = sp->u.real >= (sp+1)->u.real;
	sp->type = T_NUMBER;
	break;
    case T_NUMBER | T_REAL:
	if (i == T_NUMBER) {
	    sp->type = T_NUMBER;
	    sp->u.number = sp->u.real >= (sp+1)->u.number;
	} else sp->u.number = sp->u.number >= (sp+1)->u.real;
	break;
    case T_STRING:
	i = strcmp(sp->u.string, (sp+1)->u.string) >= 0;
	free_string_svalue(sp + 1);
	free_string_svalue(sp);
	sp->type = T_NUMBER;
	sp->u.number = i;
	break;
    default:
	{
	    switch ((sp++)->type) {
	    case T_NUMBER:
	    case T_REAL:
		bad_argument(sp, T_NUMBER | T_REAL, 2, F_GE);
	    case T_STRING:
		bad_argument(sp, T_STRING, 2, F_GE);
	    default:
		bad_argument(sp - 1, T_NUMBER | T_STRING | T_REAL, 1, F_GE);
	    }
	}
    }
}

void c_subtract() {
    int i = (sp--)->type;
    switch (i | sp->type) {
    case T_NUMBER:
	sp->u.number -= (sp+1)->u.number;
	break;
	
    case T_REAL:
	sp->u.real -= (sp+1)->u.real;
	break;
	
    case T_NUMBER | T_REAL:
	if (sp->type == T_REAL) sp->u.real -= (sp+1)->u.number;
	else {
	    sp->type = T_REAL;
	    sp->u.real = sp->u.number - (sp+1)->u.real;
	}
	break;
	
    case T_ARRAY:
	{
	    /*
	     * subtract_array already takes care of
	     * destructed objects
	     */
	    sp->u.arr = subtract_array(sp->u.arr, (sp+1)->u.arr);
	    break;
	}
	
    default:
	if (!((sp++)->type & (T_NUMBER|T_REAL|T_ARRAY)))
	    error("Bad left type to -.\n");
	else if (!(sp->type & (T_NUMBER|T_REAL|T_ARRAY)))
	    error("Bad right type to -.\n");
	else error("Arguments to - do not have compatible types.\n");
    }
}

void c_negate() {
    if (sp->type == T_NUMBER)
	sp->u.number = -sp->u.number;
    else if (sp->type == T_REAL)
	sp->u.real = -sp->u.real;
    else
	error("Bad argument to unary minus\n");
}

void c_compl() {
    if (sp->type != T_NUMBER)
	error("Bad argument to ~\n");
    sp->u.number = ~sp->u.number;
}

void c_add() {
    switch (sp->type) {
    case T_BUFFER:
	{
	    if (!((sp-1)->type == T_BUFFER)) {
		error("Bad type argument to +. Had %s and %s.\n",
		      type_name((sp - 1)->type), type_name(sp->type));
	    } else {
		buffer_t *b;
		
		b = allocate_buffer(sp->u.buf->size + (sp - 1)->u.buf->size);
		memcpy(b->item, (sp - 1)->u.buf->item, (sp - 1)->u.buf->size);
		memcpy(b->item + (sp - 1)->u.buf->size, sp->u.buf->item,
		       sp->u.buf->size);
		free_buffer((sp--)->u.buf);
		free_buffer(sp->u.buf);
		sp->u.buf = b;
	    }
	    break;
	} /* end of x + T_BUFFER */
    case T_NUMBER:
	{
	    switch ((--sp)->type) {
	    case T_NUMBER:
		sp->u.number += (sp+1)->u.number;
		break;
	    case T_REAL:
		sp->u.real += (sp+1)->u.number;
		break;
	    case T_STRING:
		{
		    char buff[20];
		    
		    sprintf(buff, "%d", (sp+1)->u.number);
		    EXTEND_SVALUE_STRING(sp, buff, "f_add: 2");
		    break;
		}
	    default:
		error("Bad type argument to +.  Had %s and %s.\n",
		      type_name(sp->type), type_name((sp+1)->type));
	    }
	    break;
	} /* end of x + NUMBER */
    case T_REAL:
	{
	    switch ((--sp)->type) {
	    case T_NUMBER:
		sp->type = T_REAL;
		sp->u.real = sp->u.number + (sp+1)->u.real;
		break;
	    case T_REAL:
		sp->u.real += (sp+1)->u.real;
		break;
	    case T_STRING:
		{
		    char buff[40];
		    
		    sprintf(buff, "%f", (sp+1)->u.real);
		    EXTEND_SVALUE_STRING(sp, buff, "f_add: 2");
		    break;
		}
	    default:
		error("Bad type argument to +. Had %s and %s\n",
		      type_name(sp->type), type_name((sp+1)->type));
	    }
	    break;
	} /* end of x + T_REAL */
    case T_ARRAY:
	{
	    if (!((sp-1)->type == T_ARRAY)) {
		error("Bad type argument to +. Had %s and %s\n",
		      type_name((sp - 1)->type), type_name(sp->type));
	    } else {
		/* add_array now free's the arrays */
		(sp-1)->u.arr = add_array((sp - 1)->u.arr, sp->u.arr);
		sp--;
		break;
	    }
	} /* end of x + T_ARRAY */
    case T_MAPPING:
	{
	    if ((sp-1)->type == T_MAPPING) {
		mapping_t *map;
		
		map = add_mapping((sp - 1)->u.map, sp->u.map);
		free_mapping((sp--)->u.map);
		free_mapping(sp->u.map);
		sp->u.map = map;
		break;
	    } else
		error("Bad type argument to +. Had %s and %s\n",
		      type_name((sp - 1)->type), type_name(sp->type));
	} /* end of x + T_MAPPING */
    case T_STRING:
	{
	    switch ((sp-1)->type) {
	    case T_NUMBER:
		{
		    char buff[20];
		    
		    sprintf(buff, "%d", (sp-1)->u.number);
		    SVALUE_STRING_ADD_LEFT(buff, "f_add: 3");
		    break;
		} /* end of T_NUMBER + T_STRING */
	    case T_REAL:
		{
		    char buff[40];
		    
		    sprintf(buff, "%f", (sp - 1)->u.real);
		    SVALUE_STRING_ADD_LEFT(buff, "f_add: 3");
		    break;
		} /* end of T_REAL + T_STRING */
	    case T_STRING:
		{
		    SVALUE_STRING_JOIN(sp-1, sp, "f_add: 1");
		    sp--;
		    break;
		} /* end of T_STRING + T_STRING */
	    default:
		error("Bad type argument to +. Had %s and %s\n",
		      type_name((sp - 1)->type), type_name(sp->type));
	    }
	    break;
	} /* end of x + T_STRING */
	
    default:
	error("Bad type argument to +.  Had %s and %s.\n",
	      type_name((sp-1)->type), type_name(sp->type));
    }
}

int c_loop_cond_compare P2(svalue_t *, s1, svalue_t *, s2) {
    switch (s1->type | s2->type) {
    case T_NUMBER: 
	return s1->u.number < s2->u.number;
    case T_REAL:
	return s1->u.real < s2->u.real;
    case T_STRING:
	return (strcmp(s1->u.string, s2->u.string) < 0);
    case T_NUMBER|T_REAL:
	if (s1->type == T_NUMBER) return s1->u.number < s2->u.real;
	else return s1->u.real < s2->u.number;
    default:
	if (s1->type == T_OBJECT && (s1->u.ob->flags & O_DESTRUCTED)){
	    free_object(s1->u.ob, "do_loop_cond:1");
	    *s1 = const0;
	}
	if (s2->type == T_OBJECT && (s2->u.ob->flags & O_DESTRUCTED)){
	    free_object(s2->u.ob, "do_loop_cond:2");
	    *s2 = const0;
	}
	if (s1->type == T_NUMBER && s2->type == T_NUMBER)
	    return 0;
	
	switch(s1->type){
	case T_NUMBER:
	case T_REAL:
	    error("2nd argument to < is not numeric when the 1st is.\n");
	case T_STRING:
	    error("2nd argument to < is not string when the 1st is.\n");
	default:
	    error("Bad 1st argument to <.\n");
	}
    }
    return 0;
}

void c_sscanf P1(int, num_arg) {
    svalue_t *fp;
    int i;

    /*
     * allocate stack frame for rvalues and return value (number of matches);
     * perform some stack manipulation; note: source and template strings are
     * already on the stack by this time
     */
    fp = sp;
    sp += num_arg + 1;
    *sp = *(fp--);		/* move format description to top of stack */
    *(sp - 1) = *(fp);		/* move source string just below the format
				 * desc. */
    fp->type = T_NUMBER;	/* this svalue isn't invalidated below, and
				 * if we don't change it to something safe,
				 * it will get freed twice if an error occurs */
    /*
     * prep area for rvalues
     */
    for (i = 1; i <= num_arg; i++)
	fp[i].type = T_INVALID;

    /*
     * do it...
     */
    i = inter_sscanf(sp - 2, sp - 1, sp, num_arg);

    /*
     * remove source & template strings from top of stack
     */
    pop_2_elems();

    /*
     * save number of matches on stack
     */
    fp->type = T_NUMBER;
    fp->u.number = i;
}

void c_parse_command P1(int, num_arg) {
    svalue_t *arg;
    svalue_t *fp;
    int i;

    /*
     * type checking on first three required parameters to parse_command()
     */
    arg = sp - 2;
    CHECK_TYPES(&arg[0], T_STRING, 1, F_PARSE_COMMAND);
    CHECK_TYPES(&arg[1], T_OBJECT | T_ARRAY, 2, F_PARSE_COMMAND);
    CHECK_TYPES(&arg[2], T_STRING, 3, F_PARSE_COMMAND);

    /*
     * allocate stack frame for rvalues and return value (number of matches);
     * perform some stack manipulation;
     */
    fp = sp;
    sp += num_arg + 1;
    arg = sp;
    *(arg--) = *(fp--);		/* move pattern to top of stack */
    *(arg--) = *(fp--);		/* move source object or array to just below 
				   the pattern */
    *(arg) = *(fp);		/* move source string just below the object */
    fp->type = T_NUMBER;

    /*
     * prep area for rvalues
     */
    for (i = 1; i <= num_arg; i++)
	fp[i].type = T_INVALID;

    /*
     * do it...
     */
    i = parse(arg[0].u.string, &arg[1], arg[2].u.string, &fp[1], num_arg);

    /*
     * remove mandatory parameters
     */
    pop_3_elems();

    /*
     * save return value on stack
     */
    fp->u.number = i;
}

void c_prepare_catch P1(error_context_t *, econ) {
    save_context(econ);
    push_control_stack(FRAME_CATCH, 0);
#if defined(DEBUG) || defined(TRACE_CODE)
    csp->num_local_variables = (csp - 1)->num_local_variables;	/* marion */
#endif
    assign_svalue(&catch_value, &const1);
}

void c_caught_error P1(error_context_t *, econ) {
    restore_context(econ);
    sp++;
    *sp = catch_value;
    catch_value = const1;

    /* if it's too deep or max eval, we can't let them catch it */
    pop_context(econ);
    if (max_eval_error)
	error("Can't catch eval cost too big error.\n");
    if (too_deep_error)
	error("Can't catch too deep recursion error.\n");
}
    
void c_end_catch P1(error_context_t *, econ) {
    free_svalue(&catch_value, "F_END_CATCH");
    catch_value = const0;
    /* We come here when no longjmp() was executed */
    pop_control_stack();
    push_number(0);
    pop_context(econ);
}

static int compare_switch_entries P2(string_switch_entry_t *, p1,
				     string_switch_entry_t *, p2) {
    return ((POINTER_INT)p1->string - (POINTER_INT)p2->string);
}

#ifdef DEBUGMALLOC_EXTENSIONS
typedef struct msl_s {
    struct msl_s *next;
    string_switch_entry_t **tables;
} msl_t;

static msl_t *g_msl_tables = 0;

static void add_switch_list P1(string_switch_entry_t **, tables) {
    msl_t *new;
        
    new = ALLOCATE(msl_t, TAG_DEBUGMALLOC, "add_switch_list");
    new->next = g_msl_tables;
    new->tables = tables;
}
	
void mark_switch_lists PROT((void)) {
    string_switch_entry_t *p, **tables;
    msl_t *msl = g_msl_tables;
    
    while (msl) {
	tables = msl->tables;
	msl = msl->next;
	while (*tables) {
	    p = *tables++;
	    while (p->string) {
		EXTRA_REF(BLOCK(p->string))++;
		p++;
	    }
	}
    }
}
#endif

void fix_switches P1(string_switch_entry_t **, tables) {
    string_switch_entry_t *p;

#ifdef DEBUGMALLOC_EXTENSIONS
    add_switch_list(tables);
#endif
    while (*tables) {
	p = *tables;
	while (p->string) {
	    p->string = make_shared_string(p->string);
	    p++;
	}
	quickSort((char *)(*tables), p - *tables , 
		  sizeof(string_switch_entry_t), compare_switch_entries);
	tables++;
    }
}

int c_string_switch_lookup P3(svalue_t *, str, string_switch_entry_t *, table,
			      int, table_size) {
    char *the_string;

    if (str->subtype == STRING_SHARED)
	the_string = str->u.string;
    else {
	if (!(the_string = findstring(str->u.string)))
	    return -1;
    }

    /* this should use a binary search, but for now ... */
    while (table->string) {
	if (the_string == table->string) return table->index;
	table++;
    }
    return -1;
}

void c_evaluate P1(int, num) {
    svalue_t *v;
    svalue_t *arg = sp - num + 1;

    if (arg->type != T_FUNCTION) {
	pop_n_elems(num-1);
	return;
    }
    if (current_object->flags & O_DESTRUCTED) {
	pop_n_elems(num);
	push_undefined();
	return;
    }
    v = call_function_pointer(arg->u.fp, num - 1);
    free_funp(arg->u.fp);
    assign_svalue_no_free(sp, v);
}

int c_range_switch_lookup P3(int, num, range_switch_entry_t *, table,
			     int, table_size) {
    /* this should also be a better search method */
    
    while (table->index2 != -2) {
	if (table->index2 == -1) {
	    if (table->index1 <= num && num <= (table+1)->index1)
		return (table+1)->index2;
	    table += 2;
	} else {
	    if (table->index1 == num) return table->index2;
	    table++;
	}
    }
    return 0;
}
#endif
