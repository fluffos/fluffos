/*
 * socket_efun.c -- socket efuns for MudOS.
 *    5-92 : Dwayne Fontenot (Jacques@TMI) : original coding.
 *   10-92 : Dave Richards (Cynosure) : less original coding.
 */

#include "std.h"
#include "network_incl.h"
#include "lpc_incl.h"
#include "socket_efuns.h"
#include "socket_err.h"
#include "include/socket_err.h"
#include "debug.h"
#include "socket_ctrl.h"
#include "comm.h"
#include "eoperators.h"
#include "file.h"

#ifdef PACKAGE_SOCKETS

/* flags for socket_close */
#define SC_FORCE	1
#define SC_DO_CALLBACK	2

lpc_socket_t lpc_socks[MAX_EFUN_SOCKS];
static int socket_name_to_sin PROT((char *, struct sockaddr_in *));
static char *inet_address PROT((struct sockaddr_in *));

/*
 * check permission
 */
int check_valid_socket P5(char *, what, int, fd, object_t *, owner,
			  char *, addr, int, port) {
    array_t *info;
    svalue_t *mret;
    
    info = allocate_empty_array(4);
    info->item[0].type = T_NUMBER;
    info->item[0].u.number = fd;
    assign_socket_owner(&info->item[1], owner);
    info->item[2].type = T_STRING;
    info->item[2].subtype = STRING_SHARED;
    info->item[2].u.string = make_shared_string(addr);
    info->item[3].type = T_NUMBER;
    info->item[3].u.number = port;

    push_object(current_object);
    push_string(what, STRING_CONSTANT);
    push_refed_array(info);

    mret = apply_master_ob(APPLY_VALID_SOCKET, 3);
    return MASTER_APPROVED(mret);
}

/*
 * Initialize the LPC efun socket array
 */
void init_sockets()
{
    int i;

    debug(8192, ("init_sockets: initializing %d socket descriptor(s)\n",
		 MAX_EFUN_SOCKS));

    for (i = 0; i < MAX_EFUN_SOCKS; i++) {
	lpc_socks[i].fd = -1;
	lpc_socks[i].flags = 0;
	lpc_socks[i].mode = MUD;
	lpc_socks[i].state = CLOSED;
	memset((char *) &lpc_socks[i].l_addr, 0, sizeof(lpc_socks[i].l_addr));
	memset((char *) &lpc_socks[i].r_addr, 0, sizeof(lpc_socks[i].r_addr));
	lpc_socks[i].name[0] = '\0';
	lpc_socks[i].owner_ob = NULL;
	lpc_socks[i].release_ob = NULL;
	lpc_socks[i].read_callback.s = 0;
	lpc_socks[i].write_callback.s = 0;
	lpc_socks[i].close_callback.s = 0;
	lpc_socks[i].r_buf = NULL;
	lpc_socks[i].r_off = 0;
	lpc_socks[i].r_len = 0;
	lpc_socks[i].w_buf = NULL;
	lpc_socks[i].w_off = 0;
	lpc_socks[i].w_len = 0;
    }
}

/*
 * Set the callbacks for a socket
 */
static void
set_read_callback P2(int, which, svalue_t *, cb) {
    char *s;

    if (lpc_socks[which].flags & S_READ_FP)
	free_funp(lpc_socks[which].read_callback.f);
    else if ((s = lpc_socks[which].read_callback.s))
	free_string(s);

    if (cb) {
	if (cb->type == T_FUNCTION) {
	    lpc_socks[which].flags |= S_READ_FP;
	    lpc_socks[which].read_callback.f = cb->u.fp;
	    cb->u.fp->hdr.ref++;
	} else {
	    lpc_socks[which].read_callback.s = make_shared_string(cb->u.string);
	}
    } else 
	lpc_socks[which].read_callback.s = 0;
}

static void
set_write_callback P2(int, which, svalue_t *, cb) {
    char *s;

    if (lpc_socks[which].flags & S_WRITE_FP)
	free_funp(lpc_socks[which].write_callback.f);
    else if ((s = lpc_socks[which].write_callback.s))
	free_string(s);

    if (cb) {
	if (cb->type == T_FUNCTION) {
	    lpc_socks[which].flags |= S_WRITE_FP;
	    lpc_socks[which].write_callback.f = cb->u.fp;
	    cb->u.fp->hdr.ref++;
	} else {
	    lpc_socks[which].write_callback.s = make_shared_string(cb->u.string);
	}
    } else 
	lpc_socks[which].write_callback.s = 0;
}

