/*
 *  comm.c -- communications functions and more.
 *            Dwayne Fontenot (Jacques@TMI)
 */
#include "std.h"
#include "network_incl.h"
#include "lpc_incl.h"
#include "applies.h"
#include "main.h"
#include "comm.h"
#include "socket_efuns.h"
#include "backend.h"
#include "socket_ctrl.h"
#include "eoperators.h"
#include "debug.h"

#define TELOPTS
#ifdef OS2
#include "os2\\telnet.h"

#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORE
#define INCL_DOSNMPIPES
#define INCL_DOSERRORS
#include <os2.h>

extern HEV mudos_event_sem;

#define OPEN_MODE NP_ACCESS_DUPLEX
#define PIPE_MODE NP_WAIT | NP_WMESG | NP_RMESG
#define OUTBUF_SIZE 4096
#define INBUF_SIZE 2048
#define TIMEOUT 10000
#endif

int total_users = 0;

/*
 * local function prototypes.
 */
static int flush_message PROT((void));
static int copy_chars PROT((unsigned char *, unsigned char *, int, struct interactive *));

#ifdef SIGNAL_FUNC_TAKES_INT
static void sigpipe_handler PROT((int));
#else
static void sigpipe_handler PROT((void));
#endif
static void hname_handler PROT((void));
static void get_user_data PROT((struct interactive *));
static char *get_user_command PROT((void));
static char *first_cmd_in_buf PROT((struct interactive *));
static int cmd_in_buf PROT((struct interactive *));
static void next_cmd_in_buf PROT((struct interactive *));
static int call_function_interactive PROT((struct interactive *, char *));
static void print_prompt PROT((void));
static void telnet_neg PROT((char *, char *));
static void query_addr_name PROT((struct object *));
static void got_addr_number PROT((char *, char *));
static void add_ip_entry PROT((long, char *));
#ifndef NO_ADD_ACTION
static void clear_notify PROT((void));
#endif
static void new_user_handler PROT((void));
#ifdef RECEIVE_SNOOP
static void receive_snoop PROT((char *, struct object * ob));
#endif

/*
 * public local variables.
 */
fd_set readmask, writemask;
int num_user;
int num_hidden;			/* for the O_HIDDEN flag.  This counter must
				 * be kept up to date at all times!  If you
				 * modify the O_HIDDEN flag in an object,
				 * make sure that you update this counter if
				 * the object is interactive. */
int add_message_calls = 0;
int inet_packets = 0;
int inet_volume = 0;
struct interactive *all_users[MAX_USERS];

/*
 * private local variables.
 */
#ifdef OS2
static HPIPE new_user_handle = 0;
#else
static int new_user_fd;
static int addr_server_fd = -1;
#endif
#ifdef RECEIVE_SNOOP
static void
receive_snoop P2(char *, buf, struct object *, ob)
{
    push_constant_string(buf);
    apply(APPLY_RECEIVE_SNOOP, ob, 1, ORIGIN_DRIVER);
}
#endif

#ifdef OS2
void listen_connection P1(HPIPE, handle)
{
    int i, br, state;
    char buf[2];
    AVAILDATA avail;

    while (DosConnectNPipe(handle) == ERROR_PIPE_NOT_CONNECTED);
    DosPostEventSem(mudos_event_sem);
    new_user_handle = handle;
}				/* listen_connection() */

void spawn_new_listen_pipe()
{
    TID bing;
    int rc;
    HPIPE listen_pipe;

    rc = DosCreateNPipe("\\PIPE\\MUD_DRIVER", &listen_pipe, OPEN_MODE,
			PIPE_MODE | (MAX_USERS + 1), OUTBUF_SIZE, INBUF_SIZE,
			TIMEOUT);
    if (rc)
	return;
    DosCreateThread(&bing, listen_connection, listen_pipe, 0, 100);
}				/* spawn_new_listen_pipe() */

void check_for_data_thread()
{
    int i;

    do {
	for (i = 0; i < MAX_USERS; i++) {
	    long state, br;
	    AVAILDATA avail;
	    char buf[3];

	    if (!all_users[i] || (all_users[i]->iflags & (CLOSING | CMD_IN_BUF))
		continue;
	    if (DosPeekNPipe(all_users[i]->named_pipe, buf, 2, &br, &avail, &state)) {
		continue;
	    }
	    if (avail.cbpipe) {
		DosPostEventSem(mudos_event_sem);
		break;
	    }
	}
	DosSleep(1000);
    } while (1);
}				/* check_for_data_thread() */
#endif

/*
 * Initialize new user connection socket.
 */
void init_user_conn()
{
#ifdef OS2
    TID bing;

    spawn_new_listen_pipe();
    DosCreateThread(&bing, check_for_data_thread, NULL, 0, 100);
#else
    struct sockaddr_in sin;
    int sin_len;
    int optval;

    /*
     * create socket of proper type.
     */
    if ((new_user_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("init_user_conn: socket");
	exit(1);
    }
    /*
     * enable local address reuse.
     */
    optval = 1;
    if (setsockopt(new_user_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &optval,
		   sizeof(optval)) == -1) {
	perror("init_user_conn: setsockopt");
	exit(2);
    }
    /*
     * fill in socket address information.
     */
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons((u_short) port_number);
    /*
     * bind name to socket.
     */
    if (bind(new_user_fd, (struct sockaddr *) & sin, sizeof(sin)) == -1) {
	perror("init_user_conn: bind");
	exit(3);
    }
    /*
     * get socket name.
     */
    sin_len = sizeof(sin);
    if (getsockname(new_user_fd, (struct sockaddr *) & sin, &sin_len) == -1) {
	perror("init_user_conn: getsockname");
	exit(4);
    }
    /*
     * register signal handler for SIGPIPE.
     */
#ifndef LATTICE
    if (signal(SIGPIPE, sigpipe_handler) == SIGNAL_ERROR) {
	perror("init_user_conn: signal SIGPIPE");
	exit(5);
    }
#endif
    /*
     * set socket non-blocking,
     */
    if (set_socket_nonblocking(new_user_fd, 1) == -1) {
	perror("init_user_conn: set_socket_nonblocking 1");
	exit(8);
    }
    /*
     * listen on socket for connections.
     */
    if (listen(new_user_fd, SOMAXCONN) == -1) {
	perror("init_user_conn: listen");
	exit(10);
    }
#endif
}

/*
 * Shut down new user accept file descriptor.
 */
void ipc_remove()
{
#ifdef OS2
    DosDisConnectNPipe(new_user_handle);
    DosClose(new_user_handle);
#else
    if (close(new_user_fd) == -1) {
	perror("ipc_remove: close");
    }
#endif
    printf("closed new user port\n");
}

void init_addr_server P2(char *, hostname, int, addr_server_port)
{
#ifndef OS2
    struct sockaddr_in server;
    struct hostent *hp;
    int server_fd;
    int optval;
    long addr;

    /*
     * get network host data for hostname.
     */
    if (hostname[0] >= '0' && hostname[0] <= '9' &&
          (addr = inet_addr(&hostname[0])) != -1) {
        hp = gethostbyaddr((char *)&addr, sizeof(addr), AF_INET);
    } else {
        hp = gethostbyname(hostname);
    }
    if (hp == NULL) {
	perror("init_addr_server: gethostbyname");
	return;
    }

    /*
     * set up address information for server.
     */
    server.sin_family = AF_INET;
    server.sin_port = htons((u_short) addr_server_port);
    server.sin_addr.s_addr = inet_addr(hostname);
    memcpy((char *) &server.sin_addr, (char *) hp->h_addr, hp->h_length);
    /*
     * create socket of proper type.
     */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {	/* problem opening socket */
	perror("init_addr_server: socket");
	return;
    }
    /*
     * enable local address reuse.
     */
    optval = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &optval,
		   sizeof(optval)) == -1) {
	perror("init_addr_server: setsockopt");
	return;
    }
    /*
     * connect socket to server address.
     */
    if (connect(server_fd, (struct sockaddr *) & server, sizeof(server)) == -1) {
	perror("init_addr_server: connect");
	close(server_fd);
	return;
    }
    addr_server_fd = server_fd;
    fprintf(stderr, "Connected to address server on %s port %d\n", hostname,
	    addr_server_port);
    /*
     * set socket non-blocking.
     */
    if (set_socket_nonblocking(server_fd, 1) == -1) {
	perror("init_addr_server: set_socket_nonblocking 1");
	return;
    }
#endif
}

/*
 * Send a message to an interactive object. If that object is shadowed,
 * special handling is done.
 */
