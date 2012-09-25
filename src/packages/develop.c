#ifdef LATTICE
#include "/lpc_incl.h"
#include "/comm.h"
#include "/md.h"
#include "/sprintf.h"
#else
#include "../lpc_incl.h"
#include "../comm.h"
#include "../md.h"
#include "../sprintf.h"
#include "../efun_protos.h"
#endif

static object_t *ob;

#ifdef F_DEBUG_INFO
void
f_debug_info (void)
{
    svalue_t *arg;
    outbuffer_t out;

    outbuf_zero(&out);
    arg = sp - 1;
    switch (arg[0].u.number) {
    case 0:
        {
            int i, flags;
            object_t *obj2;

            ob = arg[1].u.ob;
            flags = ob->flags;
           outbuf_addv(&out, "O_HEART_BEAT      : %s\n",
                        flags & O_HEART_BEAT ? "TRUE" : "FALSE");
#ifndef NO_WIZARDS
           outbuf_addv(&out, "O_IS_WIZARD       : %s\n",
                        flags & O_IS_WIZARD ? "TRUE" : "FALSE");
#endif
#ifdef NO_ADD_ACTION
           outbuf_addv(&out, "O_LISTENER        : %s\n",
                        flags & O_LISTENER ? "TRUE" : "FALSE");
#else
           outbuf_addv(&out, "O_ENABLE_COMMANDS : %s\n",
                        flags & O_ENABLE_COMMANDS ? "TRUE" : "FALSE");
#endif
           outbuf_addv(&out, "O_CLONE           : %s\n",
                        flags & O_CLONE ? "TRUE" : "FALSE");
           outbuf_addv(&out, "O_VIRTUAL         : %s\n",
                        flags & O_VIRTUAL ? "TRUE" : "FALSE");
           outbuf_addv(&out, "O_DESTRUCTED      : %s\n",
                        flags & O_DESTRUCTED ? "TRUE" : "FALSE");
           outbuf_addv(&out, "O_ONCE_INTERACTIVE: %s\n",
                        flags & O_ONCE_INTERACTIVE ? "TRUE" : "FALSE");
           outbuf_addv(&out, "O_RESET_STATE     : %s\n",
                        flags & O_RESET_STATE ? "TRUE" : "FALSE");
           outbuf_addv(&out, "O_WILL_CLEAN_UP   : %s\n",
                        flags & O_WILL_CLEAN_UP ? "TRUE" : "FALSE");
           outbuf_addv(&out, "O_WILL_RESET      : %s\n",
                        flags & O_WILL_RESET ? "TRUE" : "FALSE");
#ifdef HAVE_ZLIB
           if (ob->interactive) {
             outbuf_addv(&out, "O_COMPRESSED      : %s\n",
                         ob->interactive->compressed_stream ? "TRUE" :
                         "FALSE");
             outbuf_addv(&out, "O_ZMP             : %s\n",
                                      ob->interactive->iflags & USING_ZMP ? "TRUE" :
                                      "FALSE");
             outbuf_addv(&out, "O_GMCP            : %s\n",
                                      ob->interactive->iflags & USING_GMCP ? "TRUE" :
                                      "FALSE");
             outbuf_addv(&out, "O_MXP             : %s\n",
                                      ob->interactive->iflags & USING_MXP ? "TRUE" :
                                      "FALSE");
           }
#endif
           
#ifndef NO_LIGHT
           outbuf_addv(&out, "total light : %d\n", ob->total_light);
#endif
#ifndef NO_RESETS
           outbuf_addv(&out, "next_reset  : %d\n", ob->next_reset);
#endif
           outbuf_addv(&out, "time_of_ref : %d\n", ob->time_of_ref);
           outbuf_addv(&out, "ref         : %d\n", ob->ref);
#ifdef DEBUG
           outbuf_addv(&out, "extra_ref   : %d\n", ob->extra_ref);
#endif
           outbuf_addv(&out, "name        : '/%s'\n", ob->obname);
           outbuf_addv(&out, "next_all    : OBJ(/%s)\n",
                        ob->next_all ? ob->next_all->obname : "NULL");
            if (obj_list == ob)
                outbuf_add(&out, "This object is the head of the object list.\n");
            for (obj2 = obj_list, i = 1; obj2; obj2 = obj2->next_all, i++)
                if (obj2->next_all == ob) {
                   outbuf_addv(&out, "Previous object in object list: OBJ(/%s)\n",
                                obj2->obname);
                   outbuf_addv(&out, "position in object list:%d\n", i);
                }
            break;
        }
    case 1:
        ob = arg[1].u.ob;

        outbuf_addv(&out, "program ref's %d\n", ob->prog->ref);
        outbuf_addv(&out, "Name /%s\n", ob->prog->filename);
        outbuf_addv(&out, "program size %d\n",
                    ob->prog->program_size);
        outbuf_addv(&out, "function flags table %d (%d) \n", 
                    ob->prog->last_inherited + ob->prog->num_functions_defined,
                    (ob->prog->last_inherited + ob->prog->num_functions_defined)* sizeof(unsigned short));
        outbuf_addv(&out, "compiler function table %d (%d) \n", 
                    ob->prog->num_functions_defined,
                    ob->prog->num_functions_defined * sizeof(function_t));
        outbuf_addv(&out, "num strings %d\n", ob->prog->num_strings);
        outbuf_addv(&out, "num vars %d (%d)\n", ob->prog->num_variables_defined,
                    ob->prog->num_variables_defined * (sizeof(char *) + sizeof(short)));
        outbuf_addv(&out, "num inherits %d (%d)\n", ob->prog->num_inherited,
                    ob->prog->num_inherited * sizeof(inherit_t));
        outbuf_addv(&out, "total size %d\n", ob->prog->total_size);
        break;
    case 2:
        {
            int i;
            ob = arg[1].u.ob;
            for (i=0; i<ob->prog->num_variables_total; i++) {
                /* inefficient, but: */
                outbuf_addv(&out, "%s: ", variable_name(ob->prog, i));
                svalue_to_string(&ob->variables[i], &out, 2, 0, 0);
                outbuf_add(&out, "\n");
            }
            break;
        }       
    default:
        bad_arg(1, F_DEBUG_INFO);
    }
    pop_stack();
    pop_stack();
    outbuf_push(&out);
}
#endif

