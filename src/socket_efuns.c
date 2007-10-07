
/*
 * socket_efun.c -- socket efuns for MudOS.
 *    5-92 : Dwayne Fontenot (Jacques@TMI) : original coding.
 *   10-92 : Dave Richards (Cynosure) : less original coding.
 */

#include "std.h"
#include "socket_efuns.h"
#include "socket_err.h"
#include "include/socket_err.h"
#include "socket_ctrl.h"
#include "comm.h"
#include "file.h"
#include "master.h"

#if defined(PACKAGE_SOCKETS) || defined(PACKAGE_EXTERNAL)

/* flags for socket_close */
#define SC_FORCE        1
#define SC_DO_CALLBACK  2
#define SC_FINAL_CLOSE  4

lpc_socket_t *lpc_socks = 0;
int max_lpc_socks = 0;

#ifdef PACKAGE_SOCKETS
static int socket_name_to_sin (const char *, struct sockaddr_in *);
static char *inet_address (struct sockaddr_in *);
#endif

/*
 * check permission
 */
int check_valid_socket (const char * const what, int fd, object_t * owner,
                          const char * const addr, int port)
{
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
    push_constant_string(what);
    push_refed_array(info);

    mret = apply_master_ob(APPLY_VALID_SOCKET, 3);
    return MASTER_APPROVED(mret);
}

static void clear_socket (int which, int dofree)
{
    if (dofree) {
        set_read_callback(which, 0);
        set_write_callback(which, 0);
        set_close_callback(which, 0);
    }
    
    lpc_socks[which].fd = -1;
    lpc_socks[which].flags = 0;
    lpc_socks[which].mode = MUD;
    lpc_socks[which].state = STATE_CLOSED;
    memset((char *) &lpc_socks[which].l_addr, 0, sizeof(lpc_socks[which].l_addr));
    memset((char *) &lpc_socks[which].r_addr, 0, sizeof(lpc_socks[which].r_addr));
    lpc_socks[which].owner_ob = NULL;
    lpc_socks[which].release_ob = NULL;
    lpc_socks[which].read_callback.s = 0;
    lpc_socks[which].write_callback.s = 0;
    lpc_socks[which].close_callback.s = 0;
    lpc_socks[which].r_buf = NULL;
    lpc_socks[which].r_off = 0;
    lpc_socks[which].r_len = 0;
    lpc_socks[which].w_buf = NULL;
    lpc_socks[which].w_off = 0;
    lpc_socks[which].w_len = 0;
}

/*
 * Get more LPC sockets structures if we run out
 */
static int more_lpc_sockets()
{
    int i;

    max_lpc_socks += 10;
    
    if (!lpc_socks)
        lpc_socks = CALLOCATE(10, lpc_socket_t, TAG_SOCKETS, "more_lpc_sockets");
    else
        lpc_socks = RESIZE(lpc_socks, max_lpc_socks, lpc_socket_t, TAG_SOCKETS, "more_lpc_sockets");
    
    i = max_lpc_socks;
    while (--i >= max_lpc_socks - 10)
        clear_socket(i, 0);

    return max_lpc_socks - 10;
}

/*
 * Set the callbacks for a socket
 */