static void
set_close_callback P2(int, which, svalue_t *, cb) {
    char *s;

    if (lpc_socks[which].flags & S_CLOSE_FP)
	free_funp(lpc_socks[which].close_callback.f);
    else if ((s = lpc_socks[which].close_callback.s))
	free_string(s);

    if (cb) {
	if (cb->type == T_FUNCTION) {
	    lpc_socks[which].flags |= S_CLOSE_FP;
	    lpc_socks[which].close_callback.f = cb->u.fp;
	    cb->u.fp->hdr.ref++;
	} else {
	    lpc_socks[which].close_callback.s = make_shared_string(cb->u.string);
	}
    } else 
	lpc_socks[which].close_callback.s = 0;
}

static void
copy_close_callback P2(int, to, int, from) {
    char *s;

    if (lpc_socks[to].flags & S_CLOSE_FP)
	free_funp(lpc_socks[to].close_callback.f);
    else if ((s = lpc_socks[to].close_callback.s))
	free_string(s);

    if (lpc_socks[from].flags & S_CLOSE_FP) {
	lpc_socks[to].flags |= S_CLOSE_FP;
	lpc_socks[to].close_callback.f = lpc_socks[from].close_callback.f;
	lpc_socks[to].close_callback.f->hdr.ref++;
    } else {
	lpc_socks[to].flags &= ~S_CLOSE_FP;
	s = lpc_socks[to].close_callback.s = lpc_socks[from].close_callback.s;
	if (s)
	    ref_string(s);
    }
}

static int 
find_new_socket PROT((void)) {
    int i;
    
    for (i = 0; i < MAX_EFUN_SOCKS; i++) {
	if (lpc_socks[i].state != CLOSED) continue;
	set_read_callback(i, 0);
	set_write_callback(i, 0);
	set_close_callback(i, 0);
	return i;
    }
    return EENOSOCKS;
}

/*
 * Create an LPC efun socket
 */
int
socket_create P3(enum socket_mode, mode, svalue_t *, read_callback, svalue_t *, close_callback)
{
    int type, i, fd, optval;
    int binary = 0;

    if (mode == STREAM_BINARY) {
	binary = 1;
	mode = STREAM;
    } else if (mode == DATAGRAM_BINARY) {
	binary = 1;
	mode = DATAGRAM;
    }
    switch (mode) {

    case MUD:
    case STREAM:
	type = SOCK_STREAM;
	break;
    case DATAGRAM:
	type = SOCK_DGRAM;
	break;

    default:
	return EEMODENOTSUPP;
    }

    i = find_new_socket();
    if (i >= 0) {
	fd = socket(AF_INET, type, 0);
	if (fd == -1) {
	    debug_perror("socket_create: socket", 0);
	    return EESOCKET;
	}
	optval = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &optval,
		       sizeof(optval)) == -1) {
	    debug_perror("socket_create: setsockopt", 0);
	    close(fd);
	    return EESETSOCKOPT;
	}
	if (set_socket_nonblocking(fd, 1) == -1) {
	    debug_perror("socket_create: set_socket_nonblocking", 0);
	    close(fd);
	    return EENONBLOCK;
	}
	lpc_socks[i].fd = fd;
	lpc_socks[i].flags = S_HEADER;

	if (type == SOCK_DGRAM) close_callback = 0;
	set_read_callback(i, read_callback);
	set_write_callback(i, 0);
	set_close_callback(i, close_callback);

	if (binary) {
	    lpc_socks[i].flags |= S_BINARY;
	}
	lpc_socks[i].mode = mode;
	lpc_socks[i].state = UNBOUND;
	memset((char *) &lpc_socks[i].l_addr, 0, sizeof(lpc_socks[i].l_addr));
	memset((char *) &lpc_socks[i].r_addr, 0, sizeof(lpc_socks[i].r_addr));
	lpc_socks[i].name[0] = '\0';
	lpc_socks[i].owner_ob = current_object;
	lpc_socks[i].release_ob = NULL;
	lpc_socks[i].r_buf = NULL;
	lpc_socks[i].r_off = 0;
	lpc_socks[i].r_len = 0;
	lpc_socks[i].w_buf = NULL;
	lpc_socks[i].w_off = 0;
	lpc_socks[i].w_len = 0;

	current_object->flags |= O_EFUN_SOCKET;

	debug(8192, ("socket_create: created socket %d mode %d fd %d\n",
		     i, mode, fd));
    }

    return i;
}

/*
 * Bind an address to an LPC efun socket
 */
