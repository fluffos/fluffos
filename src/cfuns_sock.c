/*
	efuns_sock.c: this file contains the socket efunctions called from
	inside eval_instruction() in interpret.c.
*/

#include "efuns.h"
#include "cfuns.h"
#include "stralloc.h"
#if defined(__386BSD__) || defined(SunOS_5)
#include <unistd.h>
#endif
#include "applies.h"

#ifdef LPC_TO_C
#ifdef F_SOCKET_CREATE
void c_socket_create P4(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2)
{
    int fd;
    struct svalue *mret;
    struct vector *info;

    if (s2 && (s2->type != T_STRING))
	bad_arg(3, F_SOCKET_CREATE);

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

    mret = apply_master_ob(APPLY_VALID_SOCKET, 3);
    if (!IS_ZERO(mret)) {
	fd = socket_create(s0->u.number, s1->u.string, s2 ? s2->u.string : NULL);
	C_NUMBER(ret, fd);
    } else {
	C_NUMBER(ret, EESECURITY);	/* Security violation attempted */
    }
}
#endif

#ifdef F_SOCKET_BIND
void c_socket_bind P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    int i, fd, port;
    struct svalue *mret;
    struct vector *info;
    char addr[ADDR_BUF_SIZE];

    fd = s0->u.number;
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

    mret = apply_master_ob(APPLY_VALID_SOCKET, 3);
    if (!IS_ZERO(mret)) {
	i = socket_bind(fd, s1->u.number);
	C_NUMBER(ret, i);
    } else {
	C_NUMBER(ret, EESECURITY);	/* Security violation attempted */
    }
}
#endif

#ifdef F_SOCKET_LISTEN
void c_socket_listen P3(svalue *, ret, svalue *, s0, svalue *, s1)
{
    int i, fd, port;
    struct svalue *mret;
    struct vector *info;
    char addr[ADDR_BUF_SIZE];

    fd = s0->u.number;
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

    mret = apply_master_ob(APPLY_VALID_SOCKET, 3);
    if (!IS_ZERO(mret)) {
	i = socket_listen(fd, s1->u.string);
	C_NUMBER(ret, i);
    } else {
	C_NUMBER(ret, EESECURITY);	/* Security violation attempted */
    }
}
#endif

#ifdef F_SOCKET_ACCEPT
void c_socket_accept P4(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2)
{
    int i, fd, port;
    struct svalue *mret;
    struct vector *info;
    char addr[ADDR_BUF_SIZE];

    if (s2->type != T_STRING) {
	bad_arg(3, F_SOCKET_ACCEPT);
    }
    fd = s0->u.number;
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

    mret = apply_master_ob(APPLY_VALID_SOCKET, 3);
    if (!IS_ZERO(mret)) {
	i = socket_accept(fd, s1->u.string, s2->u.string);
	C_NUMBER(ret, i);
    } else {
	C_NUMBER(ret, EESECURITY);	/* Security violation attempted */
    }
}
#endif

