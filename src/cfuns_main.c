#include "efuns.h"
#include "applies.h"
#include "stralloc.h"
#if defined(__386BSD__) || defined(SunOS_5)
#include <unistd.h>
#endif

#ifdef LPC_TO_C
#include "cfuns.h"

static struct object *ob;

extern int using_bsd_malloc, using_smalloc;


#ifdef F_ADD_ACTION
void c_add_action P4(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2)
{
    if (s2) {
	if (s2->type != T_NUMBER)
	    bad_arg(3, F_ADD_ACTION);
    }
    if (s1 && s1->type == T_POINTER) {
	int i,n;
	n = s1->u.vec->size;
	for (i=0; i<n; i++) {
	    if (s1->u.vec->item[i].type == T_STRING) {
		add_action(s0->u.string,
			   s1->u.vec->item[i].u.string,
			   s2 ? s2->u.number : 0);
	    }
	}
    } else {
	add_action(s0->u.string,
		   s1 ? s1->u.string : 0,
		   s2 ? s2->u.number : 0);
    }
}
#endif

#ifdef F_ALL_INVENTORY
void c_all_inventory P2(svalue *, ret, svalue *, s0)
{
    struct vector *vec;

    vec = all_inventory(s0->u.ob, 0);
    if (!vec) {
	C_NUMBER(ret, 0);
	return;
    }
    C_REFED_VECTOR(ret, vec);
}
#endif

#ifdef F_ALLOCATE
void c_allocate P2(svalue *, ret, svalue *, s0)
{
    struct vector *vec;

    vec = allocate_array(s0->u.number);
    C_REFED_VECTOR(ret, vec);
}
#endif

#ifdef F_ALLOCATE_BUFFER
void c_allocate_buffer P2(svalue *, ret, svalue *, s0)
{
    struct buffer *buf;

    buf = allocate_buffer(s0->u.number);
    if (buf) {
	C_REFED_BUFFER(ret, buf);
    } else {
	C_NUMBER(ret, 0);
    }
}
#endif

#ifdef F_ALLOCATE_MAPPING
void c_allocate_mapping P2(svalue *, ret, svalue *, s0)
{
    struct mapping *map;

    map = allocate_mapping(s0->u.number);
    C_REFED_MAPPING(ret, map);
}
#endif

#ifdef F_CACHE_STATS
void c_cache_stats P1(svalue *, ret)
{
    print_cache_stats();
    C_NUMBER(ret, 0);
}
#endif

#ifdef F_CALL_OUT
void c_call_out P4(svalue *, ret, svalue *, s0, svalue *, s1, int, num_arg)
{
    if (!(current_object->flags & O_DESTRUCTED))
	new_call_out(current_object, s0->u.string, s1->u.number,
		     num_arg - 1, num_arg ? sp - num_arg + 1 : 0);
    pop_n_elems(num_arg);
}
#endif

#ifdef F_CALL_OUT_INFO
void c_call_out_info P1(svalue *, ret)
{
    C_REFED_VECTOR(ret, get_all_call_outs());
}
#endif

#ifdef F_CAPITALIZE
void c_capitalize P2(svalue *, ret, svalue *, s0)
{
    if (s0->u.string && islower(s0->u.string[0])) {
	char *str;

	str = string_copy(s0->u.string);
	str[0] += 'A' - 'a';
	C_MALLOCED_STRING(ret, str);
    } else
	assign_svalue_no_free(ret, s0);
}
#endif

#ifdef F_CHILDREN
void c_children P2(svalue *, ret, svalue *, s0)
{
    struct vector *vec;

    vec = children(s0->u.string);
    if (!vec)
	C_NUMBER(ret, 0);
    else {
	C_REFED_VECTOR(ret, vec);
    }
}
#endif

#ifdef F_CLEAR_BIT
void c_clear_bit P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    char *str;
    int len, ind;

    if (s1->u.number > MAX_BITS)
	error("clear_bit: %d > MAX_BITS\n", s1->u.number);
    len = SVALUE_STRLEN(s0);
    ind = s1->u.number / 6;
    if (ind >= len) {		/* return first arg unmodified */
	assign_svalue_no_free(ret, s0);
	return;
    }
    str = DXALLOC(len + 1, 29, "f_clear_bit: str");
    memcpy(str, s0->u.string, len + 1);	/* including null byte */
    if (str[ind] > 0x3f + ' ' || str[ind] < ' ')
	error("Illegal bit pattern in clear_bit character %d\n", ind);
    str[ind] = ((str[ind] - ' ') & ~(1 << (s1->u.number % 6))) + ' ';
    C_MALLOCED_STRING(ret, str);
}
#endif

#ifdef F_CLONEP
void c_clonep P2(svalue *, ret, svalue *, s0)
{
    ret->type = T_NUMBER;
    if ((s0->type == T_OBJECT) && (s0->u.ob->flags & O_CLONE))
	ret->u.number = 1;
    else
	ret->u.number = 0;
}
#endif

#ifdef F_COMMAND
void c_command P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    int i;

#ifdef OLD_COMMAND
    i = command_for_object(s0->u.string, s1 ? s1->u.ob : 0);
#else
    if (s1 && (s1->u.ob != current_object)) {
	error("Use command(cmd) or command(cmd, this_object()).\n");
    }
    i = command_for_object(s0->u.string, 0);
#endif
    C_NUMBER(ret, i);
}
#endif

#ifdef F_COMMANDS
void c_commands P1(svalue *, ret)
{
    struct vector *vec;

    vec = commands(current_object);
    C_REFED_VECTOR(ret, vec);
}
#endif

#ifdef F_GENERATE_SOURCE
void c_generate_source P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    C_NUMBER(ret, generate_source(s0->u.string, (s1 ? s1->u.string : 0)));
}
#endif

#ifdef F_CP
void c_cp P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    C_NUMBER(ret, copy_file(s0->u.string, s1->u.string));
}
#endif

#ifdef F_CRC32
void c_crc32 P2(svalue *, ret, svalue *, s0)
{
    int len;
    char *buf;
    UINT32 crc;

    if (s0->type == T_STRING) {
	len = strlen(s0->u.string);
	buf = s0->u.string;
    } else if (s0->type == T_BUFFER) {
	len = s0->u.buf->size;
	buf = (char *) s0->u.buf->item;
    } else {
	bad_arg(1, F_CRC32);
    }
    crc = compute_crc32((unsigned char *)buf, len);
    C_NUMBER(ret, crc);
}
#endif

#ifdef F_CREATOR
void c_creator P2(svalue *, ret, svalue *, s0)
{
    ob = s0->u.ob;
    if (ob->uid == 0) {
	C_NUMBER(ret, 0);
    } else {
	C_CONSTANT_STRING(ret, s0->uid->name);
    }
}
#endif				/* CREATOR */

#ifdef F_CTIME
void c_ctime P2(svalue *, ret, svalue *, s0)
{
    char *cp;

    cp = string_copy(time_string(s0->u.number));
    C_MALLOCED_STRING(ret, cp);
    /* Now strip the newline. */
    cp = strchr(cp, '\n');
    if (cp)
	*cp = '\0';
}
#endif

#ifdef F_DEBUG_INFO
void c_debug_info P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    switch (s0->u.number) {
	case 0:
	{
	    int i, flags;
	    struct object *obj2;

	    ob = s1->u.ob;
	    flags = ob->flags;
	    add_message("O_HEART_BEAT      : %s\n",
			flags & O_HEART_BEAT ? "TRUE" : "FALSE");
	    add_message("O_IS_WIZARD       : %s\n",
			flags & O_IS_WIZARD ? "TRUE" : "FALSE");
	    add_message("O_ENABLE_COMMANDS : %s\n",
			flags & O_ENABLE_COMMANDS ? "TRUE" : "FALSE");
	    add_message("O_CLONE           : %s\n",
			flags & O_CLONE ? "TRUE" : "FALSE");
	    add_message("O_VIRTUAL         : %s\n",
			flags & O_VIRTUAL ? "TRUE" : "FALSE");
	    add_message("O_DESTRUCTED      : %s\n",
			flags & O_DESTRUCTED ? "TRUE" : "FALSE");
	    add_message("O_SWAPPED         : %s\n",
			flags & O_SWAPPED ? "TRUE" : "FALSE");
	    add_message("O_ONCE_INTERACTIVE: %s\n",
			flags & O_ONCE_INTERACTIVE ? "TRUE" : "FALSE");
	    add_message("O_RESET_STATE     : %s\n",
			flags & O_RESET_STATE ? "TRUE" : "FALSE");
	    add_message("O_WILL_CLEAN_UP   : %s\n",
			flags & O_WILL_CLEAN_UP ? "TRUE" : "FALSE");
	    add_message("O_WILL_RESET: %s\n",
			flags & O_WILL_RESET ? "TRUE" : "FALSE");
#ifndef NO_LIGHT
	    add_message("total light : %d\n", ob->total_light);
#endif
	    add_message("next_reset  : %d\n", ob->next_reset);
	    add_message("time_of_ref : %d\n", ob->time_of_ref);
	    add_message("ref         : %d\n", ob->ref);
#ifdef DEBUG
	    add_message("extra_ref   : %d\n", ob->extra_ref);
#endif
	    add_message("swap_num    : %d\n", ob->swap_num);
	    add_message("name        : '%s'\n", ob->name);
	    add_message("next_all    : OBJ(%s)\n",
			ob->next_all ? ob->next_all->name : "NULL");
	    if (obj_list == ob)
		add_message("This object is the head of the object list.\n");
	    for (obj2 = obj_list, i = 1; obj2; obj2 = obj2->next_all, i++)
		if (obj2->next_all == ob) {
		    add_message("Previous object in object list: OBJ(%s)\n",
				obj2->name);
		    add_message("position in object list:%d\n", i);
		}
	    C_NUMBER(ret, 0);
	    break;
	}
    case 1:
	ob = s1->u.ob;
	if (ob->flags & O_SWAPPED) {
	    add_message("Swapped\n");
	    break;
	}
	add_message("program ref's %d\n", ob->prog->p.i.ref);
	add_message("Name %s\n", ob->prog->name);
	add_message("program size %d\n",
		    ob->prog->p.i.program_size);
	add_message("num func's %d (%d) \n", ob->prog->p.i.num_functions,
		    ob->prog->p.i.num_functions * sizeof(struct function));
	add_message("num strings %d\n", ob->prog->p.i.num_strings);
	add_message("num vars %d (%d)\n", ob->prog->p.i.num_variables,
		    ob->prog->p.i.num_variables * sizeof(struct variable));
	add_message("num inherits %d (%d)\n", ob->prog->p.i.num_inherited,
		    ob->prog->p.i.num_inherited * sizeof(struct inherit));
	add_message("total size %d\n", ob->prog->p.i.total_size);
	C_NUMBER(ret, 0);
	break;
    case 2:
        {
            int i;
            ob = s1->u.ob;
            for (i=0; i<ob->prog->p.i.num_variables; i++) {
                add_message("%s: ", ob->prog->p.i.variable_names[i].name);
                print_svalue(&ob->variables[i]);
                add_message("\n");
            }
            break;
        }
    default:
	bad_arg(1, F_DEBUG_INFO);
    }
}
#endif