int
socket_bind P2(int, fd, int, port)
{
    int len;
    struct sockaddr_in sin;

    if (fd < 0 || fd >= MAX_EFUN_SOCKS)
	return EEFDRANGE;
    if (lpc_socks[fd].state == CLOSED)
	return EEBADF;
    if (lpc_socks[fd].owner_ob != current_object)
	return EESECURITY;
    if (lpc_socks[fd].state != UNBOUND)
	return EEISBOUND;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons((u_short) port);

    if (bind(lpc_socks[fd].fd, (struct sockaddr *) & sin, sizeof(sin)) == -1) {
	switch (errno) {
	case EADDRINUSE:
	    return EEADDRINUSE;
	default:
	    debug_perror("socket_bind: bind", 0);
	    return EEBIND;
	}
    }
    len = sizeof(sin);
    if (getsockname(lpc_socks[fd].fd, (struct sockaddr *) & lpc_socks[fd].l_addr, &len) == -1) {
	debug_perror("socket_bind: getsockname", 0);
	return EEGETSOCKNAME;
    }
    lpc_socks[fd].state = BOUND;

    debug(8192, ("socket_bind: bound socket %d to %s.%d\n",
		 fd, inet_ntoa(lpc_socks[fd].l_addr.sin_addr),
		 ntohs(lpc_socks[fd].l_addr.sin_port)));

    return EESUCCESS;
}

/*
 * Listen for connections on an LPC efun socket
 */
int
socket_listen P2(int, fd, svalue_t *, callback)
{
    if (fd < 0 || fd >= MAX_EFUN_SOCKS)
	return EEFDRANGE;
    if (lpc_socks[fd].state == CLOSED)
	return EEBADF;
    if (lpc_socks[fd].owner_ob != current_object)
	return EESECURITY;
    if (lpc_socks[fd].mode == DATAGRAM)
	return EEMODENOTSUPP;
    if (lpc_socks[fd].state == UNBOUND)
	return EENOADDR;
    if (lpc_socks[fd].state != BOUND)
	return EEISCONN;

    if (listen(lpc_socks[fd].fd, 5) == -1) {
	debug_perror("socket_listen: listen", 0);
	return EELISTEN;
    }
    lpc_socks[fd].state = LISTEN;
    set_read_callback(fd, callback);

    current_object->flags |= O_EFUN_SOCKET;

    debug(8192, ("socket_listen: listen on socket %d\n", fd));

    return EESUCCESS;
}

/*
 * Accept a connection on an LPC efun socket
 */
int
socket_accept P3(int, fd, svalue_t *, read_callback, svalue_t *, write_callback)
{
    int len, accept_fd, i;
    struct sockaddr_in sin;
    struct hostent *hp;

    if (fd < 0 || fd >= MAX_EFUN_SOCKS)
	return EEFDRANGE;
    if (lpc_socks[fd].state == CLOSED)
	return EEBADF;
    if (lpc_socks[fd].owner_ob != current_object)
	return EESECURITY;
    if (lpc_socks[fd].mode == DATAGRAM)
	return EEMODENOTSUPP;
    if (lpc_socks[fd].state != LISTEN)
	return EENOTLISTN;

    lpc_socks[fd].flags &= ~S_WACCEPT;

    len = sizeof(sin);
    accept_fd = accept(lpc_socks[fd].fd, (struct sockaddr *) & sin, (int *) &len);
    if (accept_fd == -1) {
	switch (errno) {
	case EWOULDBLOCK:
	    return EEWOULDBLOCK;
	case EINTR:
	    return EEINTR;
	default:
	    debug_perror("socket_accept: accept", 0);
	    return EEACCEPT;
	}
    }
    i = find_new_socket();
    if (i >= 0) {
	fd_set wmask;
	struct timeval t;
	int nb;

	lpc_socks[i].fd = accept_fd;
	lpc_socks[i].flags = S_HEADER |
	    (lpc_socks[fd].flags & S_BINARY);

	FD_ZERO(&wmask);
	FD_SET(accept_fd, &wmask);
	t.tv_sec = 0;
	t.tv_usec = 0;
#ifndef hpux
	nb = select(FD_SETSIZE, (fd_set *) 0, &wmask, (fd_set *) 0, &t);
#else
	nb = select(FD_SETSIZE, (int *) 0, (int *) &wmask, (int *) 0, &t);
#endif
	if (!(FD_ISSET(accept_fd, &wmask)))
	    lpc_socks[i].flags |= S_BLOCKED;

	lpc_socks[i].mode = lpc_socks[fd].mode;
	lpc_socks[i].state = DATA_XFER;
	lpc_socks[i].l_addr = lpc_socks[fd].l_addr;
	lpc_socks[i].r_addr = sin;
	lpc_socks[i].owner_ob = NULL;
	lpc_socks[i].release_ob = NULL;
	lpc_socks[i].r_buf = NULL;
	lpc_socks[i].r_off = 0;
	lpc_socks[i].r_len = 0;
	lpc_socks[i].w_buf = NULL;
	lpc_socks[i].w_off = 0;
	lpc_socks[i].w_len = 0;

#ifdef cray
/* cray can't take addresses of bitfields */
	hp = gethostbyaddr((char *) &sin.sin_addr,
			   (int) sizeof(sin.sin_addr), AF_INET);
#else
	hp = gethostbyaddr((char *) &sin.sin_addr.s_addr,
			   (int) sizeof(sin.sin_addr.s_addr), AF_INET);
#endif
	if (hp != NULL) {
	    strncpy(lpc_socks[i].name, hp->h_name, ADDR_BUF_SIZE);
	    lpc_socks[i].name[ADDR_BUF_SIZE - 1] = '\0';
	} else
	    lpc_socks[i].name[0] = '\0';

	lpc_socks[i].owner_ob = current_object;
	set_read_callback(i, read_callback);
	set_write_callback(i, write_callback);
	copy_close_callback(i, fd);

	current_object->flags |= O_EFUN_SOCKET;

	debug(8192, ("socket_accept: accept on socket %d\n", fd));
	debug(8192, ("socket_accept: new socket %d on fd %d\n", i, accept_fd));
    } else
	close(accept_fd);

    return i;
}