void set_read_callback (int which, svalue_t * cb)
{
    char *s;

    if (lpc_socks[which].flags & S_READ_FP) {
        free_funp(lpc_socks[which].read_callback.f);
        lpc_socks[which].flags &= ~S_READ_FP;
    } else if ((s = lpc_socks[which].read_callback.s))
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

void set_write_callback (int which, svalue_t * cb)
{
    char *s;

    if (lpc_socks[which].flags & S_WRITE_FP) {
        free_funp(lpc_socks[which].write_callback.f);
        lpc_socks[which].flags &= ~S_WRITE_FP;
    } else if ((s = lpc_socks[which].write_callback.s))
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

void set_close_callback (int which, svalue_t * cb)
{
    char *s;

    if (lpc_socks[which].flags & S_CLOSE_FP) {
        free_funp(lpc_socks[which].close_callback.f);
        lpc_socks[which].flags &= ~S_CLOSE_FP;
    } else if ((s = lpc_socks[which].close_callback.s))
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

#ifdef PACKAGE_SOCKETS

static void copy_close_callback (int to, int from)
{
    char *s;

    if (lpc_socks[to].flags & S_CLOSE_FP) {
        free_funp(lpc_socks[to].close_callback.f);
    } else if ((s = lpc_socks[to].close_callback.s))
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

#endif

int find_new_socket (void)
{
    int i;
    
    for (i = 0; i < max_lpc_socks; i++) {
        if (lpc_socks[i].state == STATE_CLOSED) 
            return i;
    }
    return more_lpc_sockets();
}

#ifdef PACKAGE_SOCKETS

/*
 * Create an LPC efun socket
 */
int socket_create (enum socket_mode mode, svalue_t * read_callback, svalue_t * close_callback)
{
    int type, i, fd, optval;
#ifndef NO_BUFFER_TYPE
    int binary = 0;

    if (mode == STREAM_BINARY) {
        binary = 1;
        mode = STREAM;
    } else if (mode == DATAGRAM_BINARY) {
        binary = 1;
        mode = DATAGRAM;
    }
#endif
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
        fd = socket(PF_INET, type, 0);
        if (fd == INVALID_SOCKET) {
            socket_perror("socket_create: socket", 0);
            return EESOCKET;
        }
        optval = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &optval,
                       sizeof(optval)) == -1) {
            socket_perror("socket_create: setsockopt", 0);
            OS_socket_close(fd);
            return EESETSOCKOPT;
        }
        if (set_socket_nonblocking(fd, 1) == -1) {
            socket_perror("socket_create: set_socket_nonblocking", 0);
            OS_socket_close(fd);
            return EENONBLOCK;
        }
        lpc_socks[i].fd = fd;
        lpc_socks[i].flags = S_HEADER;

        if (type == SOCK_DGRAM) close_callback = 0;
        set_read_callback(i, read_callback);
        set_write_callback(i, 0);
        set_close_callback(i, close_callback);

#ifndef NO_BUFFER_TYPE
        if (binary) {
            lpc_socks[i].flags |= S_BINARY;
        }
#endif
        lpc_socks[i].mode = mode;
        lpc_socks[i].state = STATE_UNBOUND;
        memset((char *) &lpc_socks[i].l_addr, 0, sizeof(lpc_socks[i].l_addr));
        memset((char *) &lpc_socks[i].r_addr, 0, sizeof(lpc_socks[i].r_addr));
        lpc_socks[i].owner_ob = current_object;
        lpc_socks[i].release_ob = NULL;
        lpc_socks[i].r_buf = NULL;
        lpc_socks[i].r_off = 0;
        lpc_socks[i].r_len = 0;
        lpc_socks[i].w_buf = NULL;
        lpc_socks[i].w_off = 0;
        lpc_socks[i].w_len = 0;

        current_object->flags |= O_EFUN_SOCKET;

        debug(sockets, ("socket_create: created socket %d mode %d fd %d\n",
                     i, mode, fd));
    }

    return i;
}

/*
 * Bind an address to an LPC efun socket
 */
int socket_bind (int fd, int port, const char * addr)
{
    int len;
    struct sockaddr_in sin;

    if (fd < 0 || fd >= max_lpc_socks)
        return EEFDRANGE;
    if (lpc_socks[fd].state == STATE_CLOSED ||
        lpc_socks[fd].state == STATE_FLUSHING)
        return EEBADF;
    if (lpc_socks[fd].owner_ob != current_object)
        return EESECURITY;
    if (lpc_socks[fd].state != STATE_UNBOUND)
        return EEISBOUND;

    sin.sin_family = AF_INET;

    if (!addr) {
        if (MUD_IP[0])
            sin.sin_addr.s_addr = inet_addr(MUD_IP);
        else
            sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons((u_short) port);
    } else {
        if (!socket_name_to_sin(addr, &sin))
            return EEBADADDR;
    }

    if (bind(lpc_socks[fd].fd, (struct sockaddr *) & sin, sizeof(sin)) == -1) {
        switch (socket_errno) {
        case EADDRINUSE:
            return EEADDRINUSE;
        default:
            socket_perror("socket_bind: bind", 0);
            return EEBIND;
        }
    }
    len = sizeof(sin);
    if (getsockname(lpc_socks[fd].fd, (struct sockaddr *) & lpc_socks[fd].l_addr, &len) == -1) {
        socket_perror("socket_bind: getsockname", 0);
        return EEGETSOCKNAME;
    }
    lpc_socks[fd].state = STATE_BOUND;

    debug(sockets, ("socket_bind: bound socket %d to %s.%d\n",
                 fd, inet_ntoa(lpc_socks[fd].l_addr.sin_addr),
                 ntohs(lpc_socks[fd].l_addr.sin_port)));

    return EESUCCESS;
}

/*
 * Listen for connections on an LPC efun socket
 */
int socket_listen (int fd, svalue_t * callback)
{
    if (fd < 0 || fd >= max_lpc_socks)
        return EEFDRANGE;
    if (lpc_socks[fd].state == STATE_CLOSED ||
        lpc_socks[fd].state == STATE_FLUSHING)
        return EEBADF;
    if (lpc_socks[fd].owner_ob != current_object)
        return EESECURITY;
    if (lpc_socks[fd].mode == DATAGRAM)
        return EEMODENOTSUPP;
    if (lpc_socks[fd].state == STATE_UNBOUND)
        return EENOADDR;
    if (lpc_socks[fd].state != STATE_BOUND)
        return EEISCONN;

    if (listen(lpc_socks[fd].fd, 5) == -1) {
        socket_perror("socket_listen: listen", 0);
        return EELISTEN;
    }
    lpc_socks[fd].state = STATE_LISTEN;
    set_read_callback(fd, callback);

    current_object->flags |= O_EFUN_SOCKET;

    debug(sockets, ("socket_listen: listen on socket %d\n", fd));

    return EESUCCESS;
}

/*
 * Accept a connection on an LPC efun socket
 */
int socket_accept (int fd, svalue_t * read_callback, svalue_t * write_callback)
{
    int len, accept_fd, i;
    struct sockaddr_in sin;

    if (fd < 0 || fd >= max_lpc_socks)
        return EEFDRANGE;
    if (lpc_socks[fd].state == STATE_CLOSED ||
        lpc_socks[fd].state == STATE_FLUSHING)
        return EEBADF;
    if (lpc_socks[fd].owner_ob != current_object)
        return EESECURITY;
    if (lpc_socks[fd].mode == DATAGRAM)
        return EEMODENOTSUPP;
    if (lpc_socks[fd].state != STATE_LISTEN)
        return EENOTLISTN;

    lpc_socks[fd].flags &= ~S_WACCEPT;

    len = sizeof(sin);
    accept_fd = accept(lpc_socks[fd].fd, (struct sockaddr *) & sin, (int *) &len);
    if (accept_fd == -1) {
        switch (socket_errno) {
#ifdef EWOULDBLOCK
        case EWOULDBLOCK:
            return EEWOULDBLOCK;
#endif
        case EINTR:
            return EEINTR;
        default:
            socket_perror("socket_accept: accept", 0);
            return EEACCEPT;
        }
    }

    /*
     * according to Amylaar, 'accepted' sockets in Linux 0.99p6 don't
     * properly inherit the nonblocking property from the listening socket.
     * Marius, 19-Jun-2000: this happens on other platforms as well, so just
     * do it for everyone
     */
    if (set_socket_nonblocking(accept_fd, 1) == -1) {
        socket_perror("socket_accept: set_socket_nonblocking 1", 0);
        OS_socket_close(accept_fd);
        return EENONBLOCK;
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
        lpc_socks[i].state = STATE_DATA_XFER;
        len = sizeof(sin);
        if (getsockname(lpc_socks[i].fd, (struct sockaddr *)&lpc_socks[i].l_addr, &len) == -1) {
            lpc_socks[i].l_addr = lpc_socks[fd].l_addr;
        }
        lpc_socks[i].r_addr = sin;
        lpc_socks[i].owner_ob = NULL;
        lpc_socks[i].release_ob = NULL;
        lpc_socks[i].r_buf = NULL;
        lpc_socks[i].r_off = 0;
        lpc_socks[i].r_len = 0;
        lpc_socks[i].w_buf = NULL;
        lpc_socks[i].w_off = 0;
        lpc_socks[i].w_len = 0;

        lpc_socks[i].owner_ob = current_object;
        set_read_callback(i, read_callback);
        set_write_callback(i, write_callback);
        copy_close_callback(i, fd);

        current_object->flags |= O_EFUN_SOCKET;

        debug(sockets, ("socket_accept: accept on socket %d\n", fd));
        debug(sockets, ("socket_accept: new socket %d on fd %d\n", i, accept_fd));
    } else
        OS_socket_close(accept_fd);

    return i;
}

/*
 * Connect an LPC efun socket
 */
int socket_connect (int fd, const char * name, svalue_t * read_callback, svalue_t * write_callback)
{
    if (fd < 0 || fd >= max_lpc_socks)
        return EEFDRANGE;
    if (lpc_socks[fd].state == STATE_CLOSED ||
        lpc_socks[fd].state == STATE_FLUSHING)
        return EEBADF;
    if (lpc_socks[fd].owner_ob != current_object)
        return EESECURITY;
    if (lpc_socks[fd].mode == DATAGRAM)
        return EEMODENOTSUPP;
    switch (lpc_socks[fd].state) {
    case STATE_CLOSED:
    case STATE_FLUSHING:
    case STATE_UNBOUND:
    case STATE_BOUND:
        break;
    case STATE_LISTEN:
        return EEISLISTEN;
    case STATE_DATA_XFER:
        return EEISCONN;
    }

    if (!socket_name_to_sin(name, &lpc_socks[fd].r_addr))
        return EEBADADDR;

    set_read_callback(fd, read_callback);
    set_write_callback(fd, write_callback);

    current_object->flags |= O_EFUN_SOCKET;

#ifdef WINSOCK
    /* Turn on blocking for connect to ensure correct errors */
    if (set_socket_nonblocking(lpc_socks[fd].fd, 0) == -1) {
        socket_perror("socket_connect: set_socket_nonblocking 0", 0);
        OS_socket_close(fd);
        return EENONBLOCK;
    }
#endif
    if (connect(lpc_socks[fd].fd, (struct sockaddr *) & lpc_socks[fd].r_addr,
                sizeof(struct sockaddr_in)) == -1) {
        switch (socket_errno) {
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
            socket_perror("socket_connect: connect", 0);
            return EECONNECT;
        }
    }
#ifdef WINSOCK
    if (set_socket_nonblocking(lpc_socks[fd].fd, 1) == -1) {
        socket_perror("socket_connect: set_socket_nonblocking 1", 0);
        OS_socket_close(fd);
        return EENONBLOCK;
    }
#endif
    lpc_socks[fd].state = STATE_DATA_XFER;
    lpc_socks[fd].flags |= S_BLOCKED;

    return EESUCCESS;
}

/*
 * Write a message on an LPC efun socket
 */
int socket_write (int fd, svalue_t * message, const char * name)
{
    int len, off;
    char *buf, *p;
    struct sockaddr_in sin;

    if (fd < 0 || fd >= max_lpc_socks)
        return EEFDRANGE;
    if (lpc_socks[fd].state == STATE_CLOSED ||
        lpc_socks[fd].state == STATE_FLUSHING)
        return EEBADF;
    if (lpc_socks[fd].owner_ob != current_object)
        return EESECURITY;
    if (lpc_socks[fd].mode == DATAGRAM) {
        if (name == NULL)
            return EENOADDR;
        if (!socket_name_to_sin(name, &sin))
            return EEBADADDR;
    } else {
        if (lpc_socks[fd].state != STATE_DATA_XFER)
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
#ifndef NO_BUFFER_TYPE
        case T_BUFFER:
            len = message->u.buf->size;
            buf = (char *) DMALLOC(len, TAG_TEMPORARY, "socket_write: T_BUFFER");
            if (buf == NULL)
                fatal("Out of memory");
            memcpy(buf, message->u.buf->item, len);
            break;
#endif
        case T_STRING:
            len = SVALUE_STRLEN(message);
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
            if ((off = sendto(lpc_socks[fd].fd, (char *)message->u.string,
                       strlen(message->u.string) + 1, 0,
                       (struct sockaddr *) & sin, sizeof(sin))) == -1) {
              //socket_perror("socket_write: sendto", 0);
                return EESENDTO;
            }
            break;

#ifndef NO_BUFFER_TYPE
        case T_BUFFER:
            if ((off = sendto(lpc_socks[fd].fd, (char *)message->u.buf->item,
                       message->u.buf->size, 0,
                       (struct sockaddr *) & sin, sizeof(sin))) == -1) {
              //socket_perror("socket_write: sendto", 0);
                return EESENDTO;
            }
            break;
#endif

        default:
            return EETYPENOTSUPP;
        }

#ifdef F_NETWORK_STATS
        if (!(lpc_socks[fd].flags & S_EXTERNAL)) {
            inet_out_packets++;
            inet_out_volume += off;
            inet_socket_out_packets++;
            inet_socket_out_volume += off;
        }
#endif
        return EESUCCESS;

    default:
        return EEMODENOTSUPP;
    }

    if (!len) {
        FREE(buf);
        return EESUCCESS;
    }
    off = OS_socket_write(lpc_socks[fd].fd, buf, len);
    if (off <= 0) {
        FREE(buf);

#ifdef EWOULDBLOCK
        if (off == -1 && socket_errno == EWOULDBLOCK)
            return EEWOULDBLOCK;
#endif
        if (off == -1 && socket_errno == EINTR)
            return EEINTR;
        
        //socket_perror("socket_write: send", 0);
        lpc_socks[fd].flags |= S_LINKDEAD;
        socket_close(fd, SC_FORCE | SC_DO_CALLBACK | SC_FINAL_CLOSE);
        return EESEND;
    }

#ifdef F_NETWORK_STATS
    if (!(lpc_socks[fd].flags & S_EXTERNAL)) {
        inet_out_packets++;
        inet_out_volume += off;
        inet_socket_out_packets++;
        inet_socket_out_volume += off;
    }
#endif

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

#endif  /* PACKAGE_SOCKETS */

static void call_callback (int fd, int what, int num_arg)
{
    union string_or_func callback;

    switch (what) {
    case S_READ_FP: callback = lpc_socks[fd].read_callback; break;
    case S_WRITE_FP: callback = lpc_socks[fd].write_callback; break;
    case S_CLOSE_FP: callback = lpc_socks[fd].close_callback; break;
    }

    if (lpc_socks[fd].flags & what) {
        safe_call_function_pointer(callback.f, num_arg);
    } else if (callback.s) {
        if (callback.s[0] == APPLY___INIT_SPECIAL_CHAR)
            error("Illegal function name.\n");
        safe_apply(callback.s, lpc_socks[fd].owner_ob, num_arg, ORIGIN_INTERNAL);
    }
}

/*
 * Handle LPC efun socket read select events
 */
void socket_read_select_handler (int fd)
{
    int cc = 0, addrlen;
    char buf[BUF_SIZE], addr[ADDR_BUF_SIZE];
    svalue_t value;
    struct sockaddr_in sin;

    debug(sockets, ("read_socket_handler: fd %d state %d\n",
                 fd, lpc_socks[fd].state));

    switch (lpc_socks[fd].state) {

    case STATE_CLOSED:
    case STATE_FLUSHING:
        return;
        
    case STATE_UNBOUND:
        debug_message("socket_read_select_handler: read on unbound socket %i\n", fd);
        break;

    case STATE_BOUND:
        switch (lpc_socks[fd].mode) {

        case MUD:
        case STREAM:
            break;

        case DATAGRAM:
            debug(sockets, ("read_socket_handler: DATA_XFER DATAGRAM\n"));
            addrlen = sizeof(sin);
            cc = recvfrom(lpc_socks[fd].fd, buf, sizeof(buf) - 1, 0,
                          (struct sockaddr *) & sin, &addrlen);
            if (cc <= 0)
                break;
#ifdef F_NETWORK_STATS
            if (!(lpc_socks[fd].flags & S_EXTERNAL)) {
                inet_in_packets++;
                inet_in_volume += cc;
                inet_socket_in_packets++;
                inet_socket_in_volume++;
            }
#endif
            debug(sockets, ("read_socket_handler: read %d bytes\n", cc));
            buf[cc] = '\0';
            sprintf(addr, "%s %d", inet_ntoa(sin.sin_addr),
                    ntohs(sin.sin_port));
            push_number(fd);
#ifndef NO_BUFFER_TYPE
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
#endif
                copy_and_push_string(buf);
#ifndef NO_BUFFER_TYPE
            }
#endif
            copy_and_push_string(addr);
            debug(sockets, ("read_socket_handler: apply\n"));
            call_callback(fd, S_READ_FP, 3);
            return;
        case STREAM_BINARY:
        case DATAGRAM_BINARY:
            ;
        }
        break;

    case STATE_LISTEN:
        debug(sockets, ("read_socket_handler: apply read callback\n"));
        lpc_socks[fd].flags |= S_WACCEPT;
        push_number(fd);
        call_callback(fd, S_READ_FP, 1);
        return;

    case STATE_DATA_XFER:
        switch (lpc_socks[fd].mode) {

        case DATAGRAM:
            break;

        case MUD:
            debug(sockets, ("read_socket_handler: DATA_XFER MUD\n"));
            if (lpc_socks[fd].flags & S_HEADER) {
                cc = OS_socket_read(lpc_socks[fd].fd, (char *) &lpc_socks[fd].r_len +
                                    lpc_socks[fd].r_off, 4 - lpc_socks[fd].r_off);
                if (cc <= 0)
                    break;
#ifdef F_NETWORK_STATS
                if (!(lpc_socks[fd].flags & S_EXTERNAL)) {
                    inet_in_packets++;
                    inet_in_volume += cc;
                    inet_socket_in_packets++;
                    inet_socket_in_volume += cc;
                }
#endif
                debug(sockets, ("read_socket_handler: read %d bytes\n", cc));
                lpc_socks[fd].r_off += cc;
                if (lpc_socks[fd].r_off != 4)
                    return;
                debug(sockets, ("read_socket_handler: read header\n"));
                lpc_socks[fd].flags &= ~S_HEADER;
                lpc_socks[fd].r_off = 0;
                lpc_socks[fd].r_len = ntohl(lpc_socks[fd].r_len);
                if (lpc_socks[fd].r_len <= 0 || lpc_socks[fd].r_len > MAX_BYTE_TRANSFER)
                    break;
                lpc_socks[fd].r_buf = (char *)
                    DMALLOC(lpc_socks[fd].r_len + 1, TAG_TEMPORARY, "socket_read_select_handler");
                if (lpc_socks[fd].r_buf == NULL)
                    fatal("Out of memory");
                debug(sockets, ("read_socket_handler: svalue len is %lu\n",
                             lpc_socks[fd].r_len));
            }
            if (lpc_socks[fd].r_off < lpc_socks[fd].r_len) {
                cc = OS_socket_read(lpc_socks[fd].fd, lpc_socks[fd].r_buf +
                                    lpc_socks[fd].r_off, lpc_socks[fd].r_len -
                                    lpc_socks[fd].r_off);
                if (cc <= 0)
                    break;
#ifdef F_NETWORK_STATS
                if (!(lpc_socks[fd].flags & S_EXTERNAL)) {
                    inet_in_packets++;
                    inet_in_volume += cc;
                    inet_socket_in_packets++;
                    inet_socket_in_volume += cc;
                }
#endif
                debug(sockets, ("read_socket_handler: read %d bytes\n", cc));
                lpc_socks[fd].r_off += cc;
                if (lpc_socks[fd].r_off != lpc_socks[fd].r_len)
                    return;
                debug(sockets, ("read_socket_handler: read svalue\n"));
            }
            lpc_socks[fd].r_buf[lpc_socks[fd].r_len] = '\0';
            value = const0;
            push_number(fd);
            if (restore_svalue(lpc_socks[fd].r_buf, &value) == 0) {
                STACK_INC;
                *sp = value;
            } else {
                push_undefined();
            }
            FREE(lpc_socks[fd].r_buf);
            lpc_socks[fd].flags |= S_HEADER;
            lpc_socks[fd].r_buf = NULL;
            lpc_socks[fd].r_off = 0;
            lpc_socks[fd].r_len = 0;
            debug(sockets, ("read_socket_handler: apply read callback\n"));
            call_callback(fd, S_READ_FP, 2);
            return;

        case STREAM:
            debug(sockets, ("read_socket_handler: DATA_XFER STREAM\n"));
            cc = OS_socket_read(lpc_socks[fd].fd, buf, sizeof(buf) - 1);
            if (cc <= 0)
                break;
#ifdef F_NETWORK_STATS
            if (!(lpc_socks[fd].flags & S_EXTERNAL)) {
                inet_in_packets++;
                inet_in_volume += cc;
                inet_socket_in_packets++;
                inet_socket_in_volume += cc;
            }
#endif
            debug(sockets, ("read_socket_handler: read %d bytes\n", cc));
            buf[cc] = '\0';
            push_number(fd);
#ifndef NO_BUFFER_TYPE
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
#endif
                copy_and_push_string(buf);
#ifndef NO_BUFFER_TYPE
            }
#endif
            debug(sockets, ("read_socket_handler: apply read callback\n"));
            call_callback(fd, S_READ_FP, 2);
            return;
        case STREAM_BINARY:
        case DATAGRAM_BINARY:
            ;
        }
        break;
    }
    if (cc == -1) {
        switch (socket_errno) {
        case ECONNREFUSED:
            /* Evidentally, on Linux 1.2.1, ECONNREFUSED gets returned
             * if an ICMP_PORT_UNREACHED error happens internally.  Why
             * they use this error message, I have no idea, but this seems
             * to work.
             */
            if (lpc_socks[fd].state == STATE_BOUND
                && lpc_socks[fd].mode == DATAGRAM)
                return;
            break;
        case EINTR:
#ifdef EWOULDBLOCK
        case EWOULDBLOCK:
            return;
#endif
        default:
            break;
        }
    }

    lpc_socks[fd].flags |= S_LINKDEAD;
    socket_close(fd, SC_FORCE | SC_DO_CALLBACK | SC_FINAL_CLOSE);
}