void add_message P1(char *, data)
{
    struct interactive *ip;
    char *cp;
    struct object *save_command_giver;

    /*
     * if command_giver->interactive is not valid, write message on stderr.
     * (maybe)
     */
    if ((command_giver == 0) || (command_giver->flags & O_DESTRUCTED)
	|| (command_giver->interactive == 0)
	|| (command_giver->interactive->iflags & (NET_DEAD | CLOSING))) {
#ifdef NONINTERACTIVE_STDERR_WRITE
	putc(']', stderr);
	fprintf(stderr, data);
#endif
#ifdef LATTICE
	fflush(stderr);
#endif
	return;
    }
    ip = command_giver->interactive;
#ifndef NO_SHADOWS
    /*
     * shadow handling.
     */
    if (shadow_catch_message(command_giver, data)) {
	/*
	 * snoop handling.
	 */
#ifdef SNOOP_SHADOWED
	if (ip->snoop_by) {
	    save_command_giver = command_giver;
	    command_giver = ip->snoop_by->ob;
#ifdef RECEIVE_SNOOP
	    receive_snoop(command_giver);
#else
	    add_message("$$ ");
	    add_message(data);
#endif
	    command_giver = save_command_giver;
	}
#endif
	return;
    }
#endif				/* NO_SHADOWS */

    /*
     * write message into command_giver->interactive->message_buf.
     */
    for (cp = data; *cp != '\0'; cp++) {
	if (ip->message_length == MESSAGE_BUF_SIZE) {
	    if (!flush_message()) {
		fprintf(stderr, "Broken connection during add_message.\n");
		return;
	    }
	    if (ip->message_length == MESSAGE_BUF_SIZE)
		break;
	}
	if (*cp == '\n') {
	    if (ip->message_length == (MESSAGE_BUF_SIZE - 1)) {
		if (!flush_message()) {
		    fprintf(stderr, "Broken connection during add_message.\n");
		    return;
		}
		if (ip->message_length == (MESSAGE_BUF_SIZE - 1))
		    break;
	    }
	    ip->message_buf[ip->message_producer] = '\r';
	    ip->message_producer = (ip->message_producer + 1)
		% MESSAGE_BUF_SIZE;
	    ip->message_length++;
	}
	ip->message_buf[ip->message_producer] = *cp;
	ip->message_producer = (ip->message_producer + 1) % MESSAGE_BUF_SIZE;
	ip->message_length++;
    }
    if (ip->message_length != 0) {
	if (!flush_message()) {
	    fprintf(stderr, "Broken connection during add_message.\n");
	    return;
	}
    }
    /*
     * snoop handling.
     */
    if (ip->snoop_by) {
	save_command_giver = command_giver;
	command_giver = ip->snoop_by->ob;
#ifdef RECEIVE_SNOOP
	receive_snoop(data, command_giver);
#else
	add_message("% ");
	add_message(data);
#endif
	command_giver = save_command_giver;
    }
    add_message_calls++;
}				/* add_message() */

void add_vmessage PVARGS(va_alist)
{
    va_list args;
    char *format;
    struct interactive *ip;
    char *cp, new_string_data[LARGEST_PRINTABLE_STRING];
    struct object *save_command_giver;

#ifdef HAS_STDARG_H
    va_start(args, va_alist);
    format = va_alist;
#else
    va_start(args);
    format = va_arg(args, char *);
#endif
    /*
     * if command_giver->interactive is not valid, write message on stderr.
     * (maybe)
     */
    if ((command_giver == 0) || (command_giver->flags & O_DESTRUCTED)
	|| (command_giver->interactive == 0)
	|| (command_giver->interactive->iflags & (NET_DEAD | CLOSING))) {
#ifdef NONINTERACTIVE_STDERR_WRITE
	putc(']', stderr);
	vfprintf(stderr, format, args);
#endif
	va_end(args);
#ifdef LATTICE
	fflush(stderr);
#endif
	return;
    }
    ip = command_giver->interactive;
    new_string_data[0] = '\0';
    /*
     * this is dangerous since the data may not all fit into new_string_data
     * but how to tell if it will without trying it first?  I suppose one
     * could rewrite vsprintf to accept a maximum length (like strncpy) --
     * have fun!
     */
    vsprintf(new_string_data, format, args);
    va_end(args);
#ifndef NO_SHADOWS
    /*
     * shadow handling.
     */
    if (shadow_catch_message(command_giver, new_string_data)) {
	/*
	 * snoop handling.
	 */
#ifdef SNOOP_SHADOWED
	if (ip->snoop_by) {
	    save_command_giver = command_giver;
	    command_giver = ip->snoop_by->ob;
#ifdef RECEIVE_SNOOP
	    receive_snoop(new_string_data, command_giver);
#else
	    add_message("$$ ");
	    add_message(new_string_data);
#endif
	    command_giver = save_command_giver;
	}
#endif
	return;
    }
#endif				/* NO_SHADOWS */

    /*
     * write message into command_giver->interactive->message_buf.
     */
    for (cp = new_string_data; *cp != '\0'; cp++) {
	if (ip->message_length == MESSAGE_BUF_SIZE) {
	    if (!flush_message()) {
		fprintf(stderr, "Broken connection during add_message.\n");
		return;
	    }
	    if (ip->message_length == MESSAGE_BUF_SIZE)
		break;
	}
	if (*cp == '\n') {
	    if (ip->message_length == (MESSAGE_BUF_SIZE - 1)) {
		if (!flush_message()) {
		    fprintf(stderr, "Broken connection during add_message.\n");
		    return;
		}
		if (ip->message_length == (MESSAGE_BUF_SIZE - 1))
		    break;
	    }
	    ip->message_buf[ip->message_producer] = '\r';
	    ip->message_producer = (ip->message_producer + 1)
		% MESSAGE_BUF_SIZE;
	    ip->message_length++;
	}
	ip->message_buf[ip->message_producer] = *cp;
	ip->message_producer = (ip->message_producer + 1) % MESSAGE_BUF_SIZE;
	ip->message_length++;
    }
    if (ip->message_length != 0) {
	if (!flush_message()) {
	    fprintf(stderr, "Broken connection during add_message.\n");
	    return;
	}
    }
    /*
     * snoop handling.
     */
    if (ip->snoop_by) {
	save_command_giver = command_giver;
	command_giver = ip->snoop_by->ob;
#ifdef RECEIVE_SNOOP
	receive_snoop(new_string_data, command_giver);
#else
	add_message("% ");
	add_message(new_string_data);
#endif
	command_giver = save_command_giver;
    }
    add_message_calls++;
}				/* add_message() */

/*
 * Flush outgoing message buffer of current interactive object.
 */