/*
 * Connect an LPC efun socket
 */
int
socket_connect P4(int, fd, char *, name, svalue_t *, read_callback, svalue_t *, write_callback)
{
    if (fd < 0 || fd >= MAX_EFUN_SOCKS)
	return EEFDRANGE;
    if (lpc_socks[fd].state == CLOSED)
	return EEBADF;
    if (lpc_socks[fd].owner_ob != current_object)
	return EESECURITY;
    if (lpc_socks[fd].mode == DATAGRAM)
	return EEMODENOTSUPP;
    switch (lpc_socks[fd].state) {
    case CLOSED:
    case UNBOUND:
    case BOUND:
	break;
    case LISTEN:
	return EEISLISTEN;
    case DATA_XFER:
	return EEISCONN;
    }

    if (!socket_name_to_sin(name, &lpc_socks[fd].r_addr))
	return EEBADADDR;

    set_read_callback(fd, read_callback);
    set_write_callback(fd, write_callback);

    current_object->flags |= O_EFUN_SOCKET;

    if (connect(lpc_socks[fd].fd, (struct sockaddr *) & lpc_socks[fd].r_addr,
		sizeof(struct sockaddr_in)) == -1) {
	switch (errno) {
	case EINTR:
	    return EEINTR;
	case EADDRINUSE:
	    return EEADDRINUSE;
	case EALREADY:
	    return EEALREADY;
	case ECONNREFUSED:
	    return EECONNREFUSED;
	case EINPROGRESS:
	    break;
	default:
	    debug_perror("socket_connect: connect", 0);
	    return EECONNECT;
	}
    }
    lpc_socks[fd].state = DATA_XFER;
    lpc_socks[fd].flags |= S_BLOCKED;

    return EESUCCESS;
}

/*
 * Write a message on an LPC efun socket
 */