/*
 * Handle LPC efun socket write select events
 */
void socket_write_select_handler (int fd)
{
    int cc;

    debug(sockets, ("write_socket_handler: fd %d state %d\n",
                 fd, lpc_socks[fd].state));

    /* if the socket isn't blocked, we've got nothing to send */
    /* if the socket is linkdead, don't send -- could block */
    if (!(lpc_socks[fd].flags & S_BLOCKED) || lpc_socks[fd].flags & S_LINKDEAD)
        return;

    if (lpc_socks[fd].w_buf != NULL) {
        cc = OS_socket_write(lpc_socks[fd].fd, 
                             lpc_socks[fd].w_buf + lpc_socks[fd].w_off,
                             lpc_socks[fd].w_len);
        if (cc <= -1) {
            if (cc == -1 && (
#ifdef EWOULDBLOCK
                        errno == EWOULDBLOCK ||
#endif
                        errno == EINTR)) {
                return;
            }

            lpc_socks[fd].flags |= S_LINKDEAD;
            if (lpc_socks[fd].state == STATE_FLUSHING) {
                lpc_socks[fd].flags &= ~S_BLOCKED;
                socket_close(fd, SC_FORCE | SC_FINAL_CLOSE);
                return;
            }
            socket_close(fd, SC_FORCE | SC_DO_CALLBACK | SC_FINAL_CLOSE);
            return;
        }
#ifdef F_NETWORK_STATS
        if (!(lpc_socks[fd].flags & S_EXTERNAL)) {
            inet_out_packets++;
            inet_out_volume += cc;
            inet_socket_out_packets++;
            inet_socket_out_volume += cc;
        }
#endif
        lpc_socks[fd].w_off += cc;
        lpc_socks[fd].w_len -= cc;
        if (lpc_socks[fd].w_len != 0)
            return;
        FREE(lpc_socks[fd].w_buf);
        lpc_socks[fd].w_buf = NULL;
        lpc_socks[fd].w_off = 0;
    }
    lpc_socks[fd].flags &= ~S_BLOCKED;
    if (lpc_socks[fd].state == STATE_FLUSHING) {
        socket_close(fd, SC_FORCE | SC_FINAL_CLOSE);
        return;
    }

    debug(sockets, ("write_socket_handler: apply write_callback\n"));

    push_number(fd);
    call_callback(fd, S_WRITE_FP, 1);
}