#if (defined(DEBUGMALLOC) && defined(DEBUGMALLOC_EXTENSIONS))
#ifdef F_DEBUGMALLOC
void c_debugmalloc P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    dump_debugmalloc(s0->u.string, s1->u.number);
    C_NUMBER(ret, 0);
}
#endif

#ifdef F_SET_MALLOC_MASK
void c_set_malloc_mask P2(svalue *, ret, svalue *, s0)
{
    set_malloc_mask(s0->u.number);
    C_NUMBER(ret, 0);
}
#endif
#endif				/* (defined(DEBUGMALLOC) &&
				 * defined(DEBUGMALLOC_EXTENSIONS)) */

#ifdef F_DEEP_INHERIT_LIST
void c_deep_inherit_list P2(svalue *, ret, svalue *, s0)
{
    struct vector *vec;
    extern struct vector *deep_inherit_list PROT((struct object *));

    if (!(s0->u.ob->flags & O_SWAPPED)) {
	vec = deep_inherit_list(s0->u.ob);
    } else {
	vec = null_array();
    }
    C_REFED_VECTOR(ret, vec);
}
#endif

#ifdef DEBUG_MACRO

#ifdef F_SET_DEBUG_LEVEL
void c_set_debug_level P2(svalue *, ret, svalue *, s0)
{
    debug_level = s0->u.number;
    C_NUMBER(ret, 0);
}
#endif

#endif

#ifdef F_DEEP_INVENTORY
void c_deep_inventory P2(svalue *, ret, svalue *, s0)
{
    struct vector *vec;

    vec = deep_inventory(s0->u.ob, 0);
    C_REFED_VECTOR(ret, vec);
}
#endif

#ifdef F_DESTRUCT
void c_destruct P2(svalue *, ret, svalue *, s0)
{
    destruct_object(s0);
    C_NUMBER(ret, 1);
}
#endif

#ifdef F_DISABLE_COMMANDS
void c_disable_commands P1(svalue *, ret)
{
    enable_commands(0);
    C_NUMBER(ret, 0);
}
#endif

#ifdef F_DUMPALLOBJ
void c_dumpallobj P2(svalue *, ret, svalue *, s0)
{
    char *fn = "/OBJ_DUMP";

    if (s0) {
	fn = s0->u.string;
    }
    dumpstat(fn);
    C_NUMBER(ret, 0);
}
#endif

/* f_each */

#ifdef F_EACH
void c_each P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    struct mapping *m;
    struct vector *v;
    int flag;

    m = s0->u.map;
    flag = s1->u.number;
    if (flag) {
	m->eachObj = current_object;
	m->bucket = 0;
	m->elt = (struct node *) 0;
	C_NUMBER(ret, 0);
	return;
    }
    v = mapping_each(m);
    C_REFED_VECTOR(ret, v);
}
#endif

#ifdef F_ED
void c_ed P5(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2, svalue *, s3)
{
    if (command_giver == 0 || command_giver->interactive == 0) {
        C_NUMBER(ret, 0);
        return;
    }

    if (!s0) {
        /* ed() */
        ed_start(0, 0, 0, 0, 0);
	C_NUMBER(ret, 1);
    } else if (!s1) {
        /* ed(fname) */
	ed_start(s0->u.string, 0, 0, 0, 0);
    } else if (!s2) {
        /* ed(fname,exitfn) */
	ed_start(s0->u.string, 0, s1->u.string, 0, current_object);
    } else if (!s3) {
        /* ed(fname,exitfn,restricted) / ed(fname,writefn,exitfn) */
	if (s2->type == T_NUMBER)
	    ed_start(s0->u.string, 0, s1->u.string, s2->u.number,
	             current_object);
	else if (s2->type == T_STRING)
	    ed_start(s0->u.string, s1->u.string, s2->u.string, 0,
	             current_object);
	else
            bad_arg(3, F_ED);
    } else {                    /* num_arg == 4 */
        /* ed(fname,writefn,exitfn,restricted) */
        if (s2->type != T_STRING)
            bad_arg(3, F_ED);
        if (s3->type != T_NUMBER)
            bad_arg(4, F_ED);
        ed_start(s0->u.string, s1->u.string, s2->u.string,
                 s3->u.number, current_object);
    }
}
#endif

#ifdef F_ENABLE_COMMANDS
void c_enable_commands P1(svalue *, ret)
{
    enable_commands(1);
}
#endif