int
socket_write P3(int, fd, svalue_t *, message, char *, name)
{
    int len, off;
    char *buf, *p;
    struct sockaddr_in sin;

    if (fd < 0 || fd >= MAX_EFUN_SOCKS)
	return EEFDRANGE;
    if (lpc_socks[fd].state == CLOSED)
	return EEBADF;
    if (lpc_socks[fd].owner_ob != current_object)
	return EESECURITY;
    if (lpc_socks[fd].mode == DATAGRAM) {
	if (name == NULL)
	    return EENOADDR;
	if (!socket_name_to_sin(name, &sin))
	    return EEBADADDR;
    } else {
	if (lpc_socks[fd].state != DATA_XFER)
	    return EENOTCONN;
	if (name != NULL)
	    return EEBADADDR;
	if (lpc_socks[fd].flags & S_BLOCKED)
	    return EEALREADY;
    }

    switch (lpc_socks[fd].mode) {

    case MUD:
	switch (message->type) {

	case T_OBJECT:
	    return EETYPENOTSUPP;

	default:
	    save_svalue_depth = 0;
	    len = svalue_save_size(message);
	    if (save_svalue_depth > MAX_SAVE_SVALUE_DEPTH) {
		return EEBADDATA;
	    }
	    buf = (char *)
		DMALLOC(len + 5, TAG_TEMPORARY, "socket_write: default");
	    if (buf == NULL)
		fatal("Out of memory");
	    *(INT_32 *) buf = htonl((long) len);
	    len += 4;
	    buf[4] = '\0';
	    p = buf + 4;
	    save_svalue(message, &p);
	    break;
	}
	break;

    case STREAM:
	switch (message->type) {
	case T_BUFFER:
	    len = message->u.buf->size;
	    buf = (char *) DMALLOC(len, TAG_TEMPORARY, "socket_write: T_BUFFER");
	    if (buf == NULL)
		fatal("Out of memory");
	    memcpy(buf, message->u.buf->item, len);
	    break;
	case T_STRING:
	    len = strlen(message->u.string);
	    buf = (char *) DMALLOC(len + 1, TAG_TEMPORARY, "socket_write: T_STRING");
	    if (buf == NULL)
		fatal("Out of memory");
	    strcpy(buf, message->u.string);
	    break;
	case T_ARRAY:
	    {
		int i, limit;
		svalue_t *el;

		len = message->u.arr->size * sizeof(int);
		buf = (char *) DMALLOC(len + 1, TAG_TEMPORARY, "socket_write: T_ARRAY");
		if (buf == NULL)
		    fatal("Out of memory");
		el = message->u.arr->item;
		limit = len / sizeof(int);
		for (i = 0; i < limit; i++) {
		    switch (el[i].type) {
		    case T_NUMBER:
			memcpy((char *) &buf[i * sizeof(int)],
			       (char *) &el[i].u.number, sizeof(int));
			break;
		    case T_REAL:
			memcpy((char *) &buf[i * sizeof(int)], (char *) &el[i].u.real,
			       sizeof(int));
			break;
		    default:
			break;
		    }
		}
		break;
	    }
	default:
	    return EETYPENOTSUPP;
	}
	break;

    case DATAGRAM:
	switch (message->type) {

	case T_STRING:
	    if (sendto(lpc_socks[fd].fd, (char *)message->u.string,
		       strlen(message->u.string) + 1, 0,
		       (struct sockaddr *) & sin, sizeof(sin)) == -1) {
		debug_perror("socket_write: sendto", 0);
		return EESENDTO;
	    }
	    return EESUCCESS;


	case T_BUFFER:
	    if (sendto(lpc_socks[fd].fd, (char *)message->u.buf->item,
		       message->u.buf->size, 0,
		       (struct sockaddr *) & sin, sizeof(sin)) == -1) {
		debug_perror("socket_write: sendto", 0);
		return EESENDTO;
	    }
	    return EESUCCESS;

	default:
	    return EETYPENOTSUPP;
	}

    default:
	return EEMODENOTSUPP;
    }

    off = write(lpc_socks[fd].fd, buf, len);
    if (off == -1) {
	FREE(buf);
	switch (errno) {

	case EWOULDBLOCK:
	    return EEWOULDBLOCK;

	default:
	    debug_perror("socket_write: send", 0);
	    return EESEND;
	}
    }
    if (off < len) {
	lpc_socks[fd].flags |= S_BLOCKED;
	lpc_socks[fd].w_buf = buf;
	lpc_socks[fd].w_off = off;
	lpc_socks[fd].w_len = len - off;
	return EECALLBACK;
    }
    FREE(buf);

    return EESUCCESS;
}

static void
call_callback P3(int, fd, int, what, int, num_arg) {
    union string_or_func callback;

    switch (what) {
    case S_READ_FP: callback = lpc_socks[fd].read_callback; break;
    case S_WRITE_FP: callback = lpc_socks[fd].write_callback; break;
    case S_CLOSE_FP: callback = lpc_socks[fd].close_callback; break;
    }

    if (fd & what) {
	safe_call_function_pointer(callback.f, num_arg);
    } else if (callback.s) {
	safe_apply(callback.s, lpc_socks[fd].owner_ob, num_arg, ORIGIN_DRIVER);
    }
}

/*
 * Handle LPC efun socket read select events
 */
