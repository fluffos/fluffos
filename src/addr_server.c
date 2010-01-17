/*
 *  addr_server.c -- socket-based ip address server.
 *                   8-92 : Dwayne Fontenot : original coding
 */

#include "std.h"
#include "addr_server.h"
#include "socket_ctrl.h"
#include "file_incl.h"
#include "port.h"

#ifdef MINGW
#include <ws2tcpip.h>
#endif

#ifdef DEBUG_MACRO
int debug_level = DBG_addr_server;
#endif				/* DEBUG_MACRO */

#define DBG(x) debug(addr_server, x)

/*
 * private local variables.
 */
static connection all_conns[MAX_CONNS];
static int total_conns = 0;
static queue_element_ptr queue_head = NULL;
static queue_element_ptr queue_tail = NULL;
static queue_element_ptr stack_head = NULL;
static int queue_length = 0;
static int conn_fd;

fd_set readmask;

int name_by_ip (int, char *);
int ip_by_name (int, char *);
INLINE_STATIC void process_queue (void);
void init_conns (void);
void init_conn_sock (int, char *);

#ifdef SIGNAL_FUNC_TAKES_INT
void sigpipe_handler (int);
#else
void sigpipe_handler (void);
#endif

INLINE void aserv_process_io (int);
void enqueue_datapending (int, int);
void handle_top_event (void);
void dequeue_top_event (void);
void pop_queue_element (queue_element_ptr *);
void push_queue_element (queue_element_ptr);
void new_conn_handler (void);
void conn_data_handler (int);
int index_by_fd (int);
void terminate (int);

void debug_perror (const char *, const char *);

void debug_perror (const char * what, const char * file) {
    if (file)
	fprintf(stderr, "System Error: %s:%s:%s\n", what, file, port_strerror(errno));
    else
	fprintf(stderr, "System Error: %s:%s\n", what, port_strerror(errno));
}

void init_conns()
{
    int i;

    for (i = 0; i < MAX_CONNS; i++) {
	all_conns[i].fd = -1;
	all_conns[i].state = CONN_CLOSED;
	all_conns[i].sname[0] = '\0';

	/* ensure 'leftover' buffer is _always_ null terminated */
	all_conns[i].buf[0] = '\0';
	all_conns[i].buf[IN_BUF_SIZE - 1] = '\0';
	all_conns[i].leftover = 0;
    }
}

/*
 * Initialize connection socket.
 */