/*
 * Close an LPC efun socket
 */
int socket_close (int fd, int flags)
{
    if (fd < 0 || fd >= max_lpc_socks)
        return EEFDRANGE;
    if (lpc_socks[fd].state == STATE_CLOSED)
        return EEBADF;
    if (lpc_socks[fd].state == STATE_FLUSHING && !(flags & SC_FINAL_CLOSE))
        return EEBADF;
    if (!(flags & SC_FORCE) && lpc_socks[fd].owner_ob != current_object)
        return EESECURITY;

    if (flags & SC_DO_CALLBACK) {
        debug(sockets, ("read_socket_handler: apply close callback\n"));
        push_number(fd);
        call_callback(fd, S_CLOSE_FP, 1);
    }
    
    set_read_callback(fd, 0);
    set_write_callback(fd, 0);
    set_close_callback(fd, 0);

    /* if we're linkdead, we'll never flush, so don't even try :-) */
    if ((lpc_socks[fd].flags & S_BLOCKED) && !(lpc_socks[fd].flags & S_LINKDEAD)) {
        /* Can't close now; we still have data to write.  Tell the mudlib
         * it is closed, but we really finish up later.
         */
        lpc_socks[fd].state = STATE_FLUSHING;
        return EESUCCESS;
    }
    
    while (OS_socket_close(lpc_socks[fd].fd) == -1 && socket_errno == EINTR)
        ;       /* empty while */
    if (lpc_socks[fd].r_buf != NULL)
        FREE(lpc_socks[fd].r_buf);
    if (lpc_socks[fd].w_buf != NULL)
        FREE(lpc_socks[fd].w_buf);
    clear_socket(fd, 1);
    
    debug(sockets, ("socket_close: closed fd %d\n", fd));
    return EESUCCESS;
}