void
socket_read_select_handler P1(int, fd)
{
    int cc = 0, addrlen;
    char buf[BUF_SIZE], addr[ADDR_BUF_SIZE];
    svalue_t value;
    struct sockaddr_in sin;

    debug(8192, ("read_socket_handler: fd %d state %d\n",
		 fd, lpc_socks[fd].state));

    switch (lpc_socks[fd].state) {

    case CLOSED:
	return;
	
    case UNBOUND:
	debug_message("socket_read_select_handler: read on unbound socket %i\n");
	break;

    case BOUND:
	switch (lpc_socks[fd].mode) {

	case MUD:
	case STREAM:
	    break;

	case DATAGRAM:
	    debug(8192, ("read_socket_handler: DATA_XFER DATAGRAM\n"));
	    addrlen = sizeof(sin);
	    cc = recvfrom(lpc_socks[fd].fd, buf, sizeof(buf) - 1, 0,
			  (struct sockaddr *) & sin, &addrlen);
	    if (cc <= 0)
		break;
	    debug(8192, ("read_socket_handler: read %d bytes\n", cc));
	    buf[cc] = '\0';
	    sprintf(addr, "%s %d", inet_ntoa(sin.sin_addr),
		    (int)ntohs(sin.sin_port));
	    push_number(fd);
	    if (lpc_socks[fd].flags & S_BINARY) {
		buffer_t *b;

		b = allocate_buffer(cc);
		if (b) {
		    memcpy(b->item, buf, cc);
		    push_refed_buffer(b);
		} else {
		    push_number(0);
		}
	    } else {
		push_string(buf, STRING_MALLOC);
	    }
	    push_string(addr, STRING_MALLOC);
	    debug(8192, ("read_socket_handler: apply\n"));
	    call_callback(fd, S_READ_FP, 3);
	    return;
#ifdef DEBUG
	    /* shut up gcc */
	case STREAM_BINARY:
	case DATAGRAM_BINARY:
#endif
	    ;
	}
	break;

    case LISTEN:
	debug(8192, ("read_socket_handler: apply read callback\n"));
	lpc_socks[fd].flags |= S_WACCEPT;
	push_number(fd);
	call_callback(fd, S_READ_FP, 1);
	return;

    case DATA_XFER:
	switch (lpc_socks[fd].mode) {

	case DATAGRAM:
	    break;

	case MUD:
	    debug(8192, ("read_socket_handler: DATA_XFER MUD\n"));
	    if (lpc_socks[fd].flags & S_HEADER) {
		cc = recv(lpc_socks[fd].fd, (char *) &lpc_socks[fd].r_len +
			  lpc_socks[fd].r_off, 4 -
			  lpc_socks[fd].r_off, 0);
		if (cc <= 0)
		    break;
		debug(8192, ("read_socket_handler: read %d bytes\n", cc));
		lpc_socks[fd].r_off += cc;
		if (lpc_socks[fd].r_off != 4)
		    return;
		debug(8192, ("read_socket_handler: read header\n"));
		lpc_socks[fd].flags &= ~S_HEADER;
		lpc_socks[fd].r_off = 0;
		lpc_socks[fd].r_len = ntohl(lpc_socks[fd].r_len);
		if (lpc_socks[fd].r_len > MAX_BYTE_TRANSFER)
		    break;
		lpc_socks[fd].r_buf = (char *)
		    DMALLOC(lpc_socks[fd].r_len + 1, TAG_TEMPORARY, "socket_read_select_handler");
		if (lpc_socks[fd].r_buf == NULL)
		    fatal("Out of memory");
		debug(8192, ("read_socket_handler: svalue len is %d\n",
			     lpc_socks[fd].r_len));
	    }
	    if (lpc_socks[fd].r_off < lpc_socks[fd].r_len) {
		cc = recv(lpc_socks[fd].fd, lpc_socks[fd].r_buf +
			  lpc_socks[fd].r_off, lpc_socks[fd].r_len -
			  lpc_socks[fd].r_off, 0);
		if (cc <= 0)
		    break;
		debug(8192, ("read_socket_handler: read %d bytes\n", cc));
		lpc_socks[fd].r_off += cc;
		if (lpc_socks[fd].r_off != lpc_socks[fd].r_len)
		    return;
		debug(8192, ("read_socket_handler: read svalue\n"));
	    }
	    lpc_socks[fd].r_buf[lpc_socks[fd].r_len] = '\0';
	    value = const0;
	    push_number(fd);
	    if (restore_svalue(lpc_socks[fd].r_buf, &value) == 0)
		push_svalue(&value);
	    else
		push_null();
	    FREE(lpc_socks[fd].r_buf);
	    lpc_socks[fd].flags |= S_HEADER;
	    lpc_socks[fd].r_buf = NULL;
	    lpc_socks[fd].r_off = 0;
	    lpc_socks[fd].r_len = 0;
	    debug(8192, ("read_socket_handler: apply read callback\n"));
	    call_callback(fd, S_READ_FP, 2);
	    return;

	case STREAM:
	    debug(8192, ("read_socket_handler: DATA_XFER STREAM\n"));
	    cc = read(lpc_socks[fd].fd, buf, sizeof(buf) - 1);
	    if (cc <= 0)
		break;
	    debug(8192, ("read_socket_handler: read %d bytes\n", cc));
	    buf[cc] = '\0';
	    push_number(fd);
	    if (lpc_socks[fd].flags & S_BINARY) {
		buffer_t *b;

		b = allocate_buffer(cc);
		if (b) {
		    b->ref--;
		    memcpy(b->item, buf, cc);
		    push_buffer(b);
		} else {
		    push_number(0);
		}
	    } else {
		push_string(buf, STRING_MALLOC);
	    }
	    debug(8192, ("read_socket_handler: apply read callback\n"));
	    call_callback(fd, S_READ_FP, 2);
	    return;
#ifdef DEBUG
	    /* shut up gcc */
	case STREAM_BINARY:
	case DATAGRAM_BINARY:
#endif
	    ;
	}
	break;
    }
    if (cc == -1) {
	switch (errno) {
	case ECONNREFUSED:
	    /* Evidentally, on Linux 1.2.1, ECONNREFUSED gets returned
	     * if an ICMP_PORT_UNREACHED error happens internally.  Why
	     * they use this error message, I have no idea, but this seems
	     * to work.
	     */
	    if (lpc_socks[fd].state == BOUND
		&& lpc_socks[fd].mode == DATAGRAM)
		return;
	    break;
	case EINTR:
	case EWOULDBLOCK:
	    return;
	default:
	    break;
	}
    }
    socket_close(fd, SC_FORCE | SC_DO_CALLBACK);
}

