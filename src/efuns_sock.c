/*
	efuns_sock.c: this file contains the socket efunctions called from
	inside eval_instruction() in interpret.c.
*/

#include "config.h"
#include "efuns.h"
#include "stralloc.h"

#if defined(LATTICE)
#include <stdlib.h>
#endif
#if defined(__386BSD__) || defined(SunOS_5)
#include <unistd.h>
#endif

#include "lint.h"
#include "applies.h"

#ifdef F_SOCKET_CREATE
void
f_socket_create P2(int, num_arg, int, instruction)
{
    int fd;
    struct svalue *arg, *ret;
    struct vector *info;

    arg = sp - num_arg + 1;
    if ((num_arg == 3) && (arg[2].type != T_STRING)) {
	bad_arg(3, instruction);
    }
    info = allocate_array(4);
    info->ref--;
    info->item[0].type = T_NUMBER;
    info->item[0].u.number = -1;
    assign_socket_owner(&info->item[1], current_object);
    info->item[2].type = T_STRING;
    info->item[2].subtype = STRING_CONSTANT;
    info->item[2].u.string = "N/A";
    info->item[3].type = T_NUMBER;
    info->item[3].u.number = -1;

    push_object(current_object);
    push_string("create", STRING_CONSTANT);
    push_vector(info);

    ret = apply_master_ob(APPLY_VALID_SOCKET, 3);
    if (!IS_ZERO(ret)) {
	if (num_arg == 2)
	    fd = socket_create(arg[0].u.number, arg[1].u.string, NULL);
	else {
	    fd = socket_create(arg[0].u.number, arg[1].u.string, arg[2].u.string);
	}
	pop_n_elems(num_arg);	/* pop both args off stack    */
	push_number(fd);	/* push return int onto stack */
    } else {
	pop_n_elems(num_arg);	/* pop both args off stack    */
	push_number(EESECURITY);/* Security violation attempted */
    }
}
#endif

#ifdef F_SOCKET_BIND
void
f_socket_bind P2(int, num_arg, int, instruction)
{
    int i, fd, port;
    struct svalue *ret;
    struct vector *info;
    char addr[ADDR_BUF_SIZE];

    fd = (sp - 1)->u.number;
    get_socket_address(fd, addr, &port);

    info = allocate_array(4);
    info->ref--;
    info->item[0].type = T_NUMBER;
    info->item[0].u.number = fd;
    assign_socket_owner(&info->item[1], get_socket_owner(fd));
    info->item[2].type = T_STRING;
    info->item[2].subtype = STRING_CONSTANT;
    info->item[2].u.string = addr;
    info->item[3].type = T_NUMBER;
    info->item[3].u.number = port;

    push_object(current_object);
    push_string("bind", STRING_CONSTANT);
    push_vector(info);

    ret = apply_master_ob(APPLY_VALID_SOCKET, 3);
    if (!IS_ZERO(ret)) {
	i = socket_bind(fd, sp->u.number);
	pop_n_elems(2);		/* pop both args off stack    */
	push_number(i);		/* push return int onto stack */
    } else {
	pop_n_elems(2);		/* pop both args off stack    */
	push_number(EESECURITY);/* Security violation attempted */
    }
}
#endif

#ifdef F_SOCKET_LISTEN
void
f_socket_listen P2(int, num_arg, int, instruction)
{
    int i, fd, port;
    struct svalue *ret;
    struct vector *info;
    char addr[ADDR_BUF_SIZE];

    fd = (sp - 1)->u.number;
    get_socket_address(fd, addr, &port);

    info = allocate_array(4);
    info->ref--;
    info->item[0].type = T_NUMBER;
    info->item[0].u.number = fd;
    assign_socket_owner(&info->item[1], get_socket_owner(fd));
    info->item[2].type = T_STRING;
    info->item[2].subtype = STRING_CONSTANT;
    info->item[2].u.string = addr;
    info->item[3].type = T_NUMBER;
    info->item[3].u.number = port;

    push_object(current_object);
    push_string("listen", STRING_CONSTANT);
    push_vector(info);

    ret = apply_master_ob(APPLY_VALID_SOCKET, 3);
    if (!IS_ZERO(ret)) {
	i = socket_listen(fd, sp->u.string);
	pop_n_elems(2);		/* pop both args off stack    */
	push_number(i);		/* push return int onto stack */
    } else {
	pop_n_elems(2);		/* pop both args off stack    */
	push_number(EESECURITY);/* Security violation attempted */
    }
}
#endif