#ifdef F_ENABLE_WIZARD
void c_enable_wizard P1(svalue *, ret)
{
    if (current_object->interactive) {
	current_object->flags |= O_IS_WIZARD;
	C_NUMBER(ret, 1);
    } else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_ERROR
void c_error P2(svalue *, ret, svalue *, s0)
{
    error(s0->u.string);
}
#endif

#ifdef F_ERRORP
void c_errorp P2(svalue *, ret, svalue *, s0)
{
    int i;

    if ((s0->type == T_NUMBER) && (s0->subtype == T_ERROR)) {
	i = 1;
    } else {
	i = 0;
    }
    C_NUMBER(ret, i);
}
#endif

#ifdef F_DISABLE_WIZARD
void c_disable_wizard P1(svalue *, ret)
{
    if (current_object->interactive) {
	current_object->flags &= ~O_IS_WIZARD;
	C_NUMBER(ret, 1);
    } else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_ENVIRONMENT
void c_environment P2(svalue *, ret, svalue *, s0)
{
    if (s0)
	ob = environment(s0);
    else if (!(current_object->flags & O_DESTRUCTED))
	ob = current_object->super;
    else
	ob = 0;
    if (ob)
	C_OBJECT(ret, ob);
    else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_EXEC
void c_exec P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    C_NUMBER(ret, replace_interactive(s0->u.ob, s1->u.ob));
}
#endif

#ifdef F_EXPLODE
void c_explode P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    struct vector *vec;

    vec = explode_string(s0->u.string, s1->u.string);
    if (vec) {
	C_REFED_VECTOR(ret, vec);	/* This will make ref count == 2 */
    } else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_EXPORT_UID
void c_export_uid P2(svalue *, ret, svalue *, s0)
{
    if (current_object->euid == NULL)
	error("Illegal to export uid 0\n");
    ob = s0->u.ob;
    if (ob->euid)
	C_NUMBER(ret, 0);
    else {
	ob->uid = current_object->euid;
	C_NUMBER(ret, 1);
    }
}
#endif

#ifdef F_FILE_NAME
void c_file_name P2(svalue *, ret, svalue *, s0)
{
    char *name, *res;

    /* This function now returns a leading '/' */
    name = s0->u.ob->name;
    res = (char *) add_slash(name);
    C_MALLOCED_STRING(ret, res);
}
#endif

#ifdef F_FILE_SIZE
void c_file_size P2(svalue *, ret, svalue *, s0)
{
    int i;

    i = file_size(s0->u.string);
    C_NUMBER(ret, i);
}
#endif

#ifdef F_FIND_CALL_OUT
void c_find_call_out P2(svalue *, ret, svalue *, s0)
{
    int i;

    i = find_call_out(current_object, s0->u.string);
    C_NUMBER(ret, i);
}
#endif

#ifdef F_FIND_LIVING
void c_find_living P2(svalue *, ret, svalue *, s0)
{
    ob = find_living_object(s0->u.string, 0);
    if (!ob)
	C_NUMBER(ret, 0);
    else
	C_OBJECT(ret, ob);
}
#endif

#ifdef F_FIND_OBJECT
void c_find_object P2(svalue *, ret, svalue *, s0)
{
    ob = find_object2(s0->u.string);
    if (ob && object_visible(ob))
	C_OBJECT(ret, ob);
    else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_FIND_PLAYER
void c_find_player P2(svalue *, ret, svalue *, s0)
{
    ob = find_living_object(s0->u.string, 1);
    if (!ob)
	C_NUMBER(ret, 0);
    else
	C_OBJECT(ret, ob);
}
#endif

#ifdef F_FUNCTION_PROFILE
/* f_function_profile: John Garnett, 1993/05/31, 0.9.17.3 */
void c_function_profile P2(svalue *, ret, svalue *, s0)
{
    struct vector *vec;
    struct mapping *map;
    struct program *prog;
    int nf, j;

    ob = s0->u.ob;
    if (ob->flags & O_SWAPPED) {
	load_ob_from_swap(ob);
    }
    prog = ob->prog;
    nf = prog->p.i.num_functions;
    vec = allocate_array(nf);
    for (j = 0; j < nf; j++) {
	map = allocate_mapping(3);
	add_mapping_pair(map, "calls", prog->p.i.functions[j].calls);
	add_mapping_pair(map, "self", prog->p.i.functions[j].self
			 - prog->p.i.functions[j].children);
	add_mapping_pair(map, "children", prog->p.i.functions[j].children);
	add_mapping_shared_string(map, "name", prog->p.i.functions[j].name);
	vec->item[j].type = T_MAPPING;
	vec->item[j].u.map = map;
    }
    C_REFED_VECTOR(ret, vec);
}
#endif

#ifdef F_FUNCTION_EXISTS
void c_function_exists P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    char *str, *res;

    str = function_exists(s0->u.string, s1->u.ob);
    if (str) {
	res = (char *) add_slash(str);
	if ((str = strrchr(res, '.')))
	    *str = 0;
	C_MALLOCED_STRING(ret, res);
    } else {
	C_NUMBER(ret, 0);
    }
}
#endif

#ifdef F_GET_CHAR
void c_get_char P3(svalue *, ret, svalue *, s0, int, num_arg)
{
    struct svalue *arg;
    int i, tmp;
    int flag;

    arg = sp - num_arg + 1;	/* Points arg at first argument. */
    if ((num_arg < 1) || (arg[0].type != T_NUMBER)) {
	tmp = 0;
	flag = 0;
    } else {
	tmp = 1;
	num_arg--;		/* Don't count the flag as an arg */
	flag = arg[0].u.number;
    }
    i = get_char(s0->u.string, flag, num_arg, &arg[tmp]);
    sp = arg;
    C_NUMBER(ret, i);
}
#endif

#ifdef F_GET_CONFIG
void c_get_config P2(svalue *, ret, svalue *, s0)
{
    if (!get_config_item(ret, s0))
	error("Bad argument to get_config()\n");
}
#endif

#ifdef F_GET_DIR
void c_get_dir P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    struct vector *vec;

    vec = get_dir(s0->u.string, s1->u.number);
    if (vec) {
	C_REFED_VECTOR(ret, vec);
    } else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_GETEUID
void c_geteuid P2(svalue *, ret, svalue *, s0)
{
    if (s0->type == T_OBJECT) {
	ob = s0->u.ob;
	if (ob->euid) {
	    char *tmp;

	    tmp = ob->euid->name;
	    C_CONSTANT_STRING(ret, tmp);
	    return;
	} else {
	    C_NUMBER(ret, 0);
	    return;
	}
    } else if (s0->type == T_FUNCTION) {
#ifdef NEW_FUNCTIONS
	if (s0->u.fp->owner && s0->u.fp->owner->euid) {
 	    char *tmp;
	    
 	    tmp = s0->u.fp->owner->euid->name;
 	    pop_stack();
 	    push_string(tmp, STRING_CONSTANT);
 	    return;
	} else {
	    assign_svalue(sp, &const0);
 	    return;
	}
#else
	if (s0->u.fp->euid) {
	    C_CONSTANT_STRING(ret, s0->u.fp->euid->name);
	    return;
	}
#endif
    }
    C_NUMBER(ret, 0);
}
#endif

#ifdef F_GETUID
void c_getuid P2(svalue *, ret, svalue *, s0)
{
    char *tmp;

    ob = s0->u.ob;
    DEBUG_CHECK(ob->uid == NULL,
		"UID is a null pointer\n");
    tmp = ob->uid->name;
    C_CONSTANT_STRING(ret, tmp);
}
#endif

#ifdef F_IMPLODE
void c_implode P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    char *str;

    check_for_destr(s0->u.vec);
    str = implode_string(s0->u.vec, s1->u.string);
    if (str)
	C_MALLOCED_STRING(ret, str);
    else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_IN_EDIT
void c_in_edit P2(svalue *, ret, svalue *, s0)
{
    C_NUMBER(ret, s0->u.ob->interactive && s0->u.ob->interactive->ed_buffer);
}
#endif

#ifdef F_IN_INPUT
void c_in_input P2(svalue *, ret, svalue *, s0)
{
    C_NUMBER(ret, s0->u.ob->interactive && s0->u.ob->interactive->input_to);
}
#endif

#ifdef F_INHERITS
void c_inherits P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    struct object *ob, *base;

    ob = find_object2(s0->u.string);
    base = s1->u.ob;
    if (IS_ZERO(s1) || !base || !ob || (ob->flags & O_SWAPPED)) {
	C_NUMBER(ret, 0);
	return;
    }
    if (base->flags & O_SWAPPED)
	load_ob_from_swap(base);
    C_NUMBER(ret, inherits(base->prog, ob->prog));
}
#endif

#ifdef F_INHERIT_LIST
void c_inherit_list P2(svalue *, ret, svalue *, s0)
{
    struct vector *vec;
    extern struct vector *inherit_list PROT((struct object *));


    if (!(s0->u.ob->flags & O_SWAPPED)) {
	vec = inherit_list(s0->u.ob);
    } else {
	vec = null_array();
    }
    C_REFED_VECTOR(ret, vec);
}
#endif

#ifdef F_INPUT_TO
void c_input_to P3(svalue *, ret, svalue *, s0, int, num_arg)
{
    struct svalue *arg;
    int i, tmp;
    int flag;

    arg = sp - num_arg + 1;	/* Points arg at first argument. */
    if ((num_arg < 1) || (arg[0].type != T_NUMBER)) {
	tmp = 0;
	flag = 0;
    } else {
	tmp = 1;
	num_arg--;		/* Don't count the flag as an arg */
	flag = arg[0].u.number;
    }
    i = input_to(s0->u.string, flag, num_arg, &arg[tmp]);
    sp = arg;
    C_NUMBER(ret, i);
}
#endif

#ifdef F_INTERACTIVE
void c_interactive P2(svalue *, ret, svalue *, s0)
{
    C_NUMBER(ret, ((int) s0->u.ob->interactive != 0));
}
#endif

#ifdef F_INTP
void c_intp P2(svalue *, ret, svalue *, s0)
{
    if (s0->type == T_NUMBER)
	C_NUMBER(ret, 1);
    else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_FUNCTIONP
void c_functionp P2(svalue *, ret, svalue *, s0)
{
#ifdef NEW_FUNCTIONS
  int i;
#endif
  
  if (s0->type == T_FUNCTION) {
#ifdef NEW_FUNCTIONS
      i = s0->u.fp->type;
      if (s0->u.fp->args.type == T_POINTER) i |= 1;
      C_NUMBER(ret, i);
      return;
#else
      if (((s0->u.fp->obj.type == T_OBJECT) &&
	   !(s0->u.fp->obj.u.ob->flags & O_DESTRUCTED)) ||
	  (s0->u.fp->obj.type == T_STRING)) {
	  if (s0->u.fp->fun.type == T_STRING) {
	      C_NUMBER(ret, 1);
	      return;
	  } else if (s0->u.fp->fun.type == T_POINTER) {
	      C_NUMBER(ret, 2);
	      return;
	  }
      }
#endif
  }
  C_NUMBER(ret, 0);
}
#endif

#ifdef F_KEYS
void c_keys P2(svalue *, ret, svalue *, s0)
{
    struct vector *vec;

    vec = mapping_indices(s0->u.map);
    C_REFED_VECTOR(ret, vec);
}
#endif

#ifdef F_VALUES
void c_values P2(svalue *, ret, svalue *, s0)
{
    struct vector *vec;

    vec = mapping_values(s0->u.map);
    C_REFED_VECTOR(ret, vec);
}
#endif

#ifdef F_LINK
void c_link P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    struct svalue *mret;
    int i;

    push_string(s0->u.string, STRING_CONSTANT);
    push_string(s1->u.string, STRING_CONSTANT);
    mret = apply_master_ob(APPLY_VALID_LINK, 2);
    if (MASTER_APPROVED(mret))
	i = do_rename(s0->u.string, s1->u.string, F_LINK);
    C_NUMBER(ret, i);
}
#endif				/* F_LINK */

#ifdef F_LIVING
void c_living P2(svalue *, ret, svalue *, s0)
{
    if (s0->u.ob->flags & O_ENABLE_COMMANDS)
	C_NUMBER(ret, 1);
    else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_LIVINGS
void c_livings P1(svalue *, ret)
{
    C_REFED_VECTOR(ret, livings());
}
#endif

#ifdef F_LOWER_CASE
void c_lower_case P2(svalue *, ret, svalue *, s0)
{
    int i;

    char *str = string_copy(s0->u.string);

    for (i = strlen(str) - 1; i >= 0; i--)
	if (isalpha(str[i]))
	    str[i] |= 'a' - 'A';
    C_MALLOCED_STRING(ret, str);
}
#endif

#ifdef F_MALLOC_STATUS
void c_malloc_status P1(svalue *, ret)
{
#ifdef DO_MSTATS
    void show_mstats PROT((char *));

#endif
#if (defined(WRAPPEDMALLOC) || defined(DEBUGMALLOC))
    void dump_malloc_data();

#endif

    if (using_bsd_malloc) {
	add_message("Using BSD malloc.\n");
    }
    if (using_smalloc) {
	add_message("Using Smalloc.\n");
    }
#ifdef DO_MSTATS
    show_mstats("malloc_status()");
#endif
#if (defined(WRAPPEDMALLOC) || defined(DEBUGMALLOC))
    dump_malloc_data();
#endif
#ifdef SYSMALLOC
    if (!using_bsd_malloc && !using_smalloc) {
	add_message("Using system malloc.\n");
    }
#endif
    C_NUMBER(ret, 0);
}
#endif

#ifdef F_MAP_DELETE
void c_map_delete P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    mapping_delete(s0->u.map, s1);
}
#endif

#ifdef F_MAPP
void c_mapp P2(svalue *, ret, svalue *, s0)
{
    C_NUMBER(ret, (s0->type == T_MAPPING) ? 1 : 0);
}
#endif

#ifdef F_MAP_ARRAY
void c_map_array P5(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2, svalue *, s3)
{
    struct vector *res = 0;
    struct mapping *map = (struct mapping *) 0;

    ob = 0;

    if (s2->type == T_OBJECT)
	ob = s2->u.ob;
    else if (s2->type == T_STRING) {
	ob = find_object(s2->u.string);
	if (ob && !object_visible(ob))
	    ob = 0;
    }
    if (!ob)
	bad_arg(3, F_MAP_ARRAY);

    if (s0->type == T_POINTER) {
	check_for_destr(s0->u.vec);
	res = map_array(s0->u.vec, s1->u.string, ob, s3);
    } else if (s0->type == T_MAPPING) {
	map = map_mapping(s0->u.map, s1->u.string, ob, s3);
    } else {
	res = 0;
    }
    if (map) {
	C_REFED_MAPPING(ret, map);
    } else if (res) {
	C_REFED_VECTOR(ret, res);
    } else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_MASTER
void c_master P1(svalue *, ret)
{
    int err;

    if (err=assert_master_ob_loaded("master", "") != 1)
	ret = &const0n;
    else 
	C_OBJECT(ret, master_ob);
}
#endif

/*
This efun searches a mapping for a path.  Each key is assumed to be a
string.  The value is completely arbitrary.  The efun finds the largest
matching path in the mapping.  Keys ended in '/' are assumed to match
paths with character that follow the '/', i.e. / is a wildcard for anything
below this directory.  DO NOT CHANGE THIS EFUN TIL YOU UNDERSTAND IT.  It
catches folks by suprise at first, but it is coded the way it is for a reason.
It effectively implements the search loop in TMI's access object as a single
efun.

        Cygnus
*/
#ifdef F_MATCH_PATH
void c_match_path P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    struct svalue *value;
    struct svalue string;
    char *src;
    char *dst;
    struct svalue *nvalue;

    value = &const0u;

    string.type = T_STRING;
    string.subtype = STRING_MALLOC;
    string.u.string = (char *) MALLOC(strlen(s1->u.string) + 1);

    src = s1->u.string;
    dst = string.u.string;

    while (*src != '\0') {
	while (*src != '/' && *src != '\0')
	    *dst++ = *src++;
	if (*src == '/') {
	    while (*++src == '/');
	    if (*src != '\0' || dst == string.u.string)
		*dst++ = '/';
	}
	*dst = '\0';
	nvalue = find_in_mapping(s0->u.map, &string);
	if (nvalue != &const0u)
	    value = nvalue;
    }

    FREE(string.u.string);

    assign_svalue_no_free(ret, value);
}
#endif				/* F_MATCH_PATH */

#ifdef F_MEMBER_ARRAY
void c_member_array P4(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2)
{
    struct vector *v;
    struct svalue *find;
    int i, ncmp = 0;

    v = s1->u.vec;
    find = s0;
    if (s2) {
	if ((s2->type == T_NUMBER) &&
	    s2->u.number && (s0->type == T_STRING)) {
	    ncmp = strlen(s0->u.string);
	}
    }
    for (i = 0; i < v->size; i++) {
	if (v->item[i].type == T_OBJECT &&
	    v->item[i].u.ob->flags & O_DESTRUCTED)
	    assign_svalue(&v->item[i], &const0);
	if (v->item[i].type != find->type)
	    continue;
	switch (find->type) {
	case T_STRING:
	    if (ncmp) {
		if (strncmp(find->u.string, v->item[i].u.string, ncmp) == 0)
		    break;
	    } else if (strcmp(find->u.string, v->item[i].u.string) == 0)
		break;
	    continue;
	case T_NUMBER:
	    if (find->u.number == v->item[i].u.number)
		break;
	    continue;
	case T_REAL:
	    if (find->u.real == v->item[i].u.real)
		break;
	    continue;
	case T_POINTER:
	    if (find->u.vec == v->item[i].u.vec)
		break;
	    continue;
	case T_OBJECT:
	    if (find->u.ob == v->item[i].u.ob)
		break;
	    continue;
	case T_MAPPING:
	    if (find->u.map == v->item[i].u.map)
		break;
	    continue;
	case T_FUNCTION:
	    if (find->u.fp == v->item[i].u.fp)
		break;
	    continue;
	case T_BUFFER:
	    if (find->u.buf == v->item[i].u.buf)
		break;
	    continue;
	default:
	    fatal("Bad type to member_array(): %d\n", find->type);
	}
	break;
    }
    if (i == v->size)
	i = -1;			/* Return -1 for failure */
    C_NUMBER(ret, i);
}
#endif

#ifdef F_MESSAGE
void c_message P5(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2, svalue *, s3)
{
    struct vector *use, *avoid;

    static struct vector vtmp1 =
	{1,
#ifdef DEBUG
	     1,
#endif
	     1,
#ifndef NO_MUDLIB_STATS
	     {(mudlib_stats_t *) NULL, (mudlib_stats_t *) NULL}
#endif
     };
    static struct vector vtmp2 =
	{1,
#ifdef DEBUG
	     1,
#endif
	     1,
#ifndef NO_MUDLIB_STATS
	     {(mudlib_stats_t *) NULL, (mudlib_stats_t *) NULL}
#endif
     };

    switch (s2->type) {
    case T_OBJECT:
	vtmp1.item[0].type = T_OBJECT;
	vtmp1.item[0].u.ob = s2->u.ob;
	use = &vtmp1;
	break;
    case T_POINTER:
	use = s2->u.vec;
	break;
    case T_STRING:
	vtmp1.item[0].type = T_STRING;
	vtmp1.item[0].u.string = s2->u.string;
	use = &vtmp1;
	break;
    case T_NUMBER:
	if (s2->u.number == 0) {
	    /* for compatibility (write() simul_efuns, etc)  -bobf */
	    check_legal_string(s1->u.string);
	    add_message(s1->u.string);
	    return;
	}
    default:
        bad_argument(s2, T_OBJECT | T_STRING | T_POINTER | T_NUMBER,
                     3, F_MESSAGE);
    }
    if (s3) {
	switch (s3->type) {
	case T_OBJECT:
	    vtmp2.item[0].type = T_OBJECT;
	    vtmp2.item[0].u.ob = s3->u.ob;
	    avoid = &vtmp2;
	    break;
	case T_POINTER:
	    avoid = s3->u.vec;
	    break;
	default:
	    avoid = null_array();
	}
    } else
	avoid = null_array();
    do_message(s0, s1->u.string, use, avoid, 1);
    return;
}
#endif

#ifdef F_MKDIR
void c_mkdir P2(svalue *, ret, svalue *, s0)
{
    char *path;

    path = check_valid_path(s0->u.string, current_object, "mkdir", 1);
    if (path == 0 || mkdir(path, 0770) == -1)
	C_NUMBER(ret, 0);
    else
	C_NUMBER(ret, 1);
}
#endif

#ifdef F_MOVE_OBJECT
void c_move_object P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    svalue *sp = s1;
    struct object *o1, *o2;

    /* get destination */
    if (!sp)
	sp = s0;
    if (sp->type == T_OBJECT)
	o2 = sp->u.ob;
    else {
	o2 = find_object(sp->u.string);
	if (!o2 || !object_visible(o2))
	    error("move_object failed: could not find destination\n");
    }

    /* get source */
    if (!s1) {
	o1 = current_object;
	if (o1->flags & O_DESTRUCTED)
	    error("move_object(): can't move a destructed object\n");
    } else {
	if (s0->type == T_OBJECT)
	    o1 = s0->u.ob;
	else {
	    o1 = find_object(s0->u.string);
	    if (!o1)
		error("move_object(): can't move non-existent object\n");
	}
    }

    move_object(o1, o2);
}
#endif