static int flush_message()
{
    struct interactive *ip;
    int length, num_bytes;

    /*
     * if command_giver->interactive is not valid, do nothing.
     */
    if (command_giver == 0 ||
	(command_giver->flags & O_DESTRUCTED) ||
	command_giver->interactive == 0 ||
	(command_giver->interactive->iflags & CLOSING)) {
	fprintf(stderr, "flush_message: invalid command_giver!\n");
	return 0;
    }
    ip = command_giver->interactive;
    /*
     * write command_giver->interactive->message_buf[] to socket.
     */
    while (ip->message_length != 0) {
	if (ip->message_consumer < ip->message_producer) {
	    length = ip->message_producer - ip->message_consumer;
	} else {
	    length = MESSAGE_BUF_SIZE - ip->message_consumer;
	}
/* Need to use send to get out of band data
		num_bytes = write(ip->fd,ip->message_buf + ip->message_consumer,length);
 */
#ifdef OS2
	if (DosWrite(ip->named_pipe, ip->message_buf + ip->message_consumer, length,
		     &num_bytes))
	    num_bytes = -1;
#else
	num_bytes = send(ip->fd, ip->message_buf + ip->message_consumer,
			 length, ip->out_of_band);
#endif
	if (num_bytes == -1) {
#ifdef EWOULDBLOCK
	    if (errno == EWOULDBLOCK) {
		debug(512, ("flush_message: write: Operation would block\n"));
		return 1;
#else
	    if (0) {
		;
#endif
#ifdef linux
	    } else if (errno == EINTR) {
		debug(512, ("flush_message: write: Interrupted system call"));
		return 1;
#endif
	    } else {
		perror("flush_message: write");
		ip->iflags |= NET_DEAD;
		return 0;
	    }
	}
	ip->message_consumer = (ip->message_consumer + num_bytes) %
	    MESSAGE_BUF_SIZE;
	ip->message_length -= num_bytes;
	ip->out_of_band = 0;
	inet_packets++;
	inet_volume += num_bytes;
    }
    return 1;
}				/* flush_message() */

#define TS_DATA         0
#define TS_IAC          1
#define TS_WILL         2
#define TS_WONT         3
#define TS_DO           4
#define TS_DONT         5
#define TS_SB		6
/*
 * Copy a string, replacing newlines with '\0'. Also add an extra
 * space and back space for every newline. This trick will allow
 * otherwise empty lines, as multiple newlines would be replaced by
 * multiple zeroes only.
 *
 * Also handle the telnet stuff.  So instead of this being a direct
 * copy it is a small state thingy.
 *
 * In fact, it is telnet_neg conglomerated into this.  This is mostly
 * done so we can sanely remove the telnet sub option negotation stuff
 * out of the input stream.  Need this for terminal types.
 * (Pinkfish change)
 */
/* the codes we send ... */
static char telnet_break_response[] = { 28, IAC, WILL, TELOPT_TM, 0 };
static char telnet_interrupt_response[] = { 127, IAC, WILL, TELOPT_TM, 0 };
static char telnet_abort_response[] = { IAC, DM, 0 };
static char telnet_do_tm_response[] = { IAC, WILL, TELOPT_TM, 0 };
static char telnet_term_query[] = { IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, 
				      IAC, SE, 0 };
static char telnet_no_echo[] = { IAC, WONT, TELOPT_ECHO, 0 };
static char telnet_no_single[] = { IAC, WONT, TELOPT_SGA, 0 };
static char telnet_yes_echo[] = { IAC, WILL, TELOPT_ECHO, 0 };
static char telnet_yes_single[] = { IAC, WILL, TELOPT_SGA, 0 };
static char telnet_ga[] = { IAC, GA, 0 };

static int copy_chars P4(unsigned char *, from, unsigned char *, to, int, n, struct interactive *, ip)
{
    int i;
    struct object *save_command_giver;
    unsigned char *start = to;

    for (i = 0; i < n; i++) {
	switch (ip->state) {
	case TS_DATA:
	    switch (from[i]) {
	    case IAC:
		ip->state = TS_IAC;
		break;
	    case '\r':
		if (ip->iflags & SINGLE_CHAR)
		    *to++ = from[i];
		break;
	    case '\n':
		if (ip->iflags & SINGLE_CHAR)
		    *to++ = from[i];
		else {
		    *to++ = ' ';
		    *to++ = '\b';
		    *to++ = '\0';
		}
		break;
	    default:
		*to++ = from[i];
		break;
	    }
	    break;
	case TS_IAC:
	    switch (from[i]) {
	    case DO:
		ip->state = TS_DO;
		break;
	    case DONT:
		ip->state = TS_DONT;
		break;
	    case WILL:
		ip->state = TS_WILL;
		break;
	    case WONT:
		ip->state = TS_WONT;
		break;
	    case BREAK:
/* Send back a break character. */
		save_command_giver = command_giver;
		command_giver = ip->ob;
		add_message(telnet_break_response);
		flush_message();
		command_giver = save_command_giver;
		break;
	    case IP:
/* Send back an interupt process character. */
		save_command_giver = command_giver;
		command_giver = ip->ob;
		add_message(telnet_interrupt_response);
		flush_message();
		command_giver = save_command_giver;
		break;
	    case AYT:
/* Are you there signal.  Yep we are. */
		save_command_giver = command_giver;
		command_giver = ip->ob;
		add_message("\n[Yes]\n");
		command_giver = save_command_giver;
		break;
	    case AO:
/* Abort output. Do a telnet sync operation. */
		save_command_giver = command_giver;
		command_giver = ip->ob;
		ip->out_of_band = MSG_OOB;
		add_message(telnet_abort_response);
		flush_message();
		command_giver = save_command_giver;
		break;
	    case SB:
		ip->state = TS_SB;
		ip->sb_pos = 0;
		break;
/* SE counts as going back into data mode */
	    case SE:
/*
 * Ok...  need to call a function on the interactive object, passing the
 * buffer as a paramater.
 */
		save_command_giver = command_giver;
		command_giver = ip->ob;
		ip->sb_buf[ip->sb_pos] = 0;
		if (ip->sb_buf[0]==TELOPT_TTYPE && ip->sb_buf[1]=='I') {
		    /* TELOPT_TTYPE TELQUAL_IS means it's telling us it's
		       terminal type */
		    push_constant_string(ip->sb_buf + 2);
		    apply(APPLY_TERMINAL_TYPE, ip->ob, 1, ORIGIN_DRIVER);
		    ip->iflags |= USING_TELNET;
		} else {
		    push_constant_string(ip->sb_buf);
		    apply(APPLY_TELNET_SUBOPTION, ip->ob, 1, ORIGIN_DRIVER);
		    command_giver = save_command_giver;
		}
		ip->state = TS_DATA;
		break;
	    case DM:
	    default:
		ip->state = TS_DATA;
		break;
	    }
	    break;
	case TS_DO:
	    if (from[i] == TELOPT_TM) {
		save_command_giver = command_giver;
		command_giver = ip->ob;
		add_message(telnet_do_tm_response);
		flush_message();
		command_giver = save_command_giver;
	    }
	case TS_DONT:
	case TS_WILL:
	case TS_WONT:
	    ip->state = TS_DATA;
	    break;
	case TS_SB:
	    if ((unsigned char) from[i] == IAC) {
		ip->state = TS_IAC;
		break;
	    }
/* Ok, put all the suboption stuff into the buffer on the interactive */
	    if (ip->sb_pos >= SB_SIZE)
		break;		/* Ignore stuff outside the range */
	    if (from[i])
		ip->sb_buf[ip->sb_pos++] = from[i];
	    else
		ip->sb_buf[ip->sb_pos++] = 'I';	/* Turn 0's into I's */
	    break;
	}
    }
    return (to - start);
}				/* copy_chars() */

/*
 * SIGPIPE handler -- does very little for now.
 */
#ifdef SIGNAL_FUNC_TAKES_INT
static void sigpipe_handler P1(int, sig)
#else
static void sigpipe_handler()
#endif
{
    fprintf(stderr, "SIGPIPE received.\n");
#ifdef linux
    signal(SIGPIPE, sigpipe_handler);
#endif
}				/* sigpipe_handler() */

/*
 * SIGALRM handler.
 */
#ifdef SIGNAL_FUNC_TAKES_INT
void sigalrm_handler P1(int, sig)
#else
void sigalrm_handler()
#endif
{
    heart_beat_flag = 1;
    debug(512, ("sigalrm_handler: SIGALRM\n"));
}				/* sigalrm_handler() */

INLINE void make_selectmasks()
{
#ifndef OS2
    int i;

    /*
     * generate readmask and writemask for select() call.
     */
    FD_ZERO(&readmask);
    FD_ZERO(&writemask);
    /*
     * set new user accept fd in readmask.
     */
    FD_SET(new_user_fd, &readmask);
    /*
     * set user fds in readmask.
     */
    for (i = 0; i < MAX_USERS; i++) {
	if (!all_users[i] || (all_users[i]->iflags & (CLOSING | CMD_IN_BUF)))
	    continue;
	/*
	 * if this user needs more input to make a complete command, set his
	 * fd so we can get it.
	 */
	FD_SET(all_users[i]->fd, &readmask);
	if (all_users[i]->message_length != 0)
	    FD_SET(all_users[i]->fd, &writemask);
    }
    /*
     * if addr_server_fd is set, set its fd in readmask.
     */
    if (addr_server_fd >= 0) {
	FD_SET(addr_server_fd, &readmask);
    }
#ifdef SOCKET_EFUNS
    /*
     * set fd's for efun sockets.
     */
    for (i = 0; i < MAX_EFUN_SOCKS; i++) {
	if (lpc_socks[i].state != CLOSED) {
	    if ((lpc_socks[i].flags & S_WACCEPT) == 0)
		FD_SET(lpc_socks[i].fd, &readmask);
	    if (lpc_socks[i].flags & S_BLOCKED)
		FD_SET(lpc_socks[i].fd, &writemask);
	}
    }
#endif
#endif				/* OS2 */
}				/* make_selectmasks() */

/*
 * Process I/O.
 */
INLINE void process_io()
{
#ifdef OS2
    int tmp;

    if (new_user_handle) {
	new_user_handler();
	new_user_handle = 0;
	spawn_new_listen_pipe();
    }
    for (i = 0; i < MAX_USERS; i++) {
	long state, br;
	AVAILDATA avail;
	char buf[3];

	if (!all_users[i] || (all_users[i] & (CLOSING | CMD_IN_BUF))
	    continue;
	if (all_users[i]->iflags & NET_DEAD) {
	    remove_interactive(all_users[i]->ob);
	    continue;
	}
	if (DosPeekNPipe(all_users[i]->named_pipe, buf, 2, &br, &avail, &state)) {
	    remove_interactive(all_users[i]->ob);
	    continue;
	}
	if (all_users[i]->message_length > 0 && !avail.cbmessage) {
	    save_command_giver = command_giver;
	    command_giver = all_users[i]->ob;
	    flush_message();
	    command_giver = save_command_giver;
	}
	if (avail.cbpipe) {
	    get_user_data(all_users[i]);
	}
    }
#else
    int i;
    struct object *save_command_giver;

    debug(256, ("@"));
    /*
     * check for new user connection.
     */
    if (FD_ISSET(new_user_fd, &readmask)) {
	debug(512, ("process_io: NEW_USER\n"));
	new_user_handler();
    }
    /*
     * check for data pending on user connections.
     */
    for (i = 0; i < MAX_USERS; i++) {
	if (!all_users[i] || (all_users[i]->iflags & (CLOSING | CMD_IN_BUF)))
	    continue;
	if (all_users[i]->iflags & NET_DEAD) {
	    remove_interactive(all_users[i]->ob);
	    continue;
	}
	if (FD_ISSET(all_users[i]->fd, &readmask)) {
	    debug(512, ("process_io: USER %d\n", i));
	    get_user_data(all_users[i]);
	    if (!all_users[i])
		continue;
	}
	if (FD_ISSET(all_users[i]->fd, &writemask)) {
	    save_command_giver = command_giver;
	    command_giver = all_users[i]->ob;
	    flush_message();
	    command_giver = save_command_giver;
	}
    }
#ifdef SOCKET_EFUNS
    /*
     * check for data pending on efun socket connections.
     */
    for (i = 0; i < MAX_EFUN_SOCKS; i++) {
	if (lpc_socks[i].state != CLOSED)
	    if (FD_ISSET(lpc_socks[i].fd, &readmask))
		socket_read_select_handler(i);
	if (lpc_socks[i].state != CLOSED)
	    if (FD_ISSET(lpc_socks[i].fd, &writemask))
		socket_write_select_handler(i);
    }
#endif
    /*
     * check for data pending from address server.
     */
    if (addr_server_fd >= 0) {
	if (FD_ISSET(addr_server_fd, &readmask)) {
	    debug(512, ("process_io: IP_DAEMON\n"));
	    hname_handler();
	}
    }
#endif
}

/*
 * This is the new user connection handler. This function is called by the
 * event handler when data is pending on the listening socket (new_user_fd).
 * If space is available, an interactive data structure is initialized and
 * the user is connected.
 */
static void new_user_handler()
{
    int new_socket_fd;

#ifndef OS2
    struct sockaddr_in addr;

#endif
    int length;
    int i;
    char *full_message;
    struct object *ob;
    struct svalue *ret;
    int err;

#ifndef OS2
    length = sizeof(addr);
    debug(512, ("new_user_handler: accept on fd %d\n", new_user_fd));
    new_socket_fd = accept(new_user_fd, (struct sockaddr *) & addr, (int *) &length);
    if (new_socket_fd < 0) {
	if (errno == EWOULDBLOCK) {
	    debug(512, ("new_user_handler: accept: Operation would block\n"));
	} else {
	    perror("new_user_handler: accept");
	}
	return;
    }
#endif
#ifdef linux
    /*
     * according to Amylaar, 'accepted' sockets in Linux 0.99p6 don't
     * properly inherit the nonblocking property from the listening socket.
     */
    if (set_socket_nonblocking(new_socket_fd, 1) == -1) {
	perror("new_user_handler: set_socket_nonblocking 1");
	exit(8);
    }
#endif				/* linux */
    for (i = 0; i < MAX_USERS; i++) {
	if (all_users[i] != 0)
	    continue;
	err = assert_master_ob_loaded("[internal] new_user_handler","");
	if (err != 1) {
	    fprintf(stderr, "Can't connect with no master object.\n");
	    close(new_socket_fd);
#ifndef OS2
	    fprintf(stderr, "Connection from %s aborted.\n", inet_ntoa(addr.sin_addr));
#endif
	    return;
	}
	command_giver = master_ob;
	master_ob->interactive =
	    (struct interactive *)
	    DXALLOC(sizeof(struct interactive), TAG_INTERACTIVE, "new_user_handler");
	total_users++;
#ifndef NO_ADD_ACTION
	master_ob->interactive->default_err_message.s = 0;
#endif
	master_ob->flags |= O_ONCE_INTERACTIVE;
	/*
	 * initialize new user interactive data structure.
	 */
	master_ob->interactive->ob = master_ob;
	master_ob->interactive->input_to = 0;
	master_ob->interactive->iflags = 0;
	master_ob->interactive->text[0] = '\0';
	master_ob->interactive->text_end = 0;
	master_ob->interactive->text_start = 0;
	master_ob->interactive->carryover = NULL;
	master_ob->interactive->snoop_on = 0;
	master_ob->interactive->snoop_by = 0;
	master_ob->interactive->last_time = current_time;
#ifdef TRACE
	master_ob->interactive->trace_level = 0;
	master_ob->interactive->trace_prefix = 0;
#endif
#ifdef OLD_ED
	master_ob->interactive->ed_buffer = 0;
#endif
	master_ob->interactive->message_producer = 0;
	master_ob->interactive->message_consumer = 0;
	master_ob->interactive->message_length = 0;
	master_ob->interactive->num_carry = 0;
	master_ob->interactive->state = TS_DATA;
	master_ob->interactive->out_of_band = 0;
	all_users[i] = master_ob->interactive;
#ifdef OS2
	all_users[i]->named_pipe = new_user_handle;
	new_user_handle = 0;
#else
	all_users[i]->fd = new_socket_fd;
#endif
	set_prompt("> ");

#ifndef OS2
	memcpy((char *) &all_users[i]->addr, (char *) &addr, length);
	debug(512, ("New connection from %s.\n", inet_ntoa(addr.sin_addr)));
#endif
	num_user++;
	/*
	 * The user object has one extra reference. It is asserted that the
	 * master_ob is loaded.
	 */
	add_ref(master_ob, "new_user");
	ret = apply_master_ob(APPLY_CONNECT, 0);
	if (ret == 0 || ret == (struct svalue *)-1 || ret->type != T_OBJECT) {
	    remove_interactive(master_ob);
#ifndef OS2
	    fprintf(stderr, "Connection from %s aborted.\n", inet_ntoa(addr.sin_addr));
#endif
	    return;
	}
	/*
	 * There was an object returned from connect(). Use this as the user
	 * object.
	 */
	ob = ret->u.ob;
	if (ob->flags & O_HIDDEN)
	    num_hidden++;
	ob->interactive = master_ob->interactive;
	ob->interactive->ob = ob;
	ob->flags |= O_ONCE_INTERACTIVE;
	/*
	 * assume the existance of write_prompt and process_input in user.c
	 * until proven wrong (after trying to call them).
	 */
	ob->interactive->iflags |= (HAS_WRITE_PROMPT | HAS_PROCESS_INPUT);

	master_ob->flags &= ~O_ONCE_INTERACTIVE;
	master_ob->interactive = 0;
	free_object(master_ob, "reconnect");
	add_ref(ob, "new_user");
	command_giver = ob;
#ifndef OS2
	if (addr_server_fd >= 0) {
	    query_addr_name(ob);
	}
#endif

	/* Ask for their terminal type */
	add_message(telnet_term_query);

	logon(ob);
	debug(512, ("new_user_handler: end\n"));
	return;
    }
    full_message = "No user slots available: closing connection...\r\n";
#ifdef OS2
    DosWrite(new_user_handle, full_message, strlen(full_message), &i);
    DosDisConnectNPipe(new_user_handle);
    DosClose(new_user_handle);
    new_user_handle = 0;
#else
    write(new_socket_fd, full_message, strlen(full_message));
    if (close(new_socket_fd) == -1) {
	perror("new_user_handler: close");
    }
#endif
}				/* new_user_handler() */

/*
 * This is the user command handler. This function is called when
 * a user command needs to be processed.
 * This function calls get_user_command() to get a user command.
 * One user command is processed per execution of this function.
 */
int process_user_command()
{
    char *user_command;
    static char buf[MAX_TEXT], *tbuf;
    struct object *save_current_object = current_object;
    struct object *save_command_giver = command_giver;
    struct svalue *ret;

    buf[MAX_TEXT - 1] = '\0';
    if ((user_command = get_user_command())) {
	if (command_giver->flags & O_DESTRUCTED)
	    return (1);
#ifndef NO_ADD_ACTION
	clear_notify();		/* moved from user_parser() */
#endif
	update_load_av();
	current_object = 0;
	current_interactive = command_giver;
	debug(512, ("process_user_command: command_giver = %s\n",
		    command_giver->name));
	tbuf = user_command;
	if ((user_command[0] == '!') && (
#ifdef OLD_ED
				    command_giver->interactive->ed_buffer ||
#endif
				    (command_giver->interactive->input_to
				  && !(command_giver->interactive->iflags & NOESC)))) {
	    if (command_giver->interactive->iflags & HAS_PROCESS_INPUT) {
		push_constant_string(user_command + 1);	/* not malloc'ed */
		ret = apply(APPLY_PROCESS_INPUT, command_giver, 1, ORIGIN_DRIVER);
#ifdef NO_ADD_ACTION
		if (!ret && command_giver->interactive)
		    command_giver->interactive->iflags &= ~HAS_PROCESS_INPUT;
#else
		if (ret && (ret->type == T_STRING) && ret->u.string) {
		    strncpy(buf + 1, ret->u.string, MAX_TEXT - 2);
		    tbuf = buf;
		} else if (command_giver->interactive) {
		    command_giver->interactive->iflags &= ~HAS_PROCESS_INPUT;
		}
#endif
	    }
	    if (!command_giver || (command_giver->flags & O_DESTRUCTED)) {
		return 1;
	    }
#ifndef NO_ADD_ACTION
	    parse_command(tbuf + 1, command_giver);
#endif
#ifdef OLD_ED
	} else if (command_giver->interactive->ed_buffer) {
	    ed_cmd(user_command);
#endif				/* ED */
	} else if (call_function_interactive(command_giver->interactive,
					     user_command)) {
	    ;			/* do nothing */
	} else {
	    /*
	     * send a copy of user input back to user object to provide
	     * support for things like command history and mud shell
	     * programming languages.
	     */
	    if (command_giver->interactive->iflags & HAS_PROCESS_INPUT) {
		push_constant_string(user_command);	/* not malloc'ed */
		ret = apply(APPLY_PROCESS_INPUT, command_giver, 1, ORIGIN_DRIVER);
#ifdef NO_ADD_ACTION
		if (!ret && command_giver->interactive)
		    command_giver->interactive->iflags &= ~HAS_PROCESS_INPUT;
#else
		if (ret && (ret->type == T_STRING) && ret->u.string) {
		    strncpy(buf, ret->u.string, MAX_TEXT - 1);
		    tbuf = buf;
		} else if (command_giver->interactive) {
		    command_giver->interactive->iflags &= ~HAS_PROCESS_INPUT;
		}
#endif
	    }
	    if (!command_giver || command_giver->flags & O_DESTRUCTED) {
		return 1;
	    }
#ifndef NO_ADD_ACTION
	    parse_command(tbuf, command_giver);
#endif
	}
	/*
	 * Print a prompt if user is still here.
	 */
	if (command_giver->interactive)
	    print_prompt();
	current_object = save_current_object;
	command_giver = save_command_giver;
	return (1);
    }
    current_object = save_current_object;
    command_giver = save_command_giver;
    return (0);
}				/* process_user_command() */

#define HNAME_BUF_SIZE 200
/*
 * This is the hname input data handler. This function is called by the
 * master handler when data is pending on the hname socket (addr_server_fd).
 */

static void hname_handler()
{
#ifndef OS2
    static char hname_buf[HNAME_BUF_SIZE];
    int num_bytes;
    int tmp;
    char *pp, *q;
    long laddr;

    if (addr_server_fd < 0) {
	return;
    }
    num_bytes = read(addr_server_fd, hname_buf, HNAME_BUF_SIZE);
    switch (num_bytes) {
    case -1:
	switch (errno) {
#ifdef EWOULDBLOCK
	case EWOULDBLOCK:
	    debug(512, ("hname_handler: read on fd %d: Operation would block.\n",
			addr_server_fd));
	    break;
#endif
	default:
	    fprintf(stderr, "hname_handler: read on fd %d\n", addr_server_fd);
	    perror("hname_handler: read");
	    tmp = addr_server_fd;
	    addr_server_fd = -1;
	    close(tmp);
	    return;
	}
	break;
    case 0:
	fprintf(stderr, "hname_handler: closing address server connection.\n");
	tmp = addr_server_fd;
	addr_server_fd = -1;
	close(tmp);
	return;
    default:
	hname_buf[num_bytes] = '\0';
	debug(512, ("hname_handler: address server replies: %s", hname_buf));
	if (hname_buf[0] >= '0' && hname_buf[0] <= '9') {
	    laddr = inet_addr(hname_buf);
	    if (laddr != -1) {
		pp = strchr(hname_buf, ' ');
		if (pp) {
		    *pp = 0;
		    pp++;
		    q = strchr(pp, '\n');
		    if (q) {
			*q = 0;
			if (strcmp(pp, "0"))
			    add_ip_entry(laddr, pp);
			got_addr_number(pp, hname_buf);	/* Recognises this as
							 * failure. */
		    }
		}
	    }
	} else {
	    char *r;

/* This means it was a name lookup... */
	    pp = strchr(hname_buf, ' ');
	    if (pp) {
		*pp = 0;
		pp++;
		r = strchr(pp, '\n');
		if (r)
		    *r = 0;
		got_addr_number(pp, hname_buf);
	    }
	}
	break;
    }
#endif				/* OS2 */
}				/* hname_handler() */

/*
 * Read pending data for a user into user->interactive->text.
 * This also does telnet negotiation.
 */
static void get_user_data P1(struct interactive *, ip)
{
    struct object *save_command_giver;
    static char buf[MAX_TEXT];
    int text_space;
    int num_bytes;
#ifdef DEBUG_COMM_FREEZE
    int i;
#endif

    /*
     * this /3 is here because of the trick copy_chars() uses to allow empty
     * commands. it needs to be fixed right. later.
     */
    text_space = (MAX_TEXT - ip->text_end - 1) / 3;
    /*
     * Check if we need more space.
     */
    if (text_space < MAX_TEXT/16) {
	int l = ip->text_end - ip->text_start;

	memmove(ip->text, ip->text + ip->text_start, l + 1);
	ip->text_start = 0;
	ip->text_end = l;
	text_space = (MAX_TEXT - ip->text_end - 1) / 3;
	if (text_space < MAX_TEXT/16) {
	    /* almost 2k data without a newline.  Flush it, otherwise
	       text_space will eventually go to zero and dest the user. */
	    ip->text_start = 0;
	    ip->text_end = 0;
	    text_space = MAX_TEXT / 3;
	}
    }
    /*
     * read user data.
     */
    debug(512, ("get_user_data: read on fd %d\n", ip->fd));
#ifdef OS2
    DosRead(ip->named_pipe, buf, text_space, &num_bytes);
#else
    num_bytes = read(ip->fd, buf, text_space);
#endif
#ifdef DEBUG_COMM_FREEZE
/* slow, but it's debugging code */
    for (i=0; i<1024; i++) {
	ip->debug_block[i] = buf[i];
    }
    ip->debug_block_size = num_bytes;
#endif
    switch (num_bytes) {
    case 0:
	if (ip->iflags & CLOSING)
	    fprintf(stderr, "get_user_data: tried to read from closing fd.\n");
	remove_interactive(ip->ob);
	return;
    case -1:
#ifdef EWOULDBLOCK
	if (errno == EWOULDBLOCK) {
	    debug(512, ("get_user_data: read on fd %d: Operation would block.\n",
			ip->fd));
	} else
#endif
	{
#ifdef OS2
	    fprintf(stderr, "get_user_data: read on fd %d\n", ip->named_pipe);
#else
	    fprintf(stderr, "get_user_data: read on fd %d\n", ip->fd);
#endif
	    perror("get_user_data: read");
	    remove_interactive(ip->ob);
	    return;
	}
	break;
    default:
	buf[num_bytes] = '\0';
	/*
	 * replace newlines with nulls and catenate to buffer. Also do all
	 * the useful telnet negotation at this point too. Rip out the sub
	 * option stuff and send back anything non useful we feel we have to.
	 */
	ip->text_end += copy_chars((unsigned char *)buf, (unsigned char *)ip->text + ip->text_end, num_bytes, ip);
	/*
	 * now, text->end is just after the last char read. If last char was
	 * a nl, char *before* text_end will be null.
	 */
	ip->text[ip->text_end] = '\0';
	/*
	 * handle snooping - snooper does not see type-ahead. seems like that
	 * would be very inefficient, for little functional gain.
	 */
	if (ip->snoop_by && !(ip->iflags & NOECHO)) {
	    save_command_giver = command_giver;
	    command_giver = ip->snoop_by->ob;
#ifdef RECEIVE_SNOOP
	    receive_snoop(buf, command_giver);
#else
	    add_message("%");
	    add_message(buf);
#endif
	    command_giver = save_command_giver;
	}
	/*
	 * set flag if new data completes command.
	 */
	if (cmd_in_buf(ip))
	    ip->iflags |= CMD_IN_BUF;
	break;
    }
}				/* get_user_data() */

/*
 * Return the first cmd of the next user in sequence that has a complete cmd
 * in their buffer.
 * CmdsGiven is used to allow users in ED to send more cmds (if they have
 * them queued up) than users not in ED.
 * This should also return a value if there is something in the
 * buffer and we are supposed to be in single character mode.
 */
#define StartCmdGiver   (MAX_USERS-1)
#define IncCmdGiver     NextCmdGiver = (NextCmdGiver == 0? StartCmdGiver: \
                                        NextCmdGiver - 1)
static int NextCmdGiver = StartCmdGiver;

#ifdef DEBUG_COMM_FREEZE
static char *debug_dump P2(char *, block, int, size) {
    char buffer[4096];
    char *bufp = buffer;
    int i=0;

    if (size > 1023) size = 1023;
    while (i<size) {
	if (block[i]<32 || block[i]>127) {
	    *bufp++='\\';
	    if (block[i]>99)
		*bufp++ = (block[i]/100) + '0';
	    if (block[i]>9)
		*bufp++ = (block[i] %100)/10 + '0';
	    *bufp++ = block[i]%10 + '0';
	} else {
	    switch(block[i]) {
	      case '\\':
		*bufp++ = '\\';
	      default:
		*bufp++ = block[i];
	    }
	}
    }
    *bufp = 0;
    return buffer;
}
#endif

static char *get_user_command()
{
    int i;
    struct interactive *ip;
    char *user_command = NULL;
    static char buf[MAX_TEXT];

    /*
     * find and return a user command.
     */
    for (i = 0; i < MAX_USERS; i++) {
	ip = all_users[NextCmdGiver];
	if (ip && (ip->iflags & CMD_IN_BUF)) {
	    user_command = first_cmd_in_buf(ip);
	    if (user_command) break;
#ifdef DEBUG_COMM_FREEZE
	    else {
		debug_message("*********************\nFrozen user found.\n");
		debug_message("Last Block = %s\n\n", 
			      debug_dump(ip->debug_block,
			      ip->debug_block_size));
		ip->iflags &= ~CMD_IN_BUF;
	    }		
#else
	    else
		ip->iflags &= ~CMD_IN_BUF;
#endif
	}
	IncCmdGiver;
    }
    /*
     * no cmds found; return(NULL).
     */
    if (!ip || !user_command)
	return ((char *) NULL);
    /*
     * we have a user cmd -- return it. If user has only one partially
     * completed cmd left after this, move it to the start of his buffer; new
     * stuff will be appended.
     */
    debug(512, ("get_user_command: user_command = (%s)\n", user_command));
    command_giver = ip->ob;
    /*
     * telnet option parsing and negotiation.
     */
    telnet_neg(buf, user_command);
    /*
     * move input buffer pointers to next command.
     */
    next_cmd_in_buf(ip);
    if (!cmd_in_buf(ip))
	ip->iflags &= ~CMD_IN_BUF;

    IncCmdGiver;

    if (ip->iflags & NOECHO) {
	/*
	 * Must not enable echo before the user input is received.
	 */
	add_message(telnet_no_echo);
	ip->iflags &= ~NOECHO;
    }
    ip->last_time = current_time;
    return (buf);
}				/* get_user_command() */

/*
 * find the first character of the next complete cmd in a buffer, 0 if no
 * completed cmd.  There is a completed cmd if there is a null between
 * text_start and text_end.  Zero length commands are discarded (as occur
 * between <cr> and <lf>).  Update text_start if we have to skip leading
 * nulls.
 * This should return true when in single char mode and there is
 * Anything at all in the buffer.
 */
static char *first_cmd_in_buf P1(struct interactive *, ip)
{
    char *p, *q;

    p = ip->text + ip->text_start;

    /*
     * skip null input.
     */
    while ((p < (ip->text + ip->text_end)) && !*p)
	p++;

    ip->text_start = p - ip->text;

    if (ip->text_start >= ip->text_end) {
	ip->text_start = ip->text_end = 0;
	ip->text[0] = '\0';
	return ((char *) NULL);
    }
    /* If we got here, must have something in the array */
    if (ip->iflags & SINGLE_CHAR) {
	/* We need to return true here... */
	return (ip->text + ip->text_start);
    }
    /*
     * find end of cmd.
     */
    while ((p < (ip->text + ip->text_end)) && *p)
	p++;
    /*
     * null terminated; was command.
     */
    if (p < ip->text + ip->text_end)
	return (ip->text + ip->text_start);
    /*
     * have a partial command at end of buffer; move it to start, return
     * null. if it can't move down, truncate it and return it as cmd.
     */
    p = ip->text + ip->text_start;
    q = ip->text;
    while (p < (ip->text + ip->text_end))
	*(q++) = *(p++);

    ip->text_end -= ip->text_start;
    ip->text_start = 0;
    if (ip->text_end > MAX_TEXT - 2) {
	ip->text[ip->text_end - 2] = '\0';	/* nulls to truncate */
	ip->text[ip->text_end - 1] = '\0';	/* nulls to truncate */
	ip->text_end--;
	return (ip->text);
    }
    /*
     * buffer not full and no newline - no cmd.
     */
    return ((char *) NULL);
}				/* first_command_in_buf() */

/*
 * return(1) if there is a complete command in ip->text, otherwise return(0).
 */
static int cmd_in_buf P1(struct interactive *, ip)
{
    char *p;

    p = ip->text + ip->text_start;

    /*
     * skip null input.
     */
    while ((p < (ip->text + ip->text_end)) && !*p)
	p++;

    if ((p - ip->text) >= ip->text_end) {
	return (0);
    }
    /* If we get here, must have something in the buffer */
    if (ip->iflags & SINGLE_CHAR) {
	return (1);
    }
    /*
     * find end of cmd.
     */
    while ((p < (ip->text + ip->text_end)) && *p)
	p++;
    /*
     * null terminated; was command.
     */
    if (p < ip->text + ip->text_end)
	return (1);
    /*
     * no newline - no cmd.
     */
    return (0);
}				/* cmd_in_buf() */

/*
 * move pointers to next cmd, or clear buf.
 */
static void next_cmd_in_buf P1(struct interactive *, ip)
{
    char *p = ip->text + ip->text_start;

    while (*p && p < ip->text + ip->text_end)
	p++;
    /*
     * skip past any nulls at the end.
     */
    while (!*p && p < ip->text + ip->text_end)
	p++;
    if (p < ip->text + ip->text_end)
	ip->text_start = p - ip->text;
    else {
	ip->text_start = ip->text_end = 0;
	ip->text[0] = '\0';
    }
}				/* next_cmd_in_buf() */

/*
 * Remove an interactive user immediately.
 */
void remove_interactive P1(struct object *, ob)
{
    struct object *save_command_giver = command_giver;
    int i;

    if (!ob->interactive) {
	return;
    }
    for (i = 0; i < MAX_USERS; i++) {
	if (all_users[i] != ob->interactive)
	    continue;
	if (ob->interactive->iflags & CLOSING) {
	    fprintf(stderr, "Double call to remove_interactive()\n");
	    return;
	}
	ob->interactive->iflags |= CLOSING;

	/*
	 * auto-notification of net death
	 */
	safe_apply(APPLY_NET_DEAD, ob, 0, ORIGIN_DRIVER);

	if (ob->interactive->snoop_by) {
	    ob->interactive->snoop_by->snoop_on = 0;
	    ob->interactive->snoop_by = 0;
	}
	if (ob->interactive->snoop_on) {
	    ob->interactive->snoop_on->snoop_by = 0;
	    ob->interactive->snoop_on = 0;
	}
	command_giver = ob;
	debug(512, ("Closing connection from %s.\n",
		    inet_ntoa(ob->interactive->addr.sin_addr)));
#ifdef F_ED
	if (ob->interactive->ed_buffer) {
	    save_ed_buffer();
	}
#endif
	debug(512, ("remove_interactive: closing fd %d\n", ob->interactive->fd));
#ifdef OS2
	DosDisConnectNPipe(ob->interactive->named_pipe);
	if (DosClose(ob->interactive->named_pipe)) {
	    perror("remove_interactive: close");
	}
#else
	if (close(ob->interactive->fd) == -1) {
	    perror("remove_interactive: close");
	}
#endif
	if (ob->flags & O_HIDDEN)
	    num_hidden--;
	num_user--;
#ifndef NO_ADD_ACTION
	clear_notify();
#endif
	if (ob->interactive->input_to) {
	    free_object(ob->interactive->input_to->ob, "remove_interactive");
	    free_sentence(ob->interactive->input_to);
	    if (ob->interactive->num_carry > 0)
		free_some_svalues(ob->interactive->carryover,
				  ob->interactive->num_carry);
	    ob->interactive->carryover = NULL;
	    ob->interactive->num_carry = 0;
	    ob->interactive->input_to = 0;
	}
	FREE((char *) ob->interactive);
	total_users--;
	ob->interactive = 0;
	all_users[i] = 0;
	free_object(ob, "remove_interactive");
	command_giver = save_command_giver;
	return;
    }
    fprintf(stderr, "remove_interactive: could not find and remove user %s\n",
	    ob->name);
    abort();
}				/* remove_interactive() */

static int call_function_interactive P2(struct interactive *, i, char *, str)
{
    struct object *ob;
    struct funp *funp;
    char *function;
    struct svalue *args;
    struct sentence *sent;
    int num_arg;

    i->iflags &= ~NOESC;
    if (!(sent = i->input_to))
	return (0);

    /*
     * Special feature: input_to() has been called to setup a call to a
     * function.
     */
    if (sent->ob->flags & O_DESTRUCTED) {
	/* Sorry, the object has selfdestructed ! */
	free_object(sent->ob, "call_function_interactive");
	free_sentence(sent);
	i->input_to = 0;
	if (i->num_carry)
	    free_some_svalues(i->carryover, i->num_carry);
	i->carryover = NULL;
	i->num_carry = 0;
	return (0);
    }
    /*
     * We must all references to input_to fields before the call to apply(),
     * because someone might want to set up a new input_to().
     */
    free_object(sent->ob, "call_function_interactive");
    /* we put the function on the stack in case of an error */
    sp++;
    if (sent->flags & V_FUNCTION) {
      function = 0;
      sp->type = T_FUNCTION;
      sp->u.fp = funp = sent->function.f;
      funp->ref++;
    } else {
      sp->type = T_STRING;
      sp->subtype = STRING_SHARED;
      sp->u.string = function = sent->function.s;
      ref_string(function);
    }
    ob = sent->ob;
    free_sentence(sent);

    /*
     * If we have args, we have to copy them, so the svalues on the
     * interactive struct can be FREEd
     */
    num_arg = i->num_carry;
    if (num_arg) {
	args = i->carryover;
	i->num_carry = 0;
	i->carryover = NULL;
    } else
	args = NULL;

    i->input_to = 0;
    if (i->iflags & SINGLE_CHAR) {
	/*
	 * clear single character mode
	 */
	i->iflags &= ~SINGLE_CHAR;
	add_message(telnet_no_single);
    }

    push_constant_string(str);
    /*
     * If we have args, we have to push them onto the stack in the order they
     * were in when we got them.  They will be popped off by the called
     * function.
     */
    if (args) {
	transfer_push_some_svalues(args, num_arg);
	FREE(args);
    }
    /* current_object no longer set */
    if (function)
       (void) apply(function, ob, num_arg + 1, ORIGIN_DRIVER);
    else
       call_function_pointer(funp, num_arg + 1);

    pop_stack();		/* remove `function' from stack */

    return (1);
}				/* call_function_interactive() */

int set_call P3(struct object *, ob, struct sentence *, sent, int, flags)
{
    struct object *save_command_giver = command_giver;

    if (ob == 0 || sent == 0)
	return (0);
    if (ob->interactive == 0 || ob->interactive->input_to)
	return (0);
    ob->interactive->input_to = sent;
    ob->interactive->iflags |= (flags & (I_NOECHO | I_NOESC | I_SINGLE_CHAR));
    command_giver = ob;
    if (flags & I_NOECHO)
	add_message(telnet_yes_echo);
    if (flags & I_SINGLE_CHAR)
	add_message(telnet_yes_single);
    command_giver = save_command_giver;
    return (1);
}				/* set_call() */

void set_prompt P1(char *, str)
{
    if (command_giver && command_giver->interactive) {
	command_giver->interactive->prompt = str;
    }
}				/* set_prompt() */

/*
 * Print the prompt, but only if input_to not is disabled.
 */
static void print_prompt()
{
    DEBUG_CHECK(command_giver == 0, "no command_giver in write_prompt.\n");
    if (command_giver->interactive->input_to == 0) {
	/* give user object a chance to write its own prompt */
	if (!(command_giver->interactive->iflags & HAS_WRITE_PROMPT))
	    tell_object(command_giver, command_giver->interactive->prompt);
#ifdef OLD_ED
	else if (command_giver->interactive && command_giver->interactive->ed_buffer)
	    tell_object(command_giver, command_giver->interactive->prompt);
#endif
	else if (!(command_giver->flags & O_DESTRUCTED) &&
		 !apply(APPLY_WRITE_PROMPT, command_giver, 0, ORIGIN_DRIVER)) {
	    if (command_giver->interactive) {
		command_giver->interactive->iflags &= ~HAS_WRITE_PROMPT;
		tell_object(command_giver, command_giver->interactive->prompt);
	    }
	}
    }
    /*
     * Put the IAC GA thing in here... Moved from before writing the prompt;
     * vt src says it's a terminator. Should it be inside the no-input_to
     * case? We'll see, I guess.
     */
    if (command_giver->interactive->iflags & USING_TELNET)
	add_message(telnet_ga);
}				/* print_prompt() */

/*
 * Let object 'me' snoop object 'you'. If 'you' is 0, then turn off
 * snooping.
 *
 * This routine is almost identical to the old set_snoop. The main
 * difference is that the routine writes nothing to user directly,
 * all such communication is taken care of by the mudlib. It communicates
 * with master.c in order to find out if the operation is permissble or
 * not. The old routine let everyone snoop anyone. This routine also returns
 * 0 or 1 depending on success.
 */
int new_set_snoop P2(struct object *, me, struct object *, you)
{
    struct interactive *on, *by, *tmp;

    /*
     * Stop if people managed to quit before we got this far.
     */
    if (me->flags & O_DESTRUCTED)
	return (0);
    if (you && (you->flags & O_DESTRUCTED))
	return (0);
    /*
     * Find the snooper && snoopee.
     */
    if (!me->interactive)
	error("First argument of snoop() is not interactive!\n");

    by = me->interactive;
    
    if (you) {
	if (!you->interactive)
	    error("Second argument of snoop() is not interactive!\n");
	on = you->interactive;
    } else {
	/*
	 * Stop snoop.
	 */
	if (by->snoop_on) {
	    by->snoop_on->snoop_by = 0;
	    by->snoop_on = 0;
	}
	return 1;
    }

    /*
     * Protect against snooping loops.
     */
    for (tmp = on; tmp; tmp = tmp->snoop_on) {
	if (tmp == by)
	    return (0);
    }
    
    /*
     * Terminate previous snoop, if any.
     */
    if (by->snoop_on) {
	by->snoop_on->snoop_by = 0;
	by->snoop_on = 0;
    }
    if (on->snoop_by) {
	on->snoop_by->snoop_on = 0;
	on->snoop_by = 0;
    }
    on->snoop_by = by;
    by->snoop_on = on;
    return (1);
}				/* set_new_snoop() */

/*
 * Bit of a misnomer now.  But I can't be bothered changeing the
 * name.  This will handle backspace resolution amongst other things,
 * (Pinkfish change)
 */
static void telnet_neg P2(char *, to, char *, from)
{
    int ch;
    char *first = to;

    while (1) {
	ch = (*from++ & 0xff);
	switch (ch) {
	case '\b':		/* Backspace */
	case 0x7f:		/* Delete */
	    if (to <= first)
		continue;
	    to -= 1;
	    continue;
	default:
	    if (ch & 0x80) {
		continue;
	    }
	    *to++ = ch;
	    if (ch == 0)
		return;
	    continue;
	}			/* switch() */
    }				/* while() */
}				/* telnet_neg() */

#ifndef OS2
static void query_addr_name P1(struct object *, ob)
{
    static char buf[100];
    static char *dbuf = &buf[sizeof(int) + sizeof(int) + sizeof(int)];
    int msglen;
    int msgtype;

    sprintf(dbuf, "%s", query_ip_number(ob));
    msglen = sizeof(int) + strlen(dbuf) +1;

    msgtype = DATALEN;
    memcpy(buf, (char *) &msgtype, sizeof(msgtype));
    memcpy(&buf[sizeof(int)], (char *) &msglen, sizeof(msglen));

    msgtype = NAMEBYIP;
    memcpy(&buf[sizeof(int) + sizeof(int)], (char *) &msgtype, sizeof(msgtype));
    debug(512, ("query_addr_name: sent address server %s\n", dbuf));

    if (write(addr_server_fd, buf, msglen + sizeof(int) + sizeof(int)) == -1) {
	switch (errno) {
	case EBADF:
	    fprintf(stderr, "Address server has closed connection.\n");
	    addr_server_fd = -1;
	    break;
	default:
	    perror("query_addr_name: write");
	    break;
	}
    }
}				/* query_addr_name() */

#define IPSIZE 200
static struct ipnumberentry {
    char *name, *call_back;
    struct object *ob_to_call;
}             ipnumbertable[IPSIZE];

/*
 * Does a call back on the current_object with the function call_back.
 */
int query_addr_number P2(char *, name, char *, call_back)
{
    static char buf[100];
    static char *dbuf = &buf[sizeof(int) + sizeof(int) + sizeof(int)];
    int msglen;
    int msgtype;

    if ((addr_server_fd < 0) || (strlen(name) >=
		  100 - (sizeof(msgtype) + sizeof(msglen) + sizeof(int)))) {
	push_constant_string(name);
	push_null();
	apply(call_back, current_object, 2, ORIGIN_DRIVER);
	return 0;
    }
    strcpy(dbuf, name);
    msglen = sizeof(int) + strlen(name) +1;

    msgtype = DATALEN;
    memcpy(buf, (char *) &msgtype, sizeof(msgtype));
    memcpy(&buf[sizeof(int)], (char *) &msglen, sizeof(msglen));

    msgtype = (name[0] >= '0' && name[0] <= '9') ? NAMEBYIP : IPBYNAME;
    memcpy(&buf[sizeof(int) + sizeof(int)], (char *) &msgtype, sizeof(msgtype));

    debug(512, ("query_addr_number: sent address server %s\n", dbuf));

    if (write(addr_server_fd, buf, msglen + sizeof(int) + sizeof(int)) == -1) {
	switch (errno) {
	case EBADF:
	    fprintf(stderr, "Address server has closed connection.\n");
	    addr_server_fd = -1;
	    break;
	default:
	    perror("query_addr_name: write");
	    break;
	}
	push_constant_string(name);
	push_null();
	apply(call_back, current_object, 2, ORIGIN_DRIVER);
	return 0;
    } else {
	int i;

/* We put ourselves into the pending name lookup entry table */
/* Find the first free entry */
	for (i = 0; i < IPSIZE && ipnumbertable[i].name; i++)
	    ;
	if (i == IPSIZE) {
/* We need to error...  */
	    push_constant_string(name);
	    push_null();
	    apply(call_back, current_object, 2, ORIGIN_DRIVER);
	    return 0;
	}
/* Create our entry... */
	ipnumbertable[i].name = make_shared_string(name);
	ipnumbertable[i].call_back = make_shared_string(call_back);
	ipnumbertable[i].ob_to_call = current_object;
	add_ref(current_object, "query_addr_number: ");
	return i + 1;
    }
}				/* query_addr_number() */

static void got_addr_number P2(char *, number, char *, name)
{
    int i;
    char *theName, *theNumber;

    /* First remove all the dested ones... */
    for (i = 0; i < IPSIZE; i++)
	if (ipnumbertable[i].name
	    && ipnumbertable[i].ob_to_call->flags & O_DESTRUCTED) {
	    free_string(ipnumbertable[i].call_back);
	    free_string(ipnumbertable[i].name);
	    free_object(ipnumbertable[i].ob_to_call, "got_addr_number: ");
	    ipnumbertable[i].name = NULL;
	}
    for (i = 0; i < IPSIZE; i++) {
	if (ipnumbertable[i].name && strcmp(name, ipnumbertable[i].name)== 0) {
	    /* Found one, do the call back... */
	    theName = ipnumbertable[i].name;
	    theNumber = number;
	    
	    if (isdigit(theName[0])) {
		char *tmp;
		
		tmp = theName;
		theName = theNumber;
		theNumber = tmp;
	    }
	    if (strcmp(theName, "0")) {
		push_string(theName, STRING_SHARED);
	    } else {
		push_null();
	    }
	    if (strcmp(number, "0")) {
		push_string(theNumber, STRING_SHARED);
	    } else {
		push_null();
	    }
	    push_number(i + 1);
	    safe_apply(ipnumbertable[i].call_back, ipnumbertable[i].ob_to_call,
		       3, ORIGIN_DRIVER);
	    free_string(ipnumbertable[i].call_back);
	    free_string(ipnumbertable[i].name);
	    free_object(ipnumbertable[i].ob_to_call, "got_addr_number: ");
	    ipnumbertable[i].name = NULL;
	}
    }
}				/* got_addr_number() */

#undef IPSIZE
#define IPSIZE 200
static struct ipentry {
    long addr;
    char *name;
}       iptable[IPSIZE];
static int ipcur;

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_iptable() {
    int i;

    for (i=0; i < IPSIZE; i++)
	if (iptable[i].name)
	    EXTRA_REF(BLOCK(iptable[i].name))++;
}
#endif

char *query_ip_name P1(struct object *, ob)
{
    int i;

    if (ob == 0)
	ob = command_giver;
    if (!ob || ob->interactive == 0)
	return ((char *) NULL);
    for (i = 0; i < IPSIZE; i++) {
	if (iptable[i].addr == ob->interactive->addr.sin_addr.s_addr &&
	    iptable[i].name)
	    return (iptable[i].name);
    }
    return (inet_ntoa(ob->interactive->addr.sin_addr));
}

static void add_ip_entry P2(long, addr, char *, name)
{
    int i;

    for (i = 0; i < IPSIZE; i++) {
	if (iptable[i].addr == addr)
	    return;
    }
    iptable[ipcur].addr = addr;
    if (iptable[ipcur].name)
	free_string(iptable[ipcur].name);
    iptable[ipcur].name = make_shared_string(name);
    ipcur = (ipcur + 1) % IPSIZE;
}

char *query_ip_number P1(struct object *, ob)
{
    if (ob == 0)
	ob = command_giver;
    if (!ob || ob->interactive == 0)
	return ((char *) NULL);
    return (inet_ntoa(ob->interactive->addr.sin_addr));
}

#ifndef INET_NTOA_OK
/*
 * Note: if the address string is "a.b.c.d" the address number is
 *       a * 256^3 + b * 256^2 + c * 256 + d
 */
char *inet_ntoa P1(struct in_addr, ad)
{
    u_long s_ad;
    int a, b, c, d;
    static char addr[20];	/* 16 + 1 should be enough */

    s_ad = ad.s_addr;
    d = s_ad % 256;
    s_ad /= 256;
    c = s_ad % 256;
    s_ad /= 256;
    b = s_ad % 256;
    a = s_ad / 256;
    sprintf(addr, "%d.%d.%d.%d", a, b, c, d);
    return (addr);
}
#endif				/* INET_NTOA_OK */

#else
/* OS2 */
char *query_ip_name P1(struct object *, ob)
{
    return "ip.name";
}				/* query_ip_name() */

char *query_ip_number P1(struct object *, ob)
{
    return "42.42.42.42";
}				/* query_ip_number() */

int query_addr_number P2(char *, name, char *, call_back)
{
    push_constant_string(name);
    push_null();
    apply(call_back, current_object, 2, ORIGIN_DRIVER);
    return 0;
}				/* query_addr_number() */
#endif				/* OS2 */

char *query_host_name()
{
    static char name[40];

    gethostname(name, sizeof(name));
    name[sizeof(name) - 1] = '\0';	/* Just to make sure */
    return (name);
}				/* query_host_name() */

struct object *query_snoop P1(struct object *, ob)
{
    if (!ob->interactive || (ob->interactive->snoop_by == 0))
	return (0);
    return (ob->interactive->snoop_by->ob);
}				/* query_snoop() */

struct object *query_snooping P1(struct object *, ob)
{
    if (!ob->interactive || (ob->interactive->snoop_on == 0))
	return (0);
    return (ob->interactive->snoop_on->ob);
}				/* query_snooping() */

int query_idle P1(struct object *, ob)
{
    if (!ob->interactive)
	error("query_idle() of non-interactive object.\n");
    return (current_time - ob->interactive->last_time);
}				/* query_idle() */

#ifndef NO_ADD_ACTION
void notify_no_command()
{
    union string_or_func p;
    struct svalue *v;

    if (!command_giver || !command_giver->interactive)
	return;
    p = command_giver->interactive->default_err_message;
    if (command_giver->interactive->iflags & NOTIFY_FAIL_FUNC) {
	v = call_function_pointer(p.f, 0);
	free_funp(p.f);
	if (v && v->type == T_STRING) {
	    tell_object(command_giver, v->u.string);
	}
	command_giver->interactive->iflags &= ~NOTIFY_FAIL_FUNC;
	command_giver->interactive->default_err_message.s = 0;
    } else {
	if (p.s) {
	    tell_object(command_giver, p.s);
	    free_string(p.s);
	    command_giver->interactive->default_err_message.s = 0;
	} else {
	    tell_object(command_giver, default_fail_message);
	    tell_object(command_giver, "\n");
	}
    }
}				/* notify_no_command() */

static void clear_notify()
{
    union string_or_func dem;

    if (!command_giver->interactive)
	return;
    dem = command_giver->interactive->default_err_message;
    if (command_giver->interactive->iflags & NOTIFY_FAIL_FUNC) {
	free_funp(dem.f);
	command_giver->interactive->iflags &= ~NOTIFY_FAIL_FUNC;
    }
    else if (dem.s)
	free_string(dem.s);
    command_giver->interactive->default_err_message.s = 0;
}				/* clear_notify() */

void set_notify_fail_message P1(char *, str)
{
    if (!command_giver || !command_giver->interactive)
	return;
    clear_notify();
    command_giver->interactive->default_err_message.s = make_shared_string(str);
}				/* set_notify_fail_message() */

void set_notify_fail_function P1(struct funp *, fp)
{
    if (!command_giver || !command_giver->interactive)
	return;
    clear_notify();
    command_giver->interactive->iflags |= NOTIFY_FAIL_FUNC;
    command_giver->interactive->default_err_message.f = fp;
    fp->ref++;
}				/* set_notify_fail_message() */
#endif /* NO_ADD_ACTION */

int replace_interactive P2(struct object *, ob, struct object *, obfrom)
{
    /* fprintf(stderr,"DEBUG: %s,%s\n",ob->name,obfrom->name); */
    if (ob->interactive) {
	error("Bad argument1 to exec()\n");
    }
    if (!obfrom->interactive) {
	error("Bad argument2 to exec()\n");
    }
    if ((ob->flags & O_HIDDEN) != (obfrom->flags & O_HIDDEN)) {
	if (ob->flags & O_HIDDEN) {
	    num_hidden++;
	} else {
	    num_hidden--;
	}
    }
    ob->interactive = obfrom->interactive;
    /*
     * assume the existance of write_prompt and process_input in user.c until
     * proven wrong (after trying to call them).
     */
    ob->interactive->iflags |= (HAS_WRITE_PROMPT | HAS_PROCESS_INPUT);
    obfrom->interactive = 0;
    ob->interactive->ob = ob;
    ob->flags |= O_ONCE_INTERACTIVE;
    obfrom->flags &= ~O_ONCE_INTERACTIVE;
    add_ref(ob, "exec");
    free_object(obfrom, "exec");
    if (obfrom == command_giver) {
	command_giver = ob;
    }
    return (1);
}				/* replace_interactive() */

#ifdef DEBUG
/*
 * This is used for debugging reference counts.
 */
void update_ref_counts_for_users()
{
    int i;

    for (i = 0; i < MAX_USERS; i++) {
	if (all_users[i] == 0)
	    continue;
	all_users[i]->ob->extra_ref++;
	if (all_users[i]->input_to)
	    all_users[i]->input_to->ob->extra_ref++;
    }
}				/* update_ref_counts_for_users() */
#endif				/* DEBUG */