#ifdef F_SOCKET_ACCEPT
void
f_socket_accept P2(int, num_arg, int, instruction)
{
    int i, fd, port;
    struct svalue *ret;
    struct vector *info;
    char addr[ADDR_BUF_SIZE];

    if (sp->type != T_STRING) {
	bad_arg(3, instruction);
    }
    fd = (sp - 2)->u.number;
    get_socket_address(fd, addr, &port);

    info = allocate_array(4);
    info->ref--;
    info->item[0].type = T_NUMBER;
    info->item[0].u.number = fd;
    assign_socket_owner(&info->item[1], get_socket_owner(fd));
    info->item[2].type = T_STRING;
    info->item[2].subtype = STRING_CONSTANT;
    info->item[2].u.string = addr;
    info->item[3].type = T_NUMBER;
    info->item[3].u.number = port;

    push_object(current_object);
    push_string("accept", STRING_CONSTANT);
    push_vector(info);

    ret = apply_master_ob(APPLY_VALID_SOCKET, 3);
    if (!IS_ZERO(ret)) {
	i = socket_accept(fd, (sp - 1)->u.string, sp->u.string);
	pop_n_elems(3);		/* pop both args off stack    */
	push_number(i);		/* push return int onto stack */
    } else {
	pop_n_elems(3);		/* pop both args off stack    */
	push_number(EESECURITY);/* Security violation attempted */
    }
}
#endif

#ifdef F_SOCKET_CONNECT
void
f_socket_connect P2(int, num_arg, int, instruction)
{
    int i, fd, port;
    struct svalue *ret;
    struct vector *info;
    char addr[ADDR_BUF_SIZE];

    if ((sp - 1)->type != T_STRING) {
	bad_arg(3, instruction);
    }
    if (sp->type != T_STRING) {
	bad_arg(4, instruction);
    }
    fd = (sp - 3)->u.number;
    get_socket_address(fd, addr, &port);

    if (!strcmp(addr, "0.0.0.0") && port == 0) {
	/*
	 * socket descriptor is not bound yet
	 */
	char *s;
	int start = 0;

	addr[0] = '\0';
	if (s = strchr((sp - 2)->u.string, ' ')) {
	    /*
	     * use specified address and port
	     */
	    i = s - (sp - 2)->u.string;
	    if (i > ADDR_BUF_SIZE - 1) {
		start = i - ADDR_BUF_SIZE - 1;
		i = ADDR_BUF_SIZE - 1;
	    }
	    strncat(addr, (sp - 2)->u.string + start, i);
	    port = atoi(s + 1);
	}
#ifdef DEBUG
    } else {
	fprintf(stderr, "socket_connect: socket already bound to address/port: %s/%d\n",
		addr, port);
	fprintf(stderr, "socket_connect: requested on: %s\n", (sp - 2)->u.string);
#endif
    }

    info = allocate_array(4);
    info->ref--;
    info->item[0].type = T_NUMBER;
    info->item[0].u.number = fd;
    assign_socket_owner(&info->item[1], get_socket_owner(fd));
    info->item[2].type = T_STRING;
    info->item[2].subtype = STRING_CONSTANT;
    info->item[2].u.string = addr;
    info->item[3].type = T_NUMBER;
    info->item[3].u.number = port;

    push_object(current_object);
    push_string("connect", STRING_CONSTANT);
    push_vector(info);

    ret = apply_master_ob(APPLY_VALID_SOCKET, 3);
    if (!IS_ZERO(ret)) {
	i = socket_connect(fd, (sp - 2)->u.string, (sp - 1)->u.string,
			   sp->u.string);
	pop_n_elems(4);		/* pop all args off stack     */
	push_number(i);		/* push return int onto stack */
    } else {
	pop_n_elems(4);		/* pop all args off stack     */
	push_number(EESECURITY);/* Security violation attempted */
    }
}
#endif