#ifdef F_MUD_STATUS
void c_mud_status P2(svalue *, ret, svalue *, s0)
{
    int tot, res, verbose = 0;
    extern char *reserved_area;
    extern int tot_alloc_object, tot_alloc_sentence, tot_alloc_object_size,
        num_mappings, num_arrays, total_array_size, total_mapping_size,
        total_users, total_mapping_nodes;
    extern int total_num_prog_blocks;
    extern int total_prog_block_size;
    extern int add_message_calls, inet_packets, inet_volume;

    verbose = s0->u.number;

    if (reserved_area)
	res = RESERVED_SIZE;
    else
	res = 0;

    if (verbose) {
	char dir_buf[1024];
	FILE *testfp;

	if (testfp = fopen(".mudos_test_file", "w")) {
	    fclose(testfp);
	    add_message("Open-file-test succeeded.\n");
	    unlink(".mudos_test_file");
	} else {
	    /* if strerror() is missing, edit the #ifdef for it in port.c */
	    add_message("Open file test failed: %s\n", strerror(errno));
	}

	add_message("current working directory: %s\n\n",
		    get_current_dir(dir_buf, 1024));
	add_message("add_message statistics\n");
	add_message("------------------------------\n");
	add_message("Calls to add_message: %d   Packets: %d   Average packet size: %f\n\n",
	add_message_calls, inet_packets, (float) inet_volume / inet_packets);

	stat_living_objects();
	add_message("\n");
#ifdef F_CACHE_STATS
	print_cache_stats();
	add_message("\n");
#endif
	print_swap_stats();
	add_message("\n");

        tot = show_otable_status(verbose);
        add_message("\n");
        tot += heart_beat_status(verbose);
        add_message("\n");
        tot += add_string_status(verbose);
        add_message("\n");
        tot += print_call_out_usage(verbose);
    } else {
	/* !verbose */
	add_message("Sentences:\t\t\t%8d %8d\n", tot_alloc_sentence,
		    tot_alloc_sentence * sizeof(struct sentence));
	add_message("Objects:\t\t\t%8d %8d\n",
		    tot_alloc_object, tot_alloc_object_size);
	add_message("Prog blocks:\t\t\t%8d %8d\n",
		    total_num_prog_blocks, total_prog_block_size);
	add_message("Arrays:\t\t\t\t%8d %8d\n", num_arrays,
		    total_array_size);
	add_message("Mappings:\t\t\t%8d %8d\n", num_mappings,
		    total_mapping_size);
	add_message("Mappings(nodes):\t\t%8d\n", total_mapping_nodes);
	add_message("Interactives:\t\t\t%8d %8d\n", total_users,
		    total_users * sizeof(struct interactive));

	tot = show_otable_status(verbose) +
	    heart_beat_status(verbose) +
	    add_string_status(verbose) +
	    print_call_out_usage(verbose);
    }

    tot += total_prog_block_size +
	total_array_size +
	total_mapping_size +
	tot_alloc_sentence * sizeof(struct sentence) +
	tot_alloc_object_size +
	total_users * sizeof(struct interactive) +
	res;

    if (!verbose) {
	add_message("\t\t\t\t\t --------\n");
	add_message("Total:\t\t\t\t\t %8d\n", tot);
    }

    C_NUMBER(ret, 0);
}
#endif