void init_conn_sock (int port_num, char * ipaddress)
{
#ifdef IPV6
	struct sockaddr_in6 sin;
#else
    struct sockaddr_in sin;
#endif
    socklen_t sin_len;
    int optval;
#ifdef WINSOCK
    WSADATA WSAData;

    WSAStartup(MAKEWORD(1,1), &WSAData);
    atexit(cleanup_sockets);
#endif

    /*
     * create socket of proper type.
     */
#ifdef IPV6
    if ((conn_fd = socket(AF_INET6, SOCK_STREAM, 0)) == INVALID_SOCKET) {
#else
    if ((conn_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
#endif
	socket_perror("init_conn_sock: socket", 0);
	exit(1);
    }
    /*
     * enable local address reuse.
     */
    optval = 1;
    if (setsockopt(conn_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &optval,
		   sizeof(optval)) == -1) {
	socket_perror("init_conn_sock: setsockopt", 0);
	exit(2);
    }
    /*
     * fill in socket address information.
     */
#ifdef IPV6
    sin.sin6_family = AF_INET6;
    if(ipaddress)
       	inet_pton(AF_INET6, ipaddress, &(sin.sin6_addr));
    else
       	sin.sin6_addr = in6addr_any;
    sin.sin6_port = htons((u_short) port_num);
#else
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = (ipaddress ? inet_addr(ipaddress) : INADDR_ANY);
    sin.sin_port = htons((u_short) port_num);
#endif
    /*
     * bind name to socket.
     */
    if (bind(conn_fd, (struct sockaddr *) & sin, sizeof(sin)) == -1) {
	socket_perror("init_conn_sock: bind", 0);
	exit(3);
    }
    /*
     * get socket name.
     */
    sin_len = sizeof(sin);
    if (getsockname(conn_fd, (struct sockaddr *) & sin, &sin_len) == -1) {
	socket_perror("init_conn_sock: getsockname", 0);
	exit(4);
    }
    /*
     * register signal handler for SIGPIPE.
     */
#if defined(SIGPIPE) && defined(SIGNAL_ERROR)/* windows has no SIGPIPE */
    if (signal(SIGPIPE, sigpipe_handler) == SIGNAL_ERROR) {
	socket_perror("init_conn_sock: signal SIGPIPE", 0);
	exit(5);
    }
#endif
    /*
     * set socket non-blocking
     */
    if (set_socket_nonblocking(conn_fd, 1) == -1) {
	socket_perror("init_conn_sock: set_socket_nonblocking 1", 0);
	exit(8);
    }
    /*
     * listen on socket for connections.
     */
    if (listen(conn_fd, 128) == -1) {
	socket_perror("init_conn_sock: listen", 0);
	exit(10);
    }
    DBG(("listening for connections on port %d", port_num));
}

/*
 * SIGPIPE handler -- does very little for now.
 */
#ifdef SIGNAL_FUNC_TAKES_INT
void sigpipe_handler (int sig)
{
#else
void sigpipe_handler()
{
#endif
    fprintf(stderr, "SIGPIPE received.\n");
}

/*
 * I/O handler.
 */
INLINE void aserv_process_io (int nb)
{
    int i;

    switch (nb) {
    case -1:
	debug_perror("sigio_handler: select", 0);
	break;
    case 0:
	break;
    default:
	/*
	 * check for new connection.
	 */
	if (FD_ISSET(conn_fd, &readmask)) {
	    DBG(("sigio_handler: NEW_CONN"));
	    enqueue_datapending(conn_fd, NEW_CONN);
	}
	/*
	 * check for data pending on established connections.
	 */
	for (i = 0; i < MAX_CONNS; i++) {
	    if (FD_ISSET(all_conns[i].fd, &readmask)) {
		DBG(("sigio_handler: CONN"));
		enqueue_datapending(all_conns[i].fd, CONN);
	    }
	}
	break;
    }
}

INLINE_STATIC void process_queue()
{
    int i;

    for (i = 0; queue_head && (i < MAX_EVENTS_TO_PROCESS); i++) {
	handle_top_event();
	dequeue_top_event();
    }
}

void enqueue_datapending (int fd, int fd_type)
{
    queue_element_ptr new_queue_element;

    pop_queue_element(&new_queue_element);
    new_queue_element->event_type = fd_type;
    new_queue_element->fd = fd;
    new_queue_element->next = NULL;
    if (queue_head) {
	queue_tail->next = new_queue_element;
    } else {
	queue_head = new_queue_element;
    }
    queue_tail = new_queue_element;
}

void dequeue_top_event()
{
    queue_element_ptr top_queue_element;

    if (queue_head) {
	top_queue_element = queue_head;
	queue_head = queue_head->next;
	push_queue_element(top_queue_element);
    } else {
	fprintf(stderr, "dequeue_top_event: tried to dequeue from empty queue!\n");
    }
}

void pop_queue_element (queue_element_ptr * the_queue_element)
{
    if ((*the_queue_element = stack_head))
	stack_head = stack_head->next;
    else
	*the_queue_element = (queue_element_ptr) malloc(sizeof(queue_element));
    queue_length++;
}

void push_queue_element (queue_element_ptr the_queue_element)
{
    the_queue_element->next = stack_head;
    stack_head = the_queue_element;
    queue_length--;
}

void handle_top_event()
{
    switch (queue_head->event_type) {
	case NEW_CONN:
	DBG(("handle_top_event: NEW_CONN"));
	new_conn_handler();
	break;
    case CONN:
	DBG(("handle_top_event: CONN data on fd %d", queue_head->fd));
	conn_data_handler(queue_head->fd);
	break;
    default:
	fprintf(stderr, "handle_top_event: unknown event type %d\n",
		queue_head->event_type);
	break;
    }
}

/*
 * This is the new connection handler. This function is called by the
 * event handler when data is pending on the listening socket (conn_fd).
 * If space is available, an interactive data structure is initialized and
 * the connected is established.
 */
void new_conn_handler()
{

#ifdef IPV6
	struct sockaddr_in6 client;
#else
    struct sockaddr_in client;
#endif
    socklen_t client_len;
    struct hostent *c_hostent;
    int new_fd;
    int conn_index;

    client_len = sizeof(client);
    new_fd = accept(conn_fd, (struct sockaddr *) & client, &client_len);
    if (new_fd == -1) {
	socket_perror("new_conn_handler: accept", 0);
	return;
    }
    if (set_socket_nonblocking(new_fd, 1) == -1) {
	socket_perror("new_conn_handler: set_socket_nonblocking 1", 0);
	OS_socket_close(new_fd);
	return;
    }
    if (total_conns >= MAX_CONNS) {
	char *message = "no available slots -- closing connection.\n";

	fprintf(stderr, "new_conn_handler: no available connection slots.\n");
	OS_socket_write(new_fd, message, strlen(message));
	if (OS_socket_close(new_fd) == -1)
	    socket_perror("new_conn_handler: close", 0);
	return;
    }
    /* get some information about new connection */
    for (conn_index = 0; conn_index < MAX_CONNS; conn_index++) {
	if (all_conns[conn_index].state == CONN_CLOSED) {
	    DBG(("new_conn_handler: opening conn index %d", conn_index));
	    /* update global data for new fd */
	    all_conns[conn_index].fd = new_fd;
	    all_conns[conn_index].state = CONN_OPEN;
	    all_conns[conn_index].addr = client;

	    char portname[256];
	    if(getnameinfo((struct sockaddr *)&client, sizeof(client), all_conns[conn_index].sname, SNAME_LEN, portname, 255, NI_NAMEREQD|NI_NUMERICHOST))
	    	strcpy(all_conns[conn_index].sname, "<unknown>");
	    total_conns++;
	    return;
	}
    }
    fprintf(stderr, "new_conn_handler: sanity check failed!\n");
}

void conn_data_handler (int fd)
{
    int conn_index;
    int buf_index;
    int num_bytes;
    int msgtype;
    int leftover;
    char *buf;
    int res, msglen, expecting;
    long unread_bytes;

    if ((conn_index = index_by_fd(fd)) == -1) {
	fprintf(stderr, "conn_data_handler: invalid fd.\n");
	return;
    }
    DBG(("conn_data_handler: read on fd %d", fd));

    /* append new data to end of leftover data (if any) */
    leftover = all_conns[conn_index].leftover;
    buf = (char *) &all_conns[conn_index].buf[0];
    num_bytes = OS_socket_read(fd, buf + leftover, (IN_BUF_SIZE - 1) - leftover);

    switch (num_bytes) {
    case -1:
	switch (socket_errno) {
	case EWOULDBLOCK:
	    DBG(("conn_data_handler: read on fd %d: Operation would block.",
			fd));
	    break;
	default:
	    socket_perror("conn_data_handler: read", 0);
	    terminate(conn_index);
	    break;
	}
	break;
    case 0:
	if (all_conns[conn_index].state == CONN_CLOSED)
	    fprintf(stderr, "get_user_data: tried to read from closed fd.\n");
	terminate(conn_index);
	break;
    default:
	DBG(("conn_data_handler: read %d bytes on fd %d", num_bytes, fd));
	num_bytes += leftover;
	buf_index = 0;
	expecting = 0;
	while (num_bytes > sizeof(int) && buf_index < (IN_BUF_SIZE - 1)) {
	    /* get message type */
	    memcpy((char *) &msgtype, (char *) &buf[buf_index], sizeof(int));
	    DBG(("conn_data_handler: message type: %d", msgtype));

	    if (msgtype == NAMEBYIP) {
		if (buf[buf_index + sizeof(int)] == '\0') {
		    /* no data here...resync */
		    buf_index++;
		    num_bytes--;
		    continue;
		}
		if (expecting && num_bytes < expecting) {
		    /*
		     * message truncated...back up to DATALEN message; exit
		     * loop...and try again later
		     */
		    buf_index -= (sizeof(int) + sizeof(int));
		    num_bytes += (sizeof(int) + sizeof(int));
		    break;
		}
		res = name_by_ip(conn_index, &buf[buf_index]);
	    } else if (msgtype == IPBYNAME) {
		if (buf[buf_index + sizeof(int)] == '\0') {
		    /* no data here...resync */
		    buf_index++;
		    num_bytes--;
		    continue;
		}
		if (expecting && num_bytes < expecting) {
		    /*
		     * message truncated...back up to DATALEN message; exit
		     * loop...and try again later
		     */
		    buf_index -= (sizeof(int) + sizeof(int));
		    num_bytes += (sizeof(int) + sizeof(int));
		    break;
		}
		res = ip_by_name(conn_index, &buf[buf_index]);
	    } else if (msgtype == DATALEN) {
		if (num_bytes > (sizeof(int) + sizeof(int))) {
		    memcpy((char *) &expecting, (char *) &buf[buf_index + sizeof(int)], sizeof(int));
		    /*
		     * advance to next message
		     */
		    buf_index += (sizeof(int) + sizeof(int));
		    num_bytes -= (sizeof(int) + sizeof(int));
		    if (expecting > IN_BUF_SIZE || expecting <= 0) {
			fprintf(stderr, "conn_data_handler: bad data length %d\n", expecting);
			expecting = 0;
		    }
		    continue;
		} else {
		    /*
		     * not enough bytes...assume truncated; exit loop...we'll
		     * handle this message later
		     */
		    break;
		}
	    } else {
		fprintf(stderr, "conn_data_handler: unknown message type %08x\n", msgtype);
		/* advance through buffer */
		buf_index++;
		num_bytes--;
		continue;
	    }

	    msglen = (int) (sizeof(int) + strlen(&buf[buf_index + sizeof(int)]) +1);
	    if (res) {
		/*
		 * ok...advance to next message
		 */
		buf_index += msglen;
		num_bytes -= msglen;
	    } else if (msglen < num_bytes || (expecting && expecting == msglen)) {
		/*
		 * failed...
		 */

		/*
		 * this was a complete message...advance to the next one
		 */
		msglen = (int) (sizeof(int) + strlen(&buf[buf_index + sizeof(int)]) +1);
		buf_index += msglen;
		num_bytes -= msglen;
		expecting = 0;
	    }
	    else if (!OS_socket_ioctl(fd, FIONREAD, &unread_bytes) &&
		     unread_bytes > 0) {
		/*
		 * msglen == num_bytes could be a complete message... if
		 * there's unread data we'll assume it was truncated
		 */
		break;
	    } else {
		/*
		 * nothing more?  then discard message (it was the last one)
		 */
		buf_index = 0;
		num_bytes = 0;
		break;
	    }
	}

	/* keep track of leftover buffer contents */
	if (num_bytes && buf_index)
	    memmove(buf, &buf[buf_index], num_bytes);
	buf[num_bytes] = '\0';
	all_conns[conn_index].leftover = num_bytes;

	break;
    }
}

#define OUT_BUF_SIZE 80

int ip_by_name (int conn_index, char * buf)
{
    struct hostent *hp;
    struct in_addr my_in_addr;
    static char out_buf[OUT_BUF_SIZE];

    hp = gethostbyname(&buf[sizeof(int)]);
    if (hp == NULL) {
/* Failed :( */
	sprintf(out_buf, "%s 0\n", &buf[sizeof(int)]);
	DBG(("%s", out_buf));
	OS_socket_write(all_conns[conn_index].fd, out_buf, strlen(out_buf));
	return 0;
    } else {
/* Success! */
	memcpy(&my_in_addr, hp->h_addr, sizeof(struct in_addr));
	sprintf(out_buf, "%s %s\n", &buf[sizeof(int)],
		inet_ntoa(my_in_addr));
	DBG(("%s", out_buf));
	OS_socket_write(all_conns[conn_index].fd, out_buf, strlen(out_buf));
	return 1;
    }
}				/* ip_by_name() */

int name_by_ip (int conn_index, char * buf)
{
    struct addrinfo hints, *res;
    int ret;

    hints.ai_family = AF_INET6;
    hints.ai_socktype = 0;
    hints.ai_protocol = 0;
#if defined(AI_V4MAPPED)
    hints.ai_flags = AI_CANONNAME| AI_V4MAPPED;
#else
    hints.ai_flags = AI_CANONNAME;
#endif
    static char out_buf[OUT_BUF_SIZE];

    if((ret = getaddrinfo(&buf[sizeof(int)], NULL, &hints, &res))){
    	//failed
    	sprintf(out_buf, "%s 0\n", &buf[sizeof(int)]);
    	DBG(("name_by_ip: malformed address request (%d).", ret));
    	OS_socket_write(all_conns[conn_index].fd, out_buf, strlen(out_buf));
    	return 0;
    }
    char tmpbuf[80], tmpp[80];
    if(ret = getnameinfo(res->ai_addr, res->ai_addrlen, tmpbuf, 79, tmpp, 79, NI_NAMEREQD|NI_NUMERICSERV)){
    	sprintf(out_buf, "%s 0\n", &buf[sizeof(int)]);
    	DBG(("%s", out_buf));
    	OS_socket_write(all_conns[conn_index].fd, out_buf, strlen(out_buf));
    	DBG(("name_by_ip: unable to resolve address."));
    	freeaddrinfo(res);
    	return 0;
    }
    sprintf(out_buf, "%s %s\n", &buf[sizeof(int)], tmpbuf);
    DBG(("%s", out_buf));
    OS_socket_write(all_conns[conn_index].fd, out_buf, strlen(out_buf));
    freeaddrinfo(res);
	return 1;
}

int index_by_fd (int fd)
{
    int i;

    for (i = 0; i < MAX_CONNS; i++) {
	if ((all_conns[i].state == CONN_OPEN) && (all_conns[i].fd == fd))
	    return (i);
    }
    return (-1);
}

void terminate (int conn_index)
{
    if (conn_index < 0 || conn_index >= MAX_CONNS) {
	fprintf(stderr, "terminate: conn_index %d out of range.\n", conn_index);
	return;
    }
    if (all_conns[conn_index].state == CONN_CLOSED) {
	fprintf(stderr, "terminate: connection %d already closed.\n", conn_index);
	return;
    }
    DBG(("terminating connection %d", conn_index));

    if (OS_socket_close(all_conns[conn_index].fd) == -1) {
	socket_perror("terminate: close", 0);
	return;
    }
    all_conns[conn_index].state = CONN_CLOSED;
    total_conns--;
}

int main (int argc, char ** argv)
{
    int addr_server_port;
    struct timeval timeout;
    int i;
    int nb;
    char *ipaddress = 0;

    if (argc > 1) {
	if ((addr_server_port = atoi(argv[1])) == 0) {
	    fprintf(stderr, "addr_server: malformed port number.\n");
	    exit(2);
	}
	if (argc > 2) {
	    if (inet_addr((ipaddress = argv[2])) == INADDR_NONE) {
		fprintf(stderr, "addr_server: malformed ip address.\n");
		exit(3);
	    }
	}
    } else {
	fprintf(stderr, "addr_server: first arg must be port number.\n");
	exit(1);
    }
    init_conn_sock(addr_server_port, ipaddress);
    while (1) {
	/*
	 * use finite timeout for robustness.
	 */
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	/*
	 * clear selectmasks.
	 */
	FD_ZERO(&readmask);
	/*
	 * set new connection accept fd in readmask.
	 */
	FD_SET(conn_fd, &readmask);
	/*
	 * set active fds in readmask.
	 */
	for (i = 0; i < MAX_CONNS; i++) {
	    if (all_conns[i].state == CONN_OPEN)
		FD_SET(all_conns[i].fd, &readmask);
	}
#ifndef hpux
	nb = select(FD_SETSIZE, &readmask, (fd_set *) 0, (fd_set *) 0, &timeout);
#else
	nb = select(FD_SETSIZE, (int *) &readmask, (int *) 0, (int *) 0, &timeout);
#endif
	if (nb != 0)
	    aserv_process_io(nb);
	process_queue();
    }
    /* the following is to shut lint up */
    /*NOTREACHED*/
    return 0;			/* never reached */
}

#ifdef WIN32
void debug_message(char *fmt, ...)
{
    static char deb_buf[1024];
    static char *deb = deb_buf;
    va_list args;

    V_START(args, fmt);
    V_VAR(char *, fmt, args);
    vfprintf(stderr, fmt, args);
    fflush(stderr);
    va_end(args);
}
#endif
