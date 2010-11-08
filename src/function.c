#define SUPPRESS_COMPILER_INLINES
#include "std.h"
#include "lpc_incl.h"
#include "eoperators.h"
#include "compiler.h"
#include "replace_program.h"

INLINE void
dealloc_funp (funptr_t * fp)
{
    program_t *prog = 0;

    switch (fp->hdr.type) {
        case FP_LOCAL | FP_NOT_BINDABLE:
            if (fp->hdr.owner)
                prog = fp->hdr.owner->prog;
            break;
        case FP_FUNCTIONAL:
        case FP_FUNCTIONAL | FP_NOT_BINDABLE:
            prog = fp->f.functional.prog;
            break;
    }

    if (fp->hdr.owner)
        free_object(&fp->hdr.owner, "free_funp");
    if (fp->hdr.args)
        free_array(fp->hdr.args);

    if (prog) {
        prog->func_ref--;
        debug(d_flag, ("subtr func ref /%s: now %i\n",
                    prog->filename, prog->func_ref));
        if (!prog->func_ref && !prog->ref)
            deallocate_program(prog);
    }

    FREE(fp);
}

INLINE void
free_funp (funptr_t * fp)
{
    fp->hdr.ref--;
    if (fp->hdr.ref > 0) {
        return;
    }
    dealloc_funp(fp);
}

INLINE void
push_refed_funp (funptr_t * fp)
{
    STACK_INC;
    sp->type = T_FUNCTION;
    sp->u.fp = fp;
}

INLINE void
push_funp (funptr_t * fp)
{
    STACK_INC;
    sp->type = T_FUNCTION;
    sp->u.fp = fp;
    fp->hdr.ref++;
}

/* num_arg args are on the stack, and the args from the array vec should be
 * put in front of them.  This is so that the order of arguments is logical.
 * 
 * evaluate( (: f, a :), b) -> f(a,b) and not f(b, a) which would happen
 * if we simply pushed the args from vec at this point.  (Note that the
 * old function pointers are broken in this regard)
 */
int merge_arg_lists (int num_arg, array_t * arr, int start) {
    int num_arr_arg = arr->size - start;
    svalue_t *sptr;
    
    if (num_arr_arg) {
        CHECK_STACK_OVERFLOW(num_arr_arg);
        sptr = (sp += num_arr_arg);
        if (num_arg) {
            /* We need to do some stack movement so that the order
               of arguments is logical */
            while (num_arg--) {
                *sptr = *(sptr - num_arr_arg);
                sptr--;
            }
        }
        num_arg = arr->size;
        while (--num_arg >= start)
            assign_svalue_no_free(sptr--, &arr->item[num_arg]);
        /* could just return num_arr_arg if num_arg is 0 but .... -Sym */
        return (sp - sptr);
    }       
    return num_arg;
}

INLINE funptr_t *
make_efun_funp (int opcode, svalue_t * args)
{
    funptr_t *fp;
    
    fp = (funptr_t *)DXALLOC(sizeof(funptr_t),
                             TAG_FUNP, "make_efun_funp");
    fp->hdr.owner = current_object;
    add_ref( current_object, "make_efun_funp" );
    fp->hdr.type = FP_EFUN;
    
    fp->f.efun.index = opcode;
    
    if (args->type == T_ARRAY) {
        fp->hdr.args = args->u.arr;
        args->u.arr->ref++;
    } else
        fp->hdr.args = 0;
    
    fp->hdr.ref = 1;
    return fp;
}

INLINE funptr_t *
make_lfun_funp (int index, svalue_t * args)
{
    funptr_t *fp;
    int newindex;

    if (replace_program_pending(current_object))
        error("cannot bind an lfun fp to an object with a pending replace_program()\n");

    fp = (funptr_t *)DXALLOC(sizeof(funptr_hdr_t) + sizeof(local_ptr_t),
                             TAG_FUNP, "make_lfun_funp");
    fp->hdr.owner = current_object;
    add_ref( current_object, "make_lfun_funp" );
    fp->hdr.type = FP_LOCAL | FP_NOT_BINDABLE;
    
    fp->hdr.owner->prog->func_ref++;
    debug(d_flag, ("add func ref /%s: now %i\n",
                fp->hdr.owner->prog->filename,
                fp->hdr.owner->prog->func_ref));
    
    newindex = index + function_index_offset;
    if (current_object->prog->function_flags[newindex] & FUNC_ALIAS)
        newindex = current_object->prog->function_flags[newindex] & ~FUNC_ALIAS;
    fp->f.local.index = newindex;
    
    if (args->type == T_ARRAY) {
        fp->hdr.args = args->u.arr;
        args->u.arr->ref++;
    } else
        fp->hdr.args = 0;
    
    fp->hdr.ref = 1;
    return fp;
}

INLINE funptr_t *
make_simul_funp (int index, svalue_t * args)
{
    funptr_t *fp;
    
    fp = (funptr_t *)DXALLOC(sizeof(funptr_hdr_t) + sizeof(simul_ptr_t),
                             TAG_FUNP, "make_simul_funp");
    fp->hdr.owner = current_object;
    add_ref( current_object, "make_simul_funp" );
    fp->hdr.type = FP_SIMUL;
    
    fp->f.simul.index = index;
    
    if (args->type == T_ARRAY) {
        fp->hdr.args = args->u.arr;
        args->u.arr->ref++;
    } else
        fp->hdr.args = 0;
    
    fp->hdr.ref = 1;
    return fp;
}