#if defined(F_NEW)
void c_new P2(svalue *, ret, svalue *, s0)
{
    struct object *ob;

    ob = clone_object(s0->u.string);
    if (ob) {
	ret->type = T_OBJECT;
	ret->u.ob = ob;
	add_ref(ob, "C_NEW");
    } else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_NOTIFY_FAIL
void c_notify_fail P2(svalue *, ret, svalue *, s0)
{
    set_notify_fail_message(s0->u.string);
    C_NUMBER(ret, 0);
}
#endif

#ifdef F_NULLP
void c_nullp P2(svalue *, ret, svalue *, s0)
{
    if (IS_NULL(s0))
	C_NUMBER(ret, 1);
    else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_OBJECTP
void c_objectp P2(svalue *, ret, svalue *, s0)
{
    if (s0->type == T_OBJECT)
	C_NUMBER(ret, 1);
    else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_OPCPROF
void c_opcprof P2(svalue *, ret, svalue *, s0)
{
    char *fn = "/OPCPROF";

    if (s0) {
	fn = s0->u.string;
    }
    opcdump(fn);
    C_NUMBER(ret, 0);
}
#endif

#ifdef F_ORIGIN
void c_origin P1(svalue *, ret)
{
    C_NUMBER(ret, (int) caller_type);
}
#endif

#ifdef F_POINTERP
void c_pointerp P2(svalue *, ret, svalue *, s0)
{
    if (s0->type == T_POINTER)
	C_NUMBER(ret, 1);
    else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_PRESENT
void c_present P3(svalue *, ret, svalue *, s0, svalue *, s1)
{

#ifdef LAZY_RESETS
    if (s1) {
	try_reset(s1->u.ob);
    }
#endif
    ob = object_present(s0, s1 ? s1->u.ob : 0);
    if (ob)
	C_OBJECT(ret, ob);
    else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_PREVIOUS_OBJECT
void c_previous_object P2(svalue *, ret, svalue *, s0)
{
    int i;
    struct control_stack *p;
    extern struct control_stack control_stack[MAX_TRACE];

    if (s0) {
	if ((i = sp->u.number) > 0) {
	    ob = 0;
	    p = csp + 1;
	    while ((p--) >= control_stack) {
		if (p->extern_call) {
		    i--;
		    if (!i) {
			ob = p->prev_ob;
			break;
		    }
		}
	    }
	} else if (i == -1) {
	    struct vector *v;

	    i = previous_ob ? 1 : 0;
	    p = csp + 1;
	    while ((p--) >= control_stack) {
		if (p->extern_call)
		    i++;
	    }
	    v = allocate_array(i);
	    p = csp + 1;
	    if (previous_ob) {
		if (!(previous_ob->flags & O_DESTRUCTED)) {
		    v->item[0].type = T_OBJECT;
		    v->item[0].u.ob = previous_ob;
		    add_ref(previous_ob, "previous_object(-1)");
		}
		i = 1;
	    } else i = 0;
	    while ((p--) >= control_stack) {
		if (p->extern_call && p->prev_ob) {
		    if (!p->prev_ob->flags & O_DESTRUCTED) {
			v->item[i].type = T_OBJECT;
			v->item[i].u.ob = p->prev_ob;
			add_ref(p->prev_ob, "previous_object(-1)");
		    }
		    i++;
		}
	    }
	    C_REFED_VECTOR(ret, v);
	    return;
	} else if (i < -1) {
	    error("Illegal negative argument to previous_object()\n");
	} else
	    ob = previous_ob;
    } else
	ob = previous_ob;
    if (ob == 0 || (ob->flags & O_DESTRUCTED))
	C_NUMBER(ret, 0);
    else
	C_OBJECT(ret, previous_ob);
}
#endif

#ifdef F_PRINTF
void c_printf P3(svalue *, ret, svalue *, s0, int, num_arg)
{
    if (command_giver)
	tell_object(command_giver, string_print_formatted(s0->u.string,
						num_arg, sp - num_arg + 1));
    pop_n_elems(num_arg);
}
#endif

#ifdef F_PROCESS_STRING
void c_process_string P2(svalue *, ret, svalue *, s0)
{
    extern char
        *process_string PROT((char *));

    char *str;

    str = process_string(s0->u.string);
    C_MALLOCED_STRING(ret, str);
}
#endif

#ifdef F_QUERY_HOST_NAME
void c_query_host_name P1(svalue *, ret)
{
    extern char *query_host_name PROT((void));
    char *tmp;

    tmp = query_host_name();
    if (tmp)
	C_CONSTANT_STRING(ret, tmp);
    else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_QUERY_IDLE
void c_query_idle P2(svalue *, ret, svalue *, s0)
{
    C_NUMBER(ret, query_idle(s0->u.ob));
}
#endif

#ifdef F_QUERY_IP_NAME
void c_query_ip_name P2(svalue *, ret, svalue *, s0)
{
    extern char *query_ip_name PROT((struct object *));
    char *tmp;

    tmp = query_ip_name(s0 ? s0->u.ob : 0);
    if (tmp == 0)
	C_NUMBER(ret, 0);
    else
	C_STRING(ret, tmp, STRING_MALLOC);
}
#endif

#ifdef F_QUERY_IP_NUMBER
void c_query_ip_number P2(svalue *, ret, svalue *, s0)
{
    extern char *query_ip_number PROT((struct object *));
    char *tmp;

    tmp = query_ip_number(s0 ? s0->u.ob : 0);
    if (tmp == 0)
	C_NUMBER(ret, 0);
    else
	C_STRING(ret, tmp, STRING_MALLOC);
}
#endif

#ifdef F_QUERY_LOAD_AVERAGE
void c_query_load_average P1(svalue *, ret)
{
    C_STRING(ret, query_load_av(), STRING_MALLOC);
}
#endif

#ifdef F_QUERY_PRIVS
void c_query_privs P2(svalue *, ret, svalue *, s0)
{
    ob = s0->u.ob;
    if (ob->privs != NULL) {
	C_STRING(ret, ob->privs, STRING_SHARED);
    } else {
	C_NUMBER(ret, 0);
    }
}
#endif

#ifdef F_QUERY_SNOOPING
void c_query_snooping P2(svalue *, ret, svalue *, s0)
{
    struct object *ob;

    ob = query_snooping(s0->u.ob);
    if (ob)
	C_OBJECT(ret, ob);
    else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_QUERY_SNOOP
void c_query_snoop P2(svalue *, ret, svalue *, s0)
{
    struct object *ob;

    ob = query_snoop(s0->u.ob);
    if (ob)
	C_OBJECT(ret, ob);
    else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_QUERY_VERB
void c_query_verb P1(svalue *, ret)
{
    if (last_verb == 0) {
	C_NUMBER(ret, 0);
	return;
    }
    C_STRING(ret, last_verb, STRING_SHARED);
}
#endif

#ifdef F_RANDOM
void c_random P2(svalue *, ret, svalue *, s0)
{
    if (s0->u.number <= 0) {
	C_NUMBER(ret, 0);
	return;
    }
    C_NUMBER(ret, random_number(s0->u.number));
}
#endif

#ifdef F_READ_BYTES
void c_read_bytes P4(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2)
{
    char *str;
    int start = 0, len = 0, rlen = 0;

    if (s1)
	start = s1->u.number;
    if (s2) {
	if (s2->type != T_NUMBER)
	    bad_arg(2, F_READ_BYTES);
	len = s2->u.number;
    }
    str = read_bytes(s0->u.string, start, len, &rlen);
    if (str == 0)
	C_NUMBER(ret, 0);
    else
	C_MALLOCED_STRING(ret, str);
}
#endif

#ifdef F_READ_BUFFER
void c_read_buffer P4(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2)
{
    char *str;
    int start = 0, len = 0, rlen = 0;
    int from_file = 0;		/* new line */

    if (s1) {
	start = s1->u.number;
    }
    if (s2) {
	if (s2->type != T_NUMBER)
	    bad_arg(2, F_READ_BUFFER);
	len = s2->u.number;
    }
    if (s0->type == T_STRING) {
	from_file = 1;		/* new line */
	str = read_bytes(s0->u.string, start, len, &rlen);
    } else {			/* T_BUFFER */
	str = read_buffer(s0->u.buf, start, len, &rlen);
    }
    if (str == 0) {
	C_NUMBER(ret, 0);
    } else if (from_file) {	/* changed */
	struct buffer *buf;

	buf = allocate_buffer(rlen);
	memcpy(buf->item, str, rlen);
	C_REFED_BUFFER(ret, buf);
	FREE(str);
    } else {			/* T_BUFFER */
	C_MALLOCED_STRING(ret, str);
    }
}
#endif

#ifdef F_READ_FILE
void c_read_file P4(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2)
{
    char *str;
    int start = 0, len = 0;

    if (s1)
	start = s1->u.number;
    if (s2) {
	if (s2->type != T_NUMBER)
	    bad_arg(2, F_READ_FILE);
	len = s2->u.number;
    }
    str = read_file(s0->u.string, start, len);
    if (str == 0)
	C_NUMBER(ret, 0);
    else {
	C_STRING(ret, str, STRING_MALLOC);
	FREE(str);
    }
}
#endif

#ifdef F_RECEIVE
void c_receive P2(svalue *, ret, svalue *, s0)
{
    if (current_object->interactive) {
	struct object *save_command_giver = command_giver;

	check_legal_string(s0->u.string);
	command_giver = current_object;
	add_message("%s", s0->u.string);
	command_giver = save_command_giver;
	C_NUMBER(ret, 1);
    } else {
	C_NUMBER(ret, 0);
    }
}
#endif

#ifdef F_REFS
void c_refs P2(svalue *, ret, svalue *, s0)
{
    int r;

    switch (s0->type) {
    case T_MAPPING:
	r = s0->u.map->ref;
	break;
    case T_POINTER:
	r = s0->u.vec->ref;
	break;
    case T_OBJECT:
	r = s0->u.ob->ref;
	break;
    case T_FUNCTION:
	r = s0->u.fp->ref;
	break;
    case T_BUFFER:
	r = s0->u.buf->ref;
	break;
    default:
	r = 0;
	break;
    }
    C_NUMBER(ret, r - 1);	/* minus 1 to compensate for being arg of
				 * refs() */
}
#endif

#ifdef F_REGEXP
void c_regexp P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    struct vector *v;

    v = match_regexp(s0->u.vec, s1->u.string);
    if (v == 0)
	C_NUMBER(ret, 0);
    else {
	C_REFED_VECTOR(ret, v);
    }
}
#endif

#ifdef F_REMOVE_ACTION
void c_remove_action P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    int success;

    success = remove_action(s0->u.string, s1->u.string);
    C_NUMBER(ret, success);
}
#endif

#ifdef F_REMOVE_CALL_OUT
void c_remove_call_out P2(svalue *, ret, svalue *, s0)
{
    C_NUMBER(ret, remove_call_out(current_object, s0->u.string));
}
#endif

#ifdef F_RENAME
void c_rename P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    C_NUMBER(ret, do_rename(s0->u.string, s1->u.string, F_RENAME));
}
#endif				/* F_RENAME */

#ifdef F_REPLACE_STRING

/*
syntax for replace_string is now:
    string replace_string(src, pat, rep);   // or
    string replace_string(src, pat, rep, max);  // or
    string replace_string(src, pat, rep, first, last);

The 4th/5th args are optional (to retain backward compatibility).
- src, pat, and rep are all strings.
- max is an integer. It will replace all occurances up to max
  matches (starting as 1 as the first), with a value of 0 meaning
  'replace all')
- first and last are just a range to replace between, with
  the following constraints
    first < 1: change all from start
    last == 0 || last > max matches:    change all to end
    first > last: return unmodified array.
(i.e, with 4 args, it's like calling it with:
    replace_string(src, pat, rep, 0, max);
)
*/

void c_replace_string P5(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2, int, num_arg)
{
    int plen, rlen, dlen, first, last, cur;
    char *src, *pattern, *replace, *dst1, *dst2;

    if (num_arg > 2) {
	error("Too many args to replace_string.\n");
	pop_n_elems(num_arg);
	return;
    }
    if (s2->type != T_STRING) {
	bad_arg(3, F_REPLACE_STRING);	/* first and second args checked
					 * elsewhere */
	pop_n_elems(num_arg);
	return;
    }
    src = s0->u.string;
    pattern = s1->u.string;
    replace = s2->u.string;
    first = 0;
    last = 0;

    if (num_arg) {
	if ((sp - num_arg + 1)->type != T_NUMBER) {
	    bad_arg(4, F_REPLACE_STRING);
	    pop_n_elems(num_arg);
	    return;
	}
	first = (sp - num_arg + 1)->u.number;
	if (num_arg == 1) {
	    last = first;
	    first = 0;
	}
    }
    if (num_arg == 2) {
	if (sp->type != T_NUMBER) {
	    bad_arg(5, F_REPLACE_STRING);
	    pop_n_elems(num_arg);
	    return;
	}
	/* first set above. */
	last = sp->u.number;
    }
    if (!last)
	last = max_string_length;
    if (first > last) {		/* just return it */
	ret->type = T_STRING;
	ret->subtype = STRING_CONSTANT;
	ret->u.string = src;

	pop_n_elems(num_arg);
	return;
    }
    dst2 = dst1 = (char *) DMALLOC(max_string_length, 31, "f_replace_string");

    plen = strlen(pattern);
    rlen = strlen(replace);
    dlen = 0;
    cur = 0;

    if (plen == 0) {
	strcpy(dst2, src);
    } else {
	while (*src != '\0') {
	    if (strncmp(src, pattern, plen) == 0) {
		cur++;
		if (cur >= first && cur <= last) {
		    if (rlen != 0) {
			if (max_string_length - dlen <= rlen) {
			    pop_n_elems(num_arg);
			    *ret = const0u;
			    FREE(dst1);
			    return;
			}
			strncpy(dst2, replace, rlen);
			dst2 += rlen;
			dlen += rlen;
		    }
		    src += plen;
		    continue;
		}
	    }
	    if (max_string_length - dlen <= 1) {
		pop_n_elems(num_arg);
		*ret = const0u;
		FREE(dst1);
		return;
	    }
	    *dst2++ = *src++;
	    dlen++;
	}
	*dst2 = '\0';
    }
    pop_n_elems(num_arg);
    C_STRING(ret, dst1, STRING_MALLOC);
    FREE(dst1);
}
#endif

#ifdef F_RESOLVE
void c_resolve P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    int query_addr_number PROT((char *, char *));

    C_NUMBER(ret, query_addr_number(s0->u.string, s1->u.string));
}				/* f_resolve() */
#endif

#ifdef F_RESTORE_OBJECT
void c_restore_object P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    int flag;

    flag = s1 ? s1->u.number : 0;
    C_NUMBER(ret, restore_object(current_object, s0->u.string, flag));
}
#endif

#ifdef F_RM
void c_rm P2(svalue *, ret, svalue *, s0)
{
    C_NUMBER(ret, remove_file(s0->u.string));
}
#endif

#ifdef F_RMDIR
void c_rmdir P2(svalue *, ret, svalue *, s0)
{
    char *path;

    path = check_valid_path(s0->u.string, current_object, "rmdir", 1);
    if (path == 0 || rmdir(path) == -1)
	C_NUMBER(ret, 0);
    else
	C_NUMBER(ret, 1);
}
#endif

#ifdef F_SAVE_OBJECT
void c_save_object P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    int flag;

    flag = s1 ? s1->u.number : 0;
    C_NUMBER(ret, save_object(current_object, s0->u.string, flag));
}
#endif

#ifdef F_SAY
void c_say P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    struct vector *avoid;
    static struct vector vtmp =
	{1,
#ifdef DEBUG
	     1,
#endif
	     1,
#ifndef NO_MUDLIB_STATS
	     {(mudlib_stats_t *) NULL, (mudlib_stats_t *) NULL}
#endif
     };

    if (!s1) {
	avoid = null_array();
	say(s0, avoid);
    } else {
	if (s1->type == T_OBJECT) {
	    vtmp.item[0].type = T_OBJECT;
	    vtmp.item[0].u.ob = s1->u.ob;
	    avoid = &vtmp;
	} else {		/* must be a vector... */
	    avoid = s1->u.vec;
	}
	say(s0, avoid);
    }
}
#endif

#ifdef F_SET_EVAL_LIMIT
/* warning: do not enable this without using valid_override() in the master
   object and a set_eval_limit() simul_efun to restrict access.
*/
void c_set_eval_limit P2(svalue *, ret, svalue *, s0)
{
    extern int max_cost;

    switch (s0->u.number) {
    case 0:
	C_NUMBER(ret, eval_cost = max_cost);
	break;
    case -1:
	C_NUMBER(ret, eval_cost);
	break;
    default:
	max_cost = s0->u.number;
    }
}
#endif

#ifdef F_SET_BIT
void c_set_bit P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    char *str;
    int len, old_len, ind;

    if (s1->u.number > MAX_BITS)
	error("set_bit: too big bit number: %d\n", s1->u.number);
    if (s1->u.number < 0)
	error("set_bit: illegal (negative) arg 2\n");
    len = SVALUE_STRLEN(s0);
    old_len = len;
    ind = s1->u.number / 6;
    if (ind >= len)
	len = ind + 1;
    str = DXALLOC(len + 1, 32, "f_set_bit");
    str[len] = '\0';
    if (old_len)
	memcpy(str, s0->u.string, old_len);
    if (len > old_len)
	memset(str + old_len, ' ', len - old_len);
    if (str[ind] > 0x3f + ' ' || str[ind] < ' ')
	error("Illegal bit pattern in set_bit character %d\n", ind);
    str[ind] = ((str[ind] - ' ') | (1 << (s1->u.number % 6))) + ' ';
    C_MALLOCED_STRING(ret, str);
}
#endif