#ifdef F_SOCKET_WRITE
void
f_socket_write P2(int, num_arg, int, instruction)
{
    int i, fd, port;
    struct svalue *arg, *ret;
    struct vector *info;
    char addr[ADDR_BUF_SIZE];

    arg = sp - num_arg + 1;
    if ((num_arg == 3) && (arg[2].type != T_STRING)) {
	bad_arg(3, instruction);
    }
    fd = arg[0].u.number;
    get_socket_address(fd, addr, &port);

    info = allocate_array(4);
    info->ref--;
    info->item[0].type = T_NUMBER;
    info->item[0].u.number = fd;
    assign_socket_owner(&info->item[1], get_socket_owner(fd));
    info->item[2].type = T_STRING;
    info->item[2].subtype = STRING_CONSTANT;
    info->item[2].u.string = addr;
    info->item[3].type = T_NUMBER;
    info->item[3].u.number = port;

    push_object(current_object);
    push_string("write", STRING_CONSTANT);
    push_vector(info);

    ret = apply_master_ob(APPLY_VALID_SOCKET, 3);
    if (!IS_ZERO(ret)) {
	i = socket_write(fd, &arg[1],
			 (num_arg == 3) ? arg[2].u.string : (char *) NULL);
	pop_n_elems(num_arg);	/* pop both args off stack    */
	push_number(i);		/* push return int onto stack */
    } else {
	pop_n_elems(num_arg);	/* pop both args off stack    */
	push_number(EESECURITY);/* Security violation attempted */
    }
}
#endif

#ifdef F_SOCKET_CLOSE
void
f_socket_close P2(int, num_arg, int, instruction)
{
    int i, fd, port;
    struct svalue *ret;
    struct vector *info;
    char addr[ADDR_BUF_SIZE];

    fd = sp->u.number;
    get_socket_address(fd, addr, &port);

    info = allocate_array(4);
    info->ref--;
    info->item[0].type = T_NUMBER;
    info->item[0].u.number = fd;
    assign_socket_owner(&info->item[1], get_socket_owner(fd));
    info->item[2].type = T_STRING;
    info->item[2].subtype = STRING_CONSTANT;
    info->item[2].u.string = addr;
    info->item[3].type = T_NUMBER;
    info->item[3].u.number = port;

    push_object(current_object);
    push_string("close", STRING_CONSTANT);
    push_vector(info);

    ret = apply_master_ob(APPLY_VALID_SOCKET, 3);
    if (!IS_ZERO(ret)) {
	i = socket_close(fd);
	pop_stack();		/* pop int arg off stack      */
	push_number(i);		/* push return int onto stack */
    } else {
	pop_stack();		/* pop int arg off stack      */
	push_number(EESECURITY);/* Security violation attempted */
    }
}
#endif

#ifdef F_SOCKET_RELEASE
void
f_socket_release P2(int, num_arg, int, instruction)
{
    int i, fd, port;
    struct svalue *ret;
    struct vector *info;
    char addr[ADDR_BUF_SIZE];

    if (sp->type != T_STRING) {
	bad_arg(3, instruction);
    }
    fd = (sp - 2)->u.number;
    get_socket_address(fd, addr, &port);

    info = allocate_array(4);
    info->ref--;
    info->item[0].type = T_NUMBER;
    info->item[0].u.number = fd;
    assign_socket_owner(&info->item[1], get_socket_owner(fd));
    info->item[2].type = T_STRING;
    info->item[2].subtype = STRING_CONSTANT;
    info->item[2].u.string = addr;
    info->item[3].type = T_NUMBER;
    info->item[3].u.number = port;

    push_object(current_object);
    push_string("release", STRING_CONSTANT);
    push_vector(info);

    ret = apply_master_ob(APPLY_VALID_SOCKET, 3);
    if (!IS_ZERO(ret)) {
	i = socket_release((sp - 2)->u.number, (sp - 1)->u.ob, sp->u.string);
	pop_n_elems(3);		/* pop all args off stack     */
	push_number(i);		/* push return int onto stack */
    } else {
	pop_n_elems(3);		/* pop all args off stack     */
	push_number(EESECURITY);/* Security violation attempted */
    }
}
#endif