INLINE funptr_t *
make_functional_funp (short num_arg, short num_local, short len, svalue_t * args, int flag)
{
    funptr_t *fp;

    if (replace_program_pending(current_object))
        error("cannot bind a functional to an object with a pending replace_program()\n");
    
    fp = (funptr_t *)DXALLOC(sizeof(funptr_hdr_t) + sizeof(functional_t), 
                             TAG_FUNP, "make_functional_funp");
    fp->hdr.owner = current_object;
    add_ref( current_object, "make_functional_funp" );
    fp->hdr.type = FP_FUNCTIONAL + flag;
    
    current_prog->func_ref++;
    debug(d_flag, ("add func ref /%s: now %i\n",
               current_prog->filename,
               current_prog->func_ref));
    
    fp->f.functional.prog = current_prog;
    fp->f.functional.offset = pc - current_prog->program;
    fp->f.functional.num_arg = num_arg;
    fp->f.functional.num_local = num_local;
    fp->f.functional.fio = function_index_offset;
    fp->f.functional.vio = variable_index_offset;
    pc += len;
    
    if (args && args->type == T_ARRAY) {
        fp->hdr.args = args->u.arr;
        args->u.arr->ref++;
        fp->f.functional.num_arg += args->u.arr->size;
    } else
        fp->hdr.args = 0;
    
    fp->hdr.ref = 1;
    return fp;
}

typedef void (*func_t) (void);
extern func_t efun_table[];

svalue_t *
call_function_pointer (funptr_t * funp, int num_arg)
{
    static func_t *oefun_table = efun_table - BASE;
    array_t *v;
    int extfr = 1;
    
    if (!funp->hdr.owner || (funp->hdr.owner->flags & O_DESTRUCTED))
        error("Owner (/%s) of function pointer is destructed.\n",
              (funp->hdr.owner ? funp->hdr.owner->obname : "(null)"));
    setup_fake_frame(funp);
    if ((v=funp->hdr.args)) {
        check_for_destr(v);
        num_arg = merge_arg_lists(num_arg, v, 0);
    }

    switch (funp->hdr.type) {
    case FP_SIMUL:
        call_simul_efun(funp->f.simul.index, num_arg);
        break;
    case FP_EFUN:
        {
            int i, def;
            fp = sp - num_arg + 1;

            i = funp->f.efun.index;
            if (num_arg == instrs[i].min_arg - 1 && 
                ((def = instrs[i].Default) != DEFAULT_NONE)) {
                if (def == DEFAULT_THIS_OBJECT) {
                    push_object(current_object);
                } else {
                    push_number(def);
                }
                num_arg++;
            } else
                if (num_arg < instrs[i].min_arg) {
                    error("Too few arguments to efun %s in efun pointer.\n", query_instr_name(i));
                } else if (num_arg > instrs[i].max_arg && instrs[i].max_arg != -1) {
                    error("Too many arguments to efun %s in efun pointer.\n", query_instr_name(i));
                }
            /* possibly we should add TRACE, OPC, etc here;
               also on eval_cost here, which is ok for just 1 efun */
            {
                int j, n = num_arg;
                st_num_arg = num_arg;

                if (n >= 4 || instrs[i].max_arg == -1)
                    n = instrs[i].min_arg;

                for (j = 0; j < n; j++) {
                    CHECK_TYPES(sp - num_arg + j + 1, instrs[i].type[j], j + 1, i);
                }
                (*oefun_table[i])();

                free_svalue(&apply_ret_value, "call_function_pointer");
                if (instrs[i].ret_type == TYPE_NOVALUE)
                    apply_ret_value = const0;
                else
                    apply_ret_value = *sp--;
               	remove_fake_frame();
                return &apply_ret_value;
            }
        }
    case FP_LOCAL | FP_NOT_BINDABLE: {
        function_t *func;

        fp = sp - num_arg + 1;

        if (current_object->prog->function_flags[funp->f.local.index] & (FUNC_PROTOTYPE|FUNC_UNDEFINED))
            error("Undefined lfun pointer called: %s\n", function_name(current_object->prog, funp->f.local.index));
        push_control_stack(FRAME_FUNCTION);
        current_prog = funp->hdr.owner->prog;
        
        caller_type = ORIGIN_LOCAL;

        csp->num_local_variables = num_arg;
        func = setup_new_frame(funp->f.local.index);

        call_program(current_prog, func->address);
        break;
    }
    case FP_FUNCTIONAL: 
    case FP_FUNCTIONAL | FP_NOT_BINDABLE: {

        fp = sp - num_arg + 1;
        push_control_stack(FRAME_FUNP);
        current_prog = funp->f.functional.prog;
        csp->fr.funp = funp;
        
        caller_type = ORIGIN_FUNCTIONAL;

        setup_variables(num_arg, funp->f.functional.num_local,
                        funp->f.functional.num_arg);

        function_index_offset = funp->f.functional.fio;
        variable_index_offset = funp->f.functional.vio;
        call_program(funp->f.functional.prog, funp->f.functional.offset);
        break;
    }
    default:
        error("Unsupported function pointer type.\n");
    }
    free_svalue(&apply_ret_value, "call_function_pointer");
    apply_ret_value = *sp--;
    remove_fake_frame();
    return &apply_ret_value;
}

svalue_t *
safe_call_function_pointer (funptr_t * funp, int num_arg)
{
    error_context_t econ;
    svalue_t *ret;

    if (!save_context(&econ))
        return 0;
    if (!SETJMP(econ.context)) {
        ret = call_function_pointer(funp, num_arg);
    } else {
        restore_context(&econ);
        /* condition was restored to where it was when we came in */
        pop_n_elems(num_arg);
        ret = 0;
    }
    pop_context(&econ);
    return ret;
}