#ifdef F_SOCKET_CONNECT
void c_socket_connect P5(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2, svalue *, s3)
{
    int i, fd, port;
    struct svalue *mret;
    struct vector *info;
    char addr[ADDR_BUF_SIZE];

    if (s2->type != T_STRING) {
	bad_arg(3, F_SOCKET_CONNECT);
    }
    if (s3->type != T_STRING) {
	bad_arg(4, F_SOCKET_CONNECT);
    }
    fd = s0->u.number;
    get_socket_address(fd, addr, &port);

    if (!strcmp(addr, "0.0.0.0") && port == 0) {
	/*
	 * socket descriptor is not bound yet
	 */
	char *s;
	int start = 0;

	addr[0] = '\0';
	if (s = strchr(s1->u.string, ' ')) {
	    /*
	     * use specified address and port
	     */
	    i = s - s1->u.string;
	    if (i > ADDR_BUF_SIZE - 1) {
		start = i - ADDR_BUF_SIZE - 1;
		i = ADDR_BUF_SIZE - 1;
	    }
	    strncat(addr, s1->u.string + start, i);
	    port = atoi(s + 1);
	}
#ifdef DEBUG
    } else {
	fprintf(stderr, "socket_connect: socket already bound to address/port: %s/%d\n",
		addr, port);
	fprintf(stderr, "socket_connect: requested on: %s\n", s1->u.string);
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

    mret = apply_master_ob(APPLY_VALID_SOCKET, 3);
    if (!IS_ZERO(mret)) {
	i = socket_connect(fd, s1->u.string, s2->u.string,
			   s3->u.string);
	C_NUMBER(ret, i);
    } else {
	C_NUMBER(ret, EESECURITY);	/* Security violation attempted */
    }
}
#endif

#ifdef F_SOCKET_WRITE
void c_socket_write P4(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2)
{
    int i, fd, port;
    struct svalue *mret;
    struct vector *info;
    char addr[ADDR_BUF_SIZE];

    if (s2 && s2->type != T_STRING) {
	bad_arg(3, F_SOCKET_WRITE);
    }
    fd = s0->u.number;
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

    mret = apply_master_ob(APPLY_VALID_SOCKET, 3);
    if (!IS_ZERO(mret)) {
	i = socket_write(fd, s1, s2 ? s2->u.string : (char *) NULL);
	C_NUMBER(ret, i);
    } else {
	C_NUMBER(ret, EESECURITY);	/* Security violation attempted */
    }
}
#endif

#ifdef F_SOCKET_CLOSE
void c_socket_close P2(svalue *, ret, svalue *, s0)
{
    int i, fd, port;
    struct svalue *mret;
    struct vector *info;
    char addr[ADDR_BUF_SIZE];

    fd = s0->u.number;
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

    mret = apply_master_ob(APPLY_VALID_SOCKET, 3);
    if (!IS_ZERO(mret)) {
	i = socket_close(fd);
	C_NUMBER(ret, i);
    } else {
	C_NUMBER(ret, EESECURITY);	/* Security violation attempted */
    }
}
#endif

#ifdef F_SOCKET_RELEASE
void c_socket_release P4(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2)
{
    int i, fd, port;
    struct svalue *mret;
    struct vector *info;
    char addr[ADDR_BUF_SIZE];

    if (s2->type != T_STRING) {
	bad_arg(3, F_SOCKET_RELEASE);
    }
    fd = s0->u.number;
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

    mret = apply_master_ob(APPLY_VALID_SOCKET, 3);
    if (!IS_ZERO(mret)) {
	i = socket_release(s0->u.number, s1->u.ob, s2->u.string);
	C_NUMBER(ret, i);
    } else {
	C_NUMBER(ret, EESECURITY);
    }
}
#endif

#ifdef F_SOCKET_ACQUIRE
void c_socket_acquire P5(svalue *, ret, svalue *, s0, svalue *, s1, svalue *, s2, svalue *, s3)
{
    int i, fd, port;
    struct svalue *mret;
    struct vector *info;
    char addr[ADDR_BUF_SIZE];

    if (s2->type != T_STRING) {
	bad_arg(3, F_SOCKET_ACQUIRE);
    }
    if (s3->type != T_STRING) {
	bad_arg(4, F_SOCKET_ACQUIRE);
    }
    fd = s2->u.number;
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

    mret = apply_master_ob(APPLY_VALID_SOCKET, 3);
    if (!IS_ZERO(mret)) {
	i = socket_acquire(s0->u.number, s1->u.string,
			   s2->u.string, s3->u.string);
	C_NUMBER(ret, i);
    } else {
	C_NUMBER(ret, EESECURITY);	/* Security violation attempted */
    }
}
#endif

#ifdef F_SOCKET_ERROR
void c_socket_error P2(svalue *, ret, svalue *, s0)
{
    char *error;

    error = socket_error(s0->u.number);
    C_CONSTANT_STRING(ret, error);	/* push return string onto stack */
}
#endif

#ifdef F_SOCKET_ADDRESS
void c_socket_address P2(svalue *, ret, svalue *, s0)
{
    char *str;
    int port;
    char addr[ADDR_BUF_SIZE];

/*
 * Ok, we will add in a cute little check thing here to see if it is
 * an object or not...
 */
    if (s0->type == T_OBJECT) {
	char *tmp;

/* This is so we can get the address of interactives as well. */

	if (!s0->u.ob->interactive) {
	    C_NUMBER(ret, 0);
	    return;
	}
	tmp = inet_ntoa(s0->u.ob->interactive->addr.sin_addr);
	str = (char *) DMALLOC(strlen(tmp) + 5 + 3, 33, "f_socket_address");
	sprintf(str, "%s %d", tmp, (int)s0->u.ob->interactive->addr.sin_port);
	C_MALLOCED_STRING(ret, str);
	return;
    }
    get_socket_address(s0->u.number, addr, &port);
    str = (char *) DMALLOC(strlen(addr) + 5 + 3, 33, "f_socket_address");
    sprintf(str, "%s %d", addr, port);
    C_MALLOCED_STRING(ret, str);
}				/* f_socket_address() */
#endif

#ifdef F_DUMP_SOCKET_STATUS
void c_dump_socket_status P1(svalue *, ret)
{
    dump_socket_status();
    C_NUMBER(ret, 0);
}
#endif

#endif