/*
 * Handle LPC efun socket write select events
 */
void
socket_write_select_handler P1(int, fd)
{
    int cc;

    debug(8192, ("write_socket_handler: fd %d state %d\n",
		 fd, lpc_socks[fd].state));

    if ((lpc_socks[fd].flags & S_BLOCKED) == 0)
	return;

    if (lpc_socks[fd].w_buf != NULL) {
	cc = write(lpc_socks[fd].fd, lpc_socks[fd].w_buf + lpc_socks[fd].w_off,
		   lpc_socks[fd].w_len);
	if (cc == -1)
	    return;
	lpc_socks[fd].w_off += cc;
	lpc_socks[fd].w_len -= cc;
	if (lpc_socks[fd].w_len != 0)
	    return;
	FREE(lpc_socks[fd].w_buf);
	lpc_socks[fd].w_buf = NULL;
	lpc_socks[fd].w_off = 0;
    }
    lpc_socks[fd].flags &= ~S_BLOCKED;

    debug(8192, ("write_socket_handler: apply write_callback\n"));

    push_number(fd);
    call_callback(fd, S_WRITE_FP, 1);

}

/*
 * Close an LPC efun socket
 */
int
socket_close P2(int, fd, int, flags)
{
    if (fd < 0 || fd >= MAX_EFUN_SOCKS)
	return EEFDRANGE;
    if (lpc_socks[fd].state == CLOSED)
	return EEBADF;
    if (!(flags & SC_FORCE) && lpc_socks[fd].owner_ob != current_object)
	return EESECURITY;
    while (close(lpc_socks[fd].fd) == -1 && errno == EINTR)
	;	/* empty while */
    lpc_socks[fd].state = CLOSED;
    if (lpc_socks[fd].r_buf != NULL)
	FREE(lpc_socks[fd].r_buf);
    if (lpc_socks[fd].w_buf != NULL)
	FREE(lpc_socks[fd].w_buf);

    if (flags & SC_DO_CALLBACK) {
	debug(8192, ("read_socket_handler: apply close callback\n"));
	push_number(fd);
	call_callback(fd, S_CLOSE_FP, 1);
    }
    
    set_read_callback(fd, 0);
    set_write_callback(fd, 0);
    set_close_callback(fd, 0);

    debug(8192, ("socket_close: closed fd %d\n", fd));
    return EESUCCESS;
}

/*
 * Release an LPC efun socket to another object
 */
int
socket_release P3(int, fd, object_t *, ob, svalue_t *, callback)
{
    if (fd < 0 || fd >= MAX_EFUN_SOCKS)
	return EEFDRANGE;
    if (lpc_socks[fd].state == CLOSED)
	return EEBADF;
    if (lpc_socks[fd].owner_ob != current_object)
	return EESECURITY;
    if (lpc_socks[fd].flags & S_RELEASE)
	return EESOCKRLSD;

    lpc_socks[fd].flags |= S_RELEASE;
    lpc_socks[fd].release_ob = ob;

    push_number(fd);
    push_object(ob);

    if (callback->type == T_FUNCTION)
	safe_call_function_pointer(callback->u.fp, 2);
    else
	safe_apply(callback->u.string, ob, 2, ORIGIN_DRIVER);

    if ((lpc_socks[fd].flags & S_RELEASE) == 0)
	return EESUCCESS;

    lpc_socks[fd].flags &= ~S_RELEASE;
    lpc_socks[fd].release_ob = NULL;

    return EESOCKNOTRLSD;
}

/*
 * Aquire an LPC efun socket from another object
 */
int
socket_acquire P4(int, fd, svalue_t *, read_callback, svalue_t *, write_callback, svalue_t *, close_callback)
{
    if (fd < 0 || fd >= MAX_EFUN_SOCKS)
	return EEFDRANGE;
    if (lpc_socks[fd].state == CLOSED)
	return EEBADF;
    if ((lpc_socks[fd].flags & S_RELEASE) == 0)
	return EESOCKNOTRLSD;
    if (lpc_socks[fd].release_ob != current_object)
	return EESECURITY;

    lpc_socks[fd].flags &= ~S_RELEASE;
    lpc_socks[fd].owner_ob = current_object;
    lpc_socks[fd].release_ob = NULL;

    set_read_callback(fd, read_callback);
    set_write_callback(fd, write_callback);
    set_close_callback(fd, close_callback);

    return EESUCCESS;
}

/*
 * Return the string representation of a socket error
 */