#ifdef F_SOCKET_ACQUIRE
void
f_socket_acquire P2(int, num_arg, int, instruction)
{
    int i, fd, port;
    struct svalue *ret;
    struct vector *info;
    char addr[ADDR_BUF_SIZE];

    if ((sp - 1)->type != T_STRING) {
	bad_arg(3, instruction);
    }
    if (sp->type != T_STRING) {
	bad_arg(4, instruction);
    }
    fd = (sp - 1)->u.number;
    get_socket_address(fd, addr, &port);

    info = allocate_array(4);
    info->ref--;
    info->item[0].type = T_NUMBER;
    info->item[0].u.number = fd;
    assign_socket_owner(&info->item[1], get_socket_owner(fd));
    info->item[2].type = T_STRING;
    info->item[2].subtype = STRING_CONSTANT;
    info->item[2].u.string = addr;
    info->item[3].type = T_NUMBER;
    info->item[3].u.number = port;

    push_object(current_object);
    push_string("acquire", STRING_CONSTANT);
    push_vector(info);

    ret = apply_master_ob(APPLY_VALID_SOCKET, 3);
    if (!IS_ZERO(ret)) {
	i = socket_acquire((sp - 3)->u.number, (sp - 2)->u.string,
			   (sp - 1)->u.string, sp->u.string);
	pop_n_elems(4);		/* pop both args off stack    */
	push_number(i);		/* push return int onto stack */
    } else {
	pop_n_elems(4);		/* pop both args off stack    */
	push_number(EESECURITY);/* Security violation attempted */
    }
}
#endif

#ifdef F_SOCKET_ERROR
void
f_socket_error P2(int, num_arg, int, instruction)
{
    char *error;

    error = socket_error(sp->u.number);
    pop_stack();		/* pop int arg off stack      */
    push_string(error, STRING_CONSTANT);	/* push return string onto
						 * stack */
}
#endif

#ifdef F_SOCKET_ADDRESS
void
f_socket_address P2(int, num_arg, int, instruction)
{
    char *str;
    int port;
    char addr[ADDR_BUF_SIZE];

/*
 * Ok, we will add in a cute little check thing here to see if it is
 * an object or not...
 */
    if (sp->type == T_OBJECT) {
	char *tmp;

/* This is so we can get the address of interactives as well. */

	if (!sp->u.ob->interactive) {
	    pop_stack();
	    push_null();
	    return;
	}
	tmp = inet_ntoa(sp->u.ob->interactive->addr.sin_addr);
	str = (char *) DMALLOC(strlen(tmp) + 5 + 3, 33, "f_socket_address");
	sprintf(str, "%s %d", tmp, sp->u.ob->interactive->addr.sin_port);
	pop_stack();
	push_malloced_string(str);
	return;
    }
    get_socket_address(sp->u.number, addr, &port);
    str = (char *) DMALLOC(strlen(addr) + 5 + 3, 33, "f_socket_address");
    sprintf(str, "%s %d", addr, port);
    pop_stack();
    push_malloced_string(str);
}				/* f_socket_address() */
#endif

#ifdef F_DUMP_SOCKET_STATUS
void
f_dump_socket_status P2(int, num_arg, int, instruction)
{
    dump_socket_status();
    push_number(0);
}
#endif