#ifdef PACKAGE_SOCKETS

/*
 * Release an LPC efun socket to another object
 */
int socket_release (int fd, object_t * ob, svalue_t * callback)
{
    if (fd < 0 || fd >= max_lpc_socks)
        return EEFDRANGE;
    if (lpc_socks[fd].state == STATE_CLOSED ||
        lpc_socks[fd].state == STATE_FLUSHING)
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
        safe_apply(callback->u.string, ob, 2, ORIGIN_INTERNAL);

    if ((lpc_socks[fd].flags & S_RELEASE) == 0)
        return EESUCCESS;

    lpc_socks[fd].flags &= ~S_RELEASE;
    lpc_socks[fd].release_ob = NULL;

    return EESOCKNOTRLSD;
}

/*
 * Aquire an LPC efun socket from another object
 */
int socket_acquire (int fd, svalue_t * read_callback, svalue_t * write_callback, svalue_t * close_callback)
{
    if (fd < 0 || fd >= max_lpc_socks)
        return EEFDRANGE;
    if (lpc_socks[fd].state == STATE_CLOSED ||
        lpc_socks[fd].state == STATE_FLUSHING)
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
const char *socket_error (int error)
{
    error = -(error + 1);
    if (error < 0 || error >= ERROR_STRINGS - 1)
        return "socket_error: invalid error number";
    return error_strings[error];
}

/*
 * Return the remote address for an LPC efun socket
 */
int get_socket_address (int fd, char * addr, int * port, int local)
{
    struct sockaddr_in *addr_in;

    if (fd < 0 || fd >= max_lpc_socks) {
        addr[0] = '\0';
        *port = 0;
        return EEFDRANGE;
    }
    addr_in = (local ? &lpc_socks[fd].l_addr : &lpc_socks[fd].r_addr);
    *port = ntohs(addr_in->sin_port);
    strcpy(addr, inet_ntoa(addr_in->sin_addr));
    return EESUCCESS;
}

/*
 * Return the current socket owner
 */
object_t *get_socket_owner (int fd)
{
    if (fd < 0 || fd >= max_lpc_socks)
        return (object_t *) NULL;
    if (lpc_socks[fd].state == STATE_CLOSED ||
        lpc_socks[fd].state == STATE_FLUSHING)
        return (object_t *) NULL;
    return lpc_socks[fd].owner_ob;
}

#endif  /* PACKAGE_SOCKETS */

/*
 * Initialize a T_OBJECT svalue
 */
void assign_socket_owner (svalue_t * sv, object_t * ob)
{
    if (ob != NULL) {
        sv->type = T_OBJECT;
        sv->u.ob = ob;
        add_ref(ob, "assign_socket_owner");
    } else
        assign_svalue_no_free(sv, &const0u);
}

#ifdef PACKAGE_SOCKETS

/*
 * Convert a string representation of an address to a sockaddr_in
 */
static int socket_name_to_sin (const char * name, struct sockaddr_in * sin)
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

#endif  /* PACKAGE_SOCKETS */

/*
 * Close any sockets owned by ob
 */
void close_referencing_sockets (object_t * ob)
{
    int i;

    for (i = 0; i < max_lpc_socks; i++)
        if (lpc_socks[i].owner_ob == ob && 
            lpc_socks[i].state != STATE_CLOSED &&
            lpc_socks[i].state != STATE_FLUSHING)
            socket_close(i, SC_FORCE);
}

#ifdef PACKAGE_SOCKETS

/*
 * Return the string representation of a sockaddr_in
 */
static char *inet_address (struct sockaddr_in * sin)
{
    static char addr[32], port[7];

    if (ntohl(sin->sin_addr.s_addr) == INADDR_ANY)
        strcpy(addr, "*");
    else
        strcpy(addr, inet_ntoa(sin->sin_addr));
    strcat(addr, ".");
    if (ntohs(sin->sin_port) == 0)
        strcpy(port, "*");
    else
        sprintf(port, "%d", ntohs(sin->sin_port));
    strcat(addr, port);

    return (addr);
}

const char *socket_modes[] = {
    "MUD",
    "STREAM",
    "DATAGRAM",
    "STREAM_BINARY",
    "DATAGRAM_BINARY"
};

const char *socket_states[] = {
    "CLOSED",
    "CLOSING",
    "UNBOUND",
    "BOUND",
    "LISTEN",
    "DATA_XFER"
};

/*
 * Return an array containing info for a socket
 */
array_t *socket_status (int which)
{
    array_t *ret;
    
    if (which < 0 || which >= max_lpc_socks) return 0;

    ret = allocate_empty_array(6);
    
    ret->item[0].type = T_NUMBER;
    ret->item[0].subtype = 0;
    ret->item[0].u.number = lpc_socks[which].fd;
    
    ret->item[1].type = T_STRING;
    ret->item[1].subtype = STRING_CONSTANT;
    ret->item[1].u.string = socket_states[lpc_socks[which].state];

    ret->item[2].type = T_STRING;
    ret->item[2].subtype = STRING_CONSTANT;
    ret->item[2].u.string = socket_modes[lpc_socks[which].mode];

    ret->item[3].type = T_STRING;
    ret->item[3].subtype = STRING_MALLOC;
    ret->item[3].u.string = string_copy(inet_address(&lpc_socks[which].l_addr),
                                        "socket_status");
    
    ret->item[4].type = T_STRING;
    ret->item[4].subtype = STRING_MALLOC;
    ret->item[4].u.string = string_copy(inet_address(&lpc_socks[which].r_addr),
                                        "socket_status");

    if (lpc_socks[which].owner_ob && !(lpc_socks[which].owner_ob->flags & O_DESTRUCTED)) {
        ret->item[5].type = T_OBJECT;
        ret->item[5].u.ob = lpc_socks[which].owner_ob;
        add_ref(lpc_socks[which].owner_ob, "socket_status");
    } else {
        ret->item[5] = const0u;
    }
    
    return ret;
}

#endif  /* PACKAGE_SOCKETS */

#endif                          /* SOCKET_EFUNS */