#ifdef F_SET_HEART_BEAT
void c_set_heart_beat P2(svalue *, ret, svalue *, s0)
{
    int i;

    i = set_heart_beat(current_object, s0->u.number);
    C_NUMBER(ret, i);
}
#endif

#ifdef F_QUERY_HEART_BEAT
void c_query_heart_beat P2(svalue *, ret, svalue *, s0)
{
    struct object *foo;

    foo = s0->u.ob;
    C_NUMBER(ret, query_heart_beat(foo));
}
#endif

#ifdef F_SET_HIDE
void c_set_hide P2(svalue *, ret, svalue *, s0)
{
    if (!valid_hide(current_object))
	return;
    if (s0->u.number) {
	if (!(current_object->flags & O_HIDDEN) && current_object->interactive)
	    num_hidden++;
	current_object->flags |= O_HIDDEN;
    } else {
	if ((current_object->flags & O_HIDDEN) && current_object->interactive)
	    num_hidden--;
	current_object->flags &= ~O_HIDDEN;
    }
}
#endif

#ifdef F_SET_LIGHT
void c_set_light P2(svalue *, ret, svalue *, s0)
{
    struct object *o1;

    add_light(current_object, s0->u.number);
    o1 = current_object;
    while (o1->super)
	o1 = o1->super;
    C_NUMBER(ret, o1->total_light);
}
#endif

#ifdef F_SET_LIVING_NAME
void c_set_living_name P2(svalue *, ret, svalue *, s0)
{
    set_living_name(current_object, s0->u.string);
}
#endif

#ifdef F_SET_PRIVS
void c_set_privs P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    struct object *ob;

    ob = s0->u.ob;
    if (ob->privs != NULL)
	free_string(ob->privs);
    if (sp->type != T_STRING)
	ob->privs = NULL;
    else
	ob->privs = make_shared_string(s1->u.string);
    C_NUMBER(ret, 0);
}
#endif

#ifdef F_SETEUID
void c_seteuid P2(svalue *, ret, svalue *, s0)
{
    struct svalue *mret;

    if (s0->type == T_NUMBER) {
	if (s0->u.number != 0)
	    bad_arg(1, F_SETEUID);
	current_object->euid = NULL;
	C_NUMBER(ret, 1);
	return;
    }
    if (s0->type != T_STRING)
	bad_arg(1, F_SETEUID);
    push_object(current_object);
    push_string(s0->u.string, STRING_CONSTANT);
    mret = apply_master_ob(APPLY_VALID_SETEUID, 2);
    if (!MASTER_APPROVED(mret)) {
	C_NUMBER(ret, 0);
	return;
    }
    current_object->euid = add_uid(s0->u.string);
    C_NUMBER(ret, 1);
}
#endif

#ifdef F_SETUID
void c_setuid P1(svalue *, ret)
{
    setuid();
    C_NUMBER(ret, 0);
}
#endif				/* F_SETUID */

#ifdef F_SHADOW
void c_shadow P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    struct object *ob;

    ob = s0->u.ob;
    if (s1->u.number == 0) {
	ob = ob->shadowed;
	if (ob)
	    C_OBJECT(ret, ob);
	else
	    C_NUMBER(ret, 0);
	return;
    }
    if (ob == current_object) {
	error("shadow: Can't shadow self\n");
    }
    if (validate_shadowing(ob)) {
	if (current_object->flags & O_DESTRUCTED) {
	    C_NUMBER(ret, 0);
	    return;
	}
	/*
	 * The shadow is entered first in the chain.
	 */
	while (ob->shadowed)
	    ob = ob->shadowed;
	current_object->shadowing = ob;
	ob->shadowed = current_object;
	C_OBJECT(ret, ob);
	return;
    }
    C_NUMBER(ret, 0);
}
#endif

#ifdef F_SHOUT
void c_shout P2(svalue *, ret, svalue *, s0)
{
    shout_string(s0->u.string);
}
#endif

#ifdef F_SHUTDOWN
void c_shutdown P2(svalue *, ret, svalue *, s0)
{
    int exit_code = 0;

    if (s0) {
	exit_code = s0->u.number;
    }
    shutdownMudOS(exit_code);
}
#endif