char *
     socket_error P1(int, error)
{
    error = -(error + 1);
    if (error < 0 || error >= ERROR_STRINGS)
	return "socket_error: invalid error number";
    return error_strings[error];
}

/*
 * Return the remote address for an LPC efun socket
 */
int get_socket_address P3(int, fd, char *, addr, int *, port)
{
    if (fd < 0 || fd >= MAX_EFUN_SOCKS) {
	addr[0] = '\0';
	*port = 0;
	return EEFDRANGE;
    }
    *port = (int) ntohs(lpc_socks[fd].r_addr.sin_port);
    sprintf(addr, "%s", inet_ntoa(lpc_socks[fd].r_addr.sin_addr));
    return EESUCCESS;
}

/*
 * Return the current socket owner
 */
object_t *
       get_socket_owner P1(int, fd)
{
    if (fd < 0 || fd >= MAX_EFUN_SOCKS)
	return (object_t *) NULL;
    if (lpc_socks[fd].state == CLOSED)
	return (object_t *) NULL;
    return lpc_socks[fd].owner_ob;
}

/*
 * Initialize a T_OBJECT svalue
 */
void
assign_socket_owner P2(svalue_t *, sv, object_t *, ob)
{
    if (ob != NULL) {
	sv->type = T_OBJECT;
	sv->u.ob = ob;
	add_ref(ob, "assign_socket_owner");
    } else
	assign_svalue_no_free(sv, &const0n);
}

/*
 * Convert a string representation of an address to a sockaddr_in
 */
static int
socket_name_to_sin P2(char *, name, struct sockaddr_in *, sin)
{
    int port;
    char *cp, addr[ADDR_BUF_SIZE];

    strncpy(addr, name, ADDR_BUF_SIZE);
    addr[ADDR_BUF_SIZE - 1] = '\0';

    cp = strchr(addr, ' ');
    if (cp == NULL)
	return 0;

    *cp = '\0';
    port = atoi(cp + 1);

    sin->sin_family = AF_INET;
    sin->sin_port = htons((u_short) port);
    sin->sin_addr.s_addr = inet_addr(addr);

    return 1;
}

/*
 * Close any sockets owned by ob
 */
void
close_referencing_sockets P1(object_t *, ob)
{
    int i;

    for (i = 0; i < MAX_EFUN_SOCKS; i++)
	if (lpc_socks[i].owner_ob == ob && lpc_socks[i].state != CLOSED)
	    socket_close(i, SC_FORCE);
}

/*
 * Return the string representation of a sockaddr_in
 */
static char *
     inet_address P1(struct sockaddr_in *, sin)
{
    static char addr[23], port[7];

    if (ntohl(sin->sin_addr.s_addr) == INADDR_ANY)
	strcpy(addr, "*");
    else
	strcpy(addr, inet_ntoa(sin->sin_addr));
    strcat(addr, ".");
    if (ntohs(sin->sin_port) == 0)
	strcpy(port, "*");
    else
	sprintf(port, "%d", (int)ntohs(sin->sin_port));
    strcat(addr, port);

    return (addr);
}

/*
 * Dump the LPC efun socket array
 */
void dump_socket_status P1(outbuffer_t *, out)
{
    int i;

    outbuf_add(out, "Fd    State      Mode       Local Address          Remote Address\n");
    outbuf_add(out, "--  ---------  --------  ---------------------  ---------------------\n");

    for (i = 0; i < MAX_EFUN_SOCKS; i++) {
	outbuf_addv(out, "%2d  ", lpc_socks[i].fd);

	switch (lpc_socks[i].state) {
	case CLOSED:
	    outbuf_add(out, "  CLOSED ");
	    break;
	case UNBOUND:
	    outbuf_add(out, " UNBOUND ");
	    break;
	case BOUND:
	    outbuf_add(out, "  BOUND  ");
	    break;
	case LISTEN:
	    outbuf_add(out, " LISTEN  ");
	    break;
	case DATA_XFER:
	    outbuf_add(out, "DATA_XFER");
	    break;
	default:
	    outbuf_add(out, "    ??    ");
	    break;
	}
	outbuf_add(out, "  ");

	switch (lpc_socks[i].mode) {
	case MUD:
	    outbuf_add(out, "   MUD  ");
	    break;
	case STREAM:
	    outbuf_add(out, " STREAM ");
	    break;
	case DATAGRAM:
	    outbuf_add(out, "DATAGRAM");
	    break;
	default:
	    outbuf_add(out, "   ??   ");
	    break;
	}
	outbuf_add(out, "  ");

	outbuf_addv(out, "%-21s  ", inet_address(&lpc_socks[i].l_addr));
	outbuf_addv(out, "%-21s\n", inet_address(&lpc_socks[i].r_addr));
    }
}
#endif				/* SOCKET_EFUNS */