#ifdef F_REFS
void
f_refs (void)
{
    int r;

    switch (sp->type) {
    case T_MAPPING:
        r = sp->u.map->ref;
        break;
    case T_CLASS:
    case T_ARRAY:
        r = sp->u.arr->ref;
        break;
    case T_OBJECT:
        r = sp->u.ob->ref;
        break;
    case T_FUNCTION:
        r = sp->u.fp->hdr.ref;
        break;
#ifndef NO_BUFFER_TYPE
    case T_BUFFER:
        r = sp->u.buf->ref;
        break;
#endif
    case T_STRING:
      if(sp->subtype & STRING_COUNTED)
	r = MSTR_REF(sp->u.string);
      else
	r = 0;
      break;
    default:
        r = 0;
        break;
    }
    free_svalue(sp, "f_refs");
    put_number(r - 1);          /* minus 1 to compensate for being arg of
                                 * refs() */
}
#endif

#ifdef F_DESTRUCTED_OBJECTS
void f_destructed_objects (void)
{
    int i;
    array_t *ret;
    object_t *ob;

    ret = allocate_empty_array(tot_dangling_object);
    ob = obj_list_dangling;

    for (i = 0;  i < tot_dangling_object;  i++) {
        ret->item[i].type = T_ARRAY;
        ret->item[i].u.arr = allocate_empty_array(2);
        ret->item[i].u.arr->item[0].type = T_STRING;
        ret->item[i].u.arr->item[0].subtype = STRING_SHARED;
        ret->item[i].u.arr->item[0].u.string = make_shared_string(ob->obname);
        ret->item[i].u.arr->item[1].type = T_NUMBER;
        ret->item[i].u.arr->item[1].u.number = ob->ref;

        ob = ob->next_all;
    }

    push_refed_array(ret);
}
#endif

#if (defined(DEBUGMALLOC) && defined(DEBUGMALLOC_EXTENSIONS))
#ifdef F_DEBUGMALLOC
void
f_debugmalloc (void)
{
    char *res;
    
    res = dump_debugmalloc((sp - 1)->u.string, sp->u.number);
    free_string_svalue(--sp);
    sp->subtype = STRING_MALLOC;
    sp->u.string = res;
}
#endif

#ifdef F_SET_MALLOC_MASK
void
f_set_malloc_mask (void)
{
    set_malloc_mask((sp--)->u.number);
}
#endif

#ifdef F_CHECK_MEMORY
void
f_check_memory (void)
{
    check_all_blocks((sp--)->u.number);
}
#endif
#endif                          /* (defined(DEBUGMALLOC) &&
                                 * defined(DEBUGMALLOC_EXTENSIONS)) */

#ifdef F_TRACE
void
f_trace (void)
{
    int ot = -1;

    if (command_giver && command_giver->interactive) {
        ot = command_giver->interactive->trace_level;
        command_giver->interactive->trace_level = sp->u.number;
    }
    sp->u.number = ot;
}
#endif

#ifdef F_TRACEPREFIX
void
f_traceprefix (void)
{
    char *old = 0;

    if (command_giver && command_giver->interactive) {
        old = command_giver->interactive->trace_prefix;
        if (sp->type & T_STRING) {
            const char *p = sp->u.string;
            if (*p == '/') p++;
            
            command_giver->interactive->trace_prefix = make_shared_string(p);
            free_string_svalue(sp);
        } else
            command_giver->interactive->trace_prefix = 0;
    }
    if (old) {
        put_malloced_string(add_slash(old));
        free_string(old);
    } else 
        *sp = const0;
}
#endif