#ifdef F_SIZEOF
void c_sizeof P2(svalue *, ret, svalue *, s0)
{
    int i;

    if (s0->type == T_POINTER)
	i = s0->u.vec->size;
    else if (s0->type == T_MAPPING)
	i = s0->u.map->count;
    else if (s0->type == T_BUFFER)
	i = s0->u.buf->size;
    else
	i = 0;
    C_NUMBER(ret, i);
}
#endif

#ifdef F_SNOOP
void c_snoop P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    /*
     * This one takes a variable number of arguments. It returns 0 or an
     * object.
     */
    if (!command_giver) {
	C_NUMBER(ret, 0);
    } else {
	ob = 0;			/* Do not remove this, it is not 0 by default */
	if (s1) {
	    if (new_set_snoop(s0->u.ob, s1->u.ob))
		ob = s1->u.ob;
	} else {
	    if (new_set_snoop(s0->u.ob, 0))
		ob = s1->u.ob;
	}
	if (ob)
	    C_OBJECT(ret, ob);
	else
	    C_NUMBER(ret, 0);
    }
}
#endif

#ifdef F_SORT_ARRAY
void c_sort_array P4(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2)
{
    extern struct vector *sort_array
           PROT((struct vector *, char *, struct object *));
    extern struct vector *builtin_sort_array
           PROT((struct vector *, int));
    struct vector *res;

    ob = 0;

    if (s1->type == T_STRING) {
	if (s2->type == T_OBJECT)
	    ob = s2->u.ob;
	else if (s2->type == T_STRING) {
	    ob = find_object(s2->u.string);
	    if (ob && !object_visible(ob))
		ob = 0;
	}
	if (!ob)
	    bad_arg(3, F_SORT_ARRAY);
    }
    if (s0->type == T_POINTER) {
	struct vector *tmp;

	check_for_destr(s0->u.vec);
	tmp = slice_array(s0->u.vec, 0, s0->u.vec->size - 1);
	if (s1->type == T_NUMBER)
	    res = builtin_sort_array(tmp, s1->u.number);
	else
	    res = sort_array(tmp, s1->u.string, ob);
    } else
	res = 0;

    if (res) {
	C_REFED_VECTOR(ret, res);
    } else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_SPRINTF
void c_sprintf P3(svalue *, ret, svalue *, s0, int, num_arg)
{
    char *s;

    /*
     * string_print_formatted() returns a pointer to it's internal buffer, or
     * to an internal constant...  Either way, it must be copied before it's
     * returned as a string.
     */

    s = string_print_formatted(s0->u.string,
			       num_arg, sp - num_arg + 1);
    pop_n_elems(num_arg);
    if (!s)
	C_NUMBER(ret, 0);
    else
	C_MALLOCED_STRING(ret, string_copy(s));
}
#endif

#ifdef F_STAT
void c_stat P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    struct stat buf;
    char *path;
    struct vector *v;

    path = s0->u.string;
    path = check_valid_path(path, current_object, "stat", 0);
    if (!path) {
	C_NUMBER(ret, 0);
	return;
    }
    if (stat(path, &buf) != -1) {
	if (buf.st_mode & S_IFREG) {	/* if a regular file */
	    v = allocate_array(3);
	    v->item[0].type = T_NUMBER;
	    v->item[0].u.number = buf.st_size;
	    v->item[1].type = T_NUMBER;
	    v->item[1].u.number = buf.st_mtime;
	    v->item[2].type = T_NUMBER;
	    ob = find_object2(path);
	    if (ob && !object_visible(ob))
		ob = 0;
	    if (ob)
		v->item[2].u.number = ob->load_time;
	    else
		v->item[2].u.number = 0;
	    C_REFED_VECTOR(ret, v);
	    return;
	}
    }
    v = get_dir(s0->u.string, s1->u.number);
    if (v) {
	C_REFED_VECTOR(ret, v);
    } else {
	C_NUMBER(ret, 0);
    }
}
#endif

#ifdef F_STRSRCH
/*
 * int strsrch(string big, string little, [ int flag ])
 * - search for little in big, starting at right if flag is set
 *   return int offset of little, -1 if not found
 *
 * Written 930706 by Luke Mewburn <zak@rmit.edu.au>
 */

void c_strsrch P4(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2)
{
    register char *big, *little, *pos;
    static char buf[2];		/* should be initialized to 0 */
    int i;

    if (s2->type != T_NUMBER)
	bad_arg(3, F_STRSRCH);

    big = s0->u.string;
    if (s1->type == T_NUMBER) {
	buf[0] = (char) s1->u.number;
	little = buf;
    } else
	little = s1->u.string;

    /* little == ""  or  len(big) < len(little) */
    if (!little[0] || strlen(big) < strlen(little)) {
	pos = NULL;

	/* start at left */
    } else if (!s2->u.number) {
	if (!little[1])		/* 1 char srch pattern */
	    pos = strchr(big, (int) little[0]);
	else
	    pos = _strstr(big, little);

	/* start at right */
    } else {			/* XXX: maybe test for -1 */
	if (!little[1])		/* 1 char srch pattern */
	    pos = strrchr(big, (int) little[0]);
	else {
	    pos = strrchr(big, '\0');	/* find end */
	    pos -= strlen(little) - 1;	/* find rightmost pos it _can_ be */
	    while (1) {
		while (pos-- > big)
		    if (pos[0] == little[0])
			break;
		if (pos[0] != little[0]) {
		    pos = NULL;
		    break;
		}
		for (i = 1; little[i] && (pos[i] == little[i]); i++);	/* scan all chars */
		if (!little[i])
		    break;
	    }
	}
    }

    if (!pos)
	i = -1;
    else
	i = (int) (pos - big);
    C_NUMBER(ret, i);
}				/* strsrch */
#endif

#ifdef F_STRCMP
void c_strcmp P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    C_NUMBER(ret, strcmp(s0->u.string, s1->u.string));
}
#endif

#ifdef F_STRINGP
void c_stringp P2(svalue *, ret, svalue *, s0)
{
    C_NUMBER(ret, s0->type == T_STRING);
}
#endif

#ifdef F_BUFFERP
void c_bufferp P2(svalue *, ret, svalue *, s0)
{
    C_NUMBER(ret, s0->type == T_BUFFER);
}
#endif

#ifdef F_STRLEN
void c_strlen P2(svalue *, ret, svalue *, s0)
{
    C_NUMBER(ret, SVALUE_STRLEN(s0));
}
#endif

#ifdef F_SWAP
void c_swap P2(svalue *, ret, svalue *, s0)
{
    (void) swap(s0->u.ob);
}
#endif

#ifdef F_TAIL
void c_tail P2(svalue *, ret, svalue *, s0)
{
    if (tail(s0->u.string))
	C_NUMBER(ret, 1);
    else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_TELL_OBJECT
void c_tell_object P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    tell_object(s0->u.ob, s1->u.string);
}
#endif

#ifdef F_TELL_ROOM
void c_tell_room P4(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2)
{
    struct vector *avoid;

    if (s0->type == T_OBJECT) {
	ob = s0->u.ob;
    } else {			/* must be a string... */
	ob = find_object(s0->u.string);
	if (!ob || !object_visible(ob))
	    error("Bad argument 1 to tell_room()\n");
    }

    if (!s2) {
	avoid = null_array();
    } else {
	if (s2->type != T_POINTER)
	    bad_arg(3, F_TELL_ROOM);
	else
	    avoid = s2->u.vec;
    }

    tell_room(ob, s1, avoid);
}
#endif

#ifdef F_TEST_BIT
void c_test_bit P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    int len;

    len = SVALUE_STRLEN(s0);
    if (s1->u.number / 6 >= len) {
	C_NUMBER(ret, 0);
	return;
    }
    if ((s0->u.string[s1->u.number / 6] - ' ') & (1 << (s1->u.number % 6))) {
	C_NUMBER(ret, 1);
    } else {
	C_NUMBER(ret, 0);
    }
}
#endif

#ifdef F_THIS_OBJECT
void c_this_object P1(svalue *, ret)
{
    if (current_object->flags & O_DESTRUCTED)	/* Fixed from 3.1.1 */
	C_NUMBER(ret, 0);
    else
	C_OBJECT(ret, current_object);
}
#endif

#ifdef F_THIS_PLAYER
void c_this_player P2(svalue *, ret, svalue *, s0)
{
    if (s0 && current_interactive &&
	!(current_interactive->flags & O_DESTRUCTED))
	C_OBJECT(ret, current_interactive);
    else if (command_giver && !(command_giver->flags & O_DESTRUCTED))
	C_OBJECT(ret, command_giver);
    else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_THROW
void c_throw P2(svalue *, ret, svalue *, s0)
{
    assign_svalue(&catch_value, s0);
    throw_error();		/* do the longjump, with extra checks... */
}
#endif

#ifdef F_TIME
void c_time P1(svalue *, ret)
{
    C_NUMBER(ret, current_time);
}
#endif

#ifdef F_TO_FLOAT
void c_to_float P2(svalue *, ret, svalue *, s0)
{
    double temp;

    if (s0->type == T_REAL) {
	free_svalue(ret,"c_to_float");
	*ret = *s0;
	return;
    } else if (s0->type == T_NUMBER) {
	temp = (double) s0->u.number;
	C_REAL(ret, temp);
    } else if (s0->type == T_STRING) {
	temp = 0.0;
	sscanf(s0->u.string, "%lf", &temp);
	C_REAL(ret, temp);
    }
}
#endif

#ifdef F_TO_INT
void c_to_int P2(svalue *, ret, svalue *, s0)
{
    if (s0->type == T_NUMBER) {
	C_NUMBER(ret, s0->u.number);
	return;
    } else if (s0->type == T_REAL) {
	C_NUMBER(ret, (int) sp->u.real);
    } else if (s0->type == T_STRING) {
	C_NUMBER(ret, atoi(s0->u.string));
    } else if (s0->type == T_BUFFER) {
	if (s0->u.buf->size < sizeof(int)) {
	    C_NUMBER(ret, 0);
	} else {
	    int hostint, netint;

	    memcpy((char *) &netint, s0->u.buf->item, sizeof(int));
	    hostint = ntohl(netint);
	    C_NUMBER(ret, hostint);
	}
    }
}
#endif

#ifdef F_TRACE
void c_trace P2(svalue *, ret, svalue *, s0)
{
    int ot = -1;

    if (command_giver && command_giver->interactive &&
	command_giver->flags & O_IS_WIZARD) {
	ot = command_giver->interactive->trace_level;
	command_giver->interactive->trace_level = s0->u.number;
    }
    C_NUMBER(ret, ot);
}
#endif

#ifdef F_TRACEPREFIX
void c_traceprefix P2(svalue *, ret, svalue *, s0)
{
    char *old = 0;

    if (command_giver && command_giver->interactive &&
	command_giver->flags & O_IS_WIZARD) {
	old = command_giver->interactive->trace_prefix;
	if (s0->type == T_STRING) {
	    command_giver->interactive->trace_prefix =
		make_shared_string(s0->u.string);
	} else
	    command_giver->interactive->trace_prefix = 0;
    }
    if (old) {
	C_STRING(ret, old, STRING_SHARED);	/* Will incr ref count */
	free_string(old);
    } else {
	C_NUMBER(ret, 0);
    }
}
#endif

#ifdef F_TYPE
void c_type P2(svalue *, ret, svalue *, s0)
{
    int k;

    C_NUMBER(ret, s0->type);
}
#endif

#ifdef F_UNDEFINEDP
void c_undefinedp P2(svalue *, ret, svalue *, s0)
{
    if (IS_UNDEFINED(s0))
	C_NUMBER(ret, 1);
    else
	C_NUMBER(ret, 0);
}
#endif

#if 0 /* out of date */
void c_unique_array P4(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2)
{
    struct vector *res;

    check_for_destr(s0->u.vec);
    res = make_unique(s0->u.vec, s1->u.string, (s2 ? s2 : &const0));
    if (res) {
	C_REFED_VECTOR(ret, res);
    } else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_UPTIME
void c_uptime P1(svalue *, ret)
{
    C_NUMBER(ret, current_time - boot_time);
}
#endif

#ifdef F_USERP
void c_userp P2(svalue *, ret, svalue *, s0)
{
    int i;

    i = (int) s0->u.ob->flags & O_ONCE_INTERACTIVE;
    C_NUMBER(ret, i != 0);
}
#endif

#ifdef F_USERS
void c_users P1(svalue *, ret)
{
    C_REFED_VECTOR(ret, users());
}
#endif

#ifdef F_WIZARDP
void c_wizardp P2(svalue *, ret, svalue *, s0)
{
    int i;

    i = (int) s0->u.ob->flags & O_IS_WIZARD;
    C_NUMBER(ret, i != 0);
}
#endif

#ifdef F_VIRTUALP
void c_virtualp P2(svalue *, ret, svalue *, s0)
{
    int i;

    i = (int) s0->u.ob->flags & O_VIRTUAL;
    C_NUMBER(ret, i != 0);
}
#endif

#ifdef F_DOMAIN_STATS
void c_domain_stats P2(svalue *, ret, svalue *, s0)
{
    struct mapping *m;

    if (s0) {
	m = get_domain_stats(s0->u.string);
    } else {
	m = get_domain_stats(0);
    }
    if (!m) {
	C_NUMBER(ret, 0);
    } else {
	/* ref count is properly decremented by get_domain_stats */
	C_MAPPING(ret, m);
    }
}
#endif

#ifdef F_AUTHOR_STATS
void c_author_stats P2(svalue *, ret, svalue *, s0)
{
    struct mapping *m;

    if (s0) {
	m = get_author_stats(s0->u.string);
    } else {
	m = get_author_stats(0);
    }
    if (!m) {
	C_NUMBER(ret, 0);
    } else {
	/* ref count is properly decremented by get_author_stats */
	C_MAPPING(ret, m);
    }
}
#endif

#ifdef F_SET_AUTHOR
void c_set_author P2(svalue *, ret, svalue *, s0)
{
    set_author(s0->u.string);
}
#endif

#ifdef F_WRITE
void c_write P2(svalue *, ret, svalue *, s0)
{
    do_write(s0);
    C_NUMBER(ret, 0);
}
#endif

#ifdef F_WRITE_BYTES
void c_write_bytes P4(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2)
{
    int i;

    if (IS_ZERO(s2)) {
	bad_arg(3, F_WRITE_BYTES);
	C_NUMBER(ret, 0);
    } else {
	if (s2->type == T_NUMBER) {
	    int netint;
	    char *netbuf;

	    netint = htonl(s2->u.number);	/* convert to network
						 * byte-order */
	    netbuf = (char *) &netint;
	    i = write_bytes(s0->u.string, s1->u.number, netbuf,
			    sizeof(int));
	} else if (s2->type == T_BUFFER) {
	    i = write_bytes(s0->u.string, s1->u.number,
			    (char *)s2->u.buf->item, s2->u.buf->size);
	} else if (s2->type == T_STRING) {
	    i = write_bytes(s0->u.string, s1->u.number,
			    s2->u.string, strlen(s2->u.string));
	} else {
	    bad_arg(3, F_WRITE_BYTES);
	}
	C_NUMBER(ret, i);
    }
}
#endif

#ifdef F_WRITE_BUFFER
void c_write_buffer P4(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2)
{
    int i;

    if (IS_ZERO(s2)) {
	bad_arg(3, F_WRITE_BUFFER);
	C_NUMBER(ret, 0);
    } else {
	if (s0->type == T_STRING) {
	    c_write_bytes(ret, s0, s1, s2);
	    return;
	}
	if (s2->type == T_NUMBER) {
	    int netint;
	    char *netbuf;

	    netint = htonl(s2->u.number);	/* convert to network
						 * byte-order */
	    netbuf = (char *) &netint;
	    i = write_buffer(s0->u.buf, s1->u.number, netbuf, sizeof(int));
	} else if (s2->type == T_BUFFER) {
	    i = write_buffer(s0->u.buf, s1->u.number,
			     (char *) s2->u.buf->item, s2->u.buf->size);
	} else if (s2->type == T_STRING) {
	    i = write_buffer(s0->u.buf, s1->u.number,
			     s2->u.string, strlen(s2->u.string));
	} else {
	    bad_arg(3, F_WRITE_BUFFER);
	}
	C_NUMBER(ret, i);
    }
}
#endif

#ifdef F_WRITE_FILE
void c_write_file P4(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2)
{
    int i;
    int flags = 0;

    if (s2) {
	CHECK_TYPES(s2, T_NUMBER, 3, F_WRITE_FILE);
	flags = s2->u.number;
    }
    i = write_file(s0->u.string, s1->u.string, flags);
    C_NUMBER(ret, i);
}
#endif

#ifdef F_DUMP_FILE_DESCRIPTORS
void c_dump_file_descriptors P1(svalue *, ret)
{
    dump_file_descriptors();
    C_NUMBER(ret, 0);
}
#endif

#ifdef F_RECLAIM_OBJECTS
extern int reclaim_objects PROT((void));

void c_reclaim_objects P1(svalue *, ret)
{
    C_NUMBER(ret, reclaim_objects());
}
#endif

#if 0 /* needs to be updated */
extern struct vector *objects PROT((char *, struct object *));

void c_objects P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    char *func;
    struct object *ob;
    struct vector *vec;

    if (!s0)
	func = NULL;
    else {
	func = s0->u.string;
	if (!s1)
	    ob = current_object;
	else
	    ob = s1->u.ob;
    }
    vec = objects(func, ob);
    if (!vec) {
	C_NUMBER(ret, 0);
    } else {
	C_REFED_VECTOR(ret, vec);
    }
}
#endif

#ifdef F_MEMORY_INFO
void c_memory_info P2(svalue *, ret, svalue *, s0)
{
    struct object *ob;
    int mem;
    extern int total_prog_block_size;
    extern int total_array_size;
    extern int total_mapping_size;
    extern int tot_alloc_sentence;
    extern int tot_alloc_object_size;
    extern char *reserved_area;

    if (!s0) {
	int res, tot;

	if (reserved_area)
	    res = RESERVED_SIZE;
	else
	    res = 0;
	tot = total_prog_block_size + total_array_size + total_mapping_size +
	    tot_alloc_object_size + show_otable_status(-1) +
	    heart_beat_status(-1) + add_string_status(-1) +
	    tot_alloc_sentence +
	    print_call_out_usage(-1) + res;
	C_NUMBER(ret, tot);
	return;
    }
    if (s0->type != T_OBJECT)
	error("Bad argument 1 to memory_info()\n");
    ob = s0->u.ob;
    if (ob->prog && (ob->prog->p.i.ref == 1 || !(ob->flags & O_CLONE)))
	mem = ob->prog->p.i.total_size;
    else
	mem = 0;
    mem += (data_size(ob) + sizeof(struct object));
    C_NUMBER(ret, mem);
}
#endif

#ifdef F_RELOAD_OBJECT
void c_reload_object P2(svalue *, ret, svalue *, s0)
{
    if (s0->type != T_OBJECT)
	error("Bad arg 1 to reload_object()\n");
    reload_object(s0->u.ob);
    C_NUMBER(ret, 0);
}
#endif

#ifdef F_QUERY_SHADOWING
void c_query_shadowing P2(svalue *, ret, svalue *, s0)
{
    if ((s0->type == T_OBJECT) && s0->u.ob->shadowing) {
	struct object *ob;

	ob = s0->u.ob;
	C_OBJECT(ret, ob->shadowing);
    } else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_SET_RESET
void c_set_reset P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    if (s1) {
	s0->u.ob->next_reset = current_time + s1->u.number;
    } else {
	s0->u.ob->next_reset = current_time + TIME_TO_RESET / 2 +
	    random_number(TIME_TO_RESET / 2);
    }
    C_NUMBER(ret, 0);
}
#endif

#ifdef F_FLOATP
void c_floatp P2(svalue *, ret, svalue *, s0)
{
    if (s0->type == T_REAL)
	C_NUMBER(ret, 1);
    else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_FIRST_INVENTORY
void c_first_inventory P2(svalue *, ret, svalue *, s0)
{
    extern struct object *first_inventory PROT((struct svalue *));

    ob = first_inventory(s0);
    if (ob)
	C_OBJECT(ret, ob);
    else
	C_NUMBER(ret, 0);
}
#endif

#ifdef F_NEXT_INVENTORY
void c_next_inventory P2(svalue *, ret, svalue *, s0)
{
    ob = s0->u.ob;
    ob = ob->next_inv;
    while (ob) {
	if (ob->flags & O_HIDDEN) {
	    if (object_visible(ob)) {
		C_OBJECT(ret, ob);
		return;
	    }
	} else {
	    C_OBJECT(ret, ob);
	    return;
	}
	ob = ob->next_inv;
    }
    C_NUMBER(ret, 0);
}
#endif
#endif
