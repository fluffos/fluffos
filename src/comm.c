/*
 *  comm.c -- communications functions and more.
 *            Dwayne Fontenot (Jacques@TMI)
 */

#include "std.h"
#include "comm.h"
#include "main.h"
#include "socket_efuns.h"
#include "backend.h"
#include "socket_ctrl.h"
#include "debug.h"
#include "ed.h"
#include "file.h"
#include "master.h"
#include "add_action.h"
#include "eval.h"
#include "console.h"

#ifndef ENOSR
#define ENOSR 63
#endif

#ifndef ANSI_SUBSTITUTE
#define ANSI_SUBSTITUTE 0x20
#endif

#ifndef ADDRFAIL_NOTIFY
#define ADDRFAIL_NOTIFY 0
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#define TELOPT_MSSP 70
#define TELOPT_COMPRESS 85
#define TELOPT_COMPRESS2 86
#define TELOPT_MXP  91  // mud extension protocol
#define TELOPT_ZMP  93  // zenith mud protocol
#define TELOPT_GMCP 201 // something mud communication protocol, how many do we need?

#define MSSP_VAR 1
#define MSSP_VAL 2

#ifndef MAX
#define MAX(x,y) (((x)>(y))?(x):(y))
#endif

#ifndef ENV_FILLER
#define ENV_FILLER 0x1e
#endif

#define TELOPT_NEW_ENVIRON 39
#define NEW_ENV_IS               0
#define NEW_ENV_SEND             1
#define NEW_ENV_INFO             2
#define NEW_ENV_VAR              0
#define NEW_ENV_VALUE            1
#define NEW_ENV_ESC              2
#define NEW_ENV_USERVAR          3


static unsigned char telnet_break_response[] = {  28, IAC, WILL, TELOPT_TM };
static unsigned char telnet_ip_response[]    = { 127, IAC, WILL, TELOPT_TM };
static unsigned char telnet_abort_response[] = { IAC, DM };
static unsigned char telnet_do_tm_response[] = { IAC, WILL, TELOPT_TM };
static unsigned char telnet_do_naws[]        = { IAC, DO, TELOPT_NAWS };
static unsigned char telnet_dont_naws[]      = { IAC, DONT, TELOPT_NAWS };
static unsigned char telnet_do_ttype[]       = { IAC, DO, TELOPT_TTYPE };
static unsigned char telnet_term_query[]     = { IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE };
static unsigned char telnet_no_echo[]        = { IAC, WONT, TELOPT_ECHO };
static unsigned char telnet_no_single[]      = { IAC, WONT, TELOPT_SGA };
static unsigned char telnet_yes_echo[]       = { IAC, WILL, TELOPT_ECHO };
static unsigned char telnet_yes_single[]     = { IAC, WILL, TELOPT_SGA };
static unsigned char telnet_ga[]             = { IAC, GA };
static unsigned char telnet_ayt_response[]   = { '\n', '[', '-', 'Y', 'e', 's', '-', ']', ' ', '\n' };
static unsigned char telnet_line_mode[]      = { IAC, DO, TELOPT_LINEMODE };
static unsigned char telnet_lm_mode[]        = { IAC, SB, TELOPT_LINEMODE, LM_MODE, MODE_EDIT | MODE_TRAPSIG, IAC, SE };
static unsigned char telnet_char_mode[]      = { IAC, DONT, TELOPT_LINEMODE };

static unsigned char slc_default_flags[] = { SLC_NOSUPPORT, SLC_CANTCHANGE, SLC_CANTCHANGE, SLC_CANTCHANGE, SLC_CANTCHANGE, SLC_NOSUPPORT,
                                    SLC_NOSUPPORT, SLC_NOSUPPORT, SLC_CANTCHANGE, SLC_CANTCHANGE, SLC_NOSUPPORT, SLC_NOSUPPORT,
                                    SLC_NOSUPPORT, SLC_NOSUPPORT, SLC_NOSUPPORT, SLC_NOSUPPORT, SLC_NOSUPPORT, SLC_NOSUPPORT };
static unsigned char slc_default_chars[] = { 0x00, BREAK, IP, AO, AYT, 0x00, 0x00, 0x00,
                                    SUSP, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00 };
#ifdef HAVE_ZLIB
static unsigned char telnet_compress_send_request_v2[] = { IAC, WILL,
                                                  TELOPT_COMPRESS2 };

static unsigned char telnet_compress_send_request_v1[] = { IAC, WILL,
                                                  TELOPT_COMPRESS };

static unsigned char telnet_compress_v1_response[] = { IAC, SB,
                                              TELOPT_COMPRESS, WILL,
                                              SE };
static unsigned char telnet_compress_v2_response[] = { IAC, SB,
                                              TELOPT_COMPRESS2, IAC,
                                              SE };

#endif
static unsigned char telnet_do_mxp[]     = { IAC, DO, TELOPT_MXP };
static unsigned char telnet_will_mxp[]     = { IAC, SB, TELOPT_MXP, IAC, SE };
static unsigned char telnet_will_mssp[] = { IAC, WILL, TELOPT_MSSP };
static unsigned char telnet_start_mssp[] = { IAC, SB, TELOPT_MSSP };
static unsigned char telnet_mssp_value[] = {MSSP_VAR, '%', 's', MSSP_VAL, '%', 's', 0};
static unsigned char telnet_end_sub[] = {IAC, SE};
static unsigned char telnet_will_zmp[] = { IAC, WILL, TELOPT_ZMP};
static unsigned char telnet_start_zmp[] = { IAC, SB, TELOPT_ZMP};
static unsigned char telnet_do_newenv[]     = { IAC, DO, TELOPT_NEW_ENVIRON };
static unsigned char telnet_send_uservar[]     = { IAC, SB, TELOPT_NEW_ENVIRON, NEW_ENV_SEND, IAC, SE };
static unsigned char telnet_do_gmcp[] = {IAC, DO, TELOPT_GMCP};
static unsigned char telnet_start_gmcp[] = {IAC, SB, TELOPT_GMCP};
/*
 * local function prototypes.
 */
#ifdef SIGNAL_FUNC_TAKES_INT
static void sigpipe_handler (int);
#else
static void sigpipe_handler (void);
#endif

static void hname_handler (void);
static void get_user_data (interactive_t *);
static char *get_user_command (void);
static char *first_cmd_in_buf (interactive_t *);
static int cmd_in_buf (interactive_t *);
static int call_function_interactive (interactive_t *, char *);
static void print_prompt (interactive_t *);
static void query_addr_name (object_t *);
static void got_addr_number (char *, char *);
#ifdef IPV6
static void add_ip_entry (struct in6_addr, char *);
#else
static void add_ip_entry (long, char *);
#endif
static void new_user_handler (int);
static void end_compression (interactive_t *);
static void start_compression (interactive_t *);
static int send_compressed (interactive_t *ip, unsigned char* data, int length);


#ifdef NO_SNOOP
#  define handle_snoop(str, len, who)
#else
#  define handle_snoop(str, len, who) if ((who)->snooped_by) receive_snoop(str, len, who->snooped_by)

static void receive_snoop (const char *, int, object_t * ob);

#endif

/*
 * public local variables.
 */
fd_set readmask, writemask;
int num_user;
#ifdef F_SET_HIDE
int num_hidden_users = 0;       /* for the O_HIDDEN flag.  This counter must
                                 * be kept up to date at all times!  If you
                                 * modify the O_HIDDEN flag in an object,
                                 * make sure that you update this counter if
                                 * the object is interactive. */
#endif
int add_message_calls = 0;
#ifdef F_NETWORK_STATS
int inet_out_packets = 0;
int inet_out_volume = 0;
int inet_in_packets = 0;
int inet_in_volume = 0;
#ifdef PACKAGE_SOCKETS
int inet_socket_in_packets = 0;
int inet_socket_in_volume = 0;
int inet_socket_out_packets = 0;
int inet_socket_out_volume = 0;
#endif
#endif
int inet_packets = 0;
int inet_volume = 0;
interactive_t **all_users = 0;
int max_users = 0;
#ifdef HAS_CONSOLE
int has_console = -1;
#endif

/*
 * private local variables.
 */
static int addr_server_fd = -1;

static
void set_linemode (interactive_t * ip)
{
    if (ip->iflags & USING_LINEMODE) {
        add_binary_message(ip->ob, telnet_line_mode, sizeof(telnet_line_mode));
        add_binary_message(ip->ob, telnet_lm_mode, sizeof(telnet_lm_mode));
    } else {
        add_binary_message(ip->ob, telnet_no_single, sizeof(telnet_no_single));
    }
}

static
void set_charmode (interactive_t * ip)
{
    if (ip->iflags & USING_LINEMODE) {
        add_binary_message(ip->ob, telnet_char_mode, sizeof(telnet_char_mode));
    } else {
        add_binary_message(ip->ob, telnet_yes_single, sizeof(telnet_yes_single));
    }
}

#ifndef NO_SNOOP
static void
receive_snoop (const char * buf, int len, object_t * snooper)
{
    /* command giver no longer set to snooper */
#ifdef RECEIVE_SNOOP
    char *str;

    str = new_string(len, "receive_snoop");
    memcpy(str, buf, len);
    str[len] = 0;
    push_malloced_string(str);
    apply(APPLY_RECEIVE_SNOOP, snooper, 1, ORIGIN_DRIVER);
#else
    /* snoop output is now % in all cases */
    add_message(snooper, "%", 1);
    add_message(snooper, buf, len);
#endif
}
#endif

/*
 * Initialize new user connection socket.
 */
void init_user_conn()
{
#ifdef IPV6
	struct sockaddr_in6 sin;
#else
    struct sockaddr_in sin;
#endif
    memset(&sin, 0, sizeof(sin));
    socklen_t sin_len;
    int optval;
    int i;
    int have_fd6;
    int fd6_which = -1;

    /* Check for fd #6 open as a valid socket */
    optval = 1;
    have_fd6 = (setsockopt(6, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval)) == 0);

    for (i=0; i < 5; i++) {
#ifdef F_NETWORK_STATS
        external_port[i].in_packets = 0;
        external_port[i].in_volume = 0;
        external_port[i].out_packets = 0;
        external_port[i].out_volume = 0;
#endif
        if (!external_port[i].port) {
#if defined(FD6_KIND) && defined(FD6_PORT)
            if (!have_fd6) continue;
            fd6_which = i;
            have_fd6 = 0;
            if (FD6_KIND == PORT_UNDEFINED || FD6_PORT < 1) {
                debug_message("Socket passed to fd 6 ignored (support is disabled).\n");
                continue;
            }

            debug_message("Accepting connections on fd 6 (port %d).\n", FD6_PORT);
            external_port[i].kind = FD6_KIND;
            external_port[i].port = FD6_PORT;
            external_port[i].fd = 6;
#else
            continue;
#endif
        } else {
            /*
             * create socket of proper type.
             */
        	int sockflags = SOCK_STREAM;
#ifdef IPV6
            if ((external_port[i].fd = socket(PF_INET6, sockflags, 0)) == -1) {
#else
            if ((external_port[i].fd = socket(PF_INET, sockflags, 0)) == -1) {
#endif
                debug_perror("init_user_conn: socket", 0);
                exit(1);
            }

            /*
             * enable local address reuse.
             */
            optval = 1;
            if (setsockopt(external_port[i].fd, SOL_SOCKET, SO_REUSEADDR,
                           (char *) &optval, sizeof(optval)) == -1) {
                socket_perror("init_user_conn: setsockopt", 0);
                exit(2);
            }
#ifdef FD_CLOEXEC
            fcntl(external_port[i].fd, F_SETFD, FD_CLOEXEC);
#endif
            /*
             * fill in socket address information.
             */
#ifdef IPV6
            sin.sin6_family = AF_INET6;
            if(MUD_IP[0])
            	inet_pton(AF_INET6, MUD_IP, &(sin.sin6_addr));
            else
            	sin.sin6_addr = in6addr_any;
            sin.sin6_port = htons((u_short) external_port[i].port);
#else
            sin.sin_family = AF_INET;
            if (MUD_IP[0])
            	sin.sin_addr.s_addr = inet_addr(MUD_IP);
            else
            	sin.sin_addr.s_addr = INADDR_ANY;
            sin.sin_port = htons((u_short) external_port[i].port);
#endif

            /*
             * bind name to socket.
             */
            if (bind(external_port[i].fd, (struct sockaddr *) & sin,
                     sizeof(sin)) == -1) {
                socket_perror("init_user_conn: bind", 0);
                exit(3);
            }
        }
        /*
         * get socket name.
         */
        sin_len = sizeof(sin);
        if (getsockname(external_port[i].fd, (struct sockaddr *) & sin,
                        &sin_len) == -1) {
            socket_perror("init_user_conn: getsockname", 0);
            if (i != fd6_which) {
                exit(4);
            }
        }
        /*
         * set socket non-blocking,
         */
        if (set_socket_nonblocking(external_port[i].fd, 1) == -1) {
            socket_perror("init_user_conn: set_socket_nonblocking 1", 0);
            if (i != fd6_which) {
                exit(8);
            }
        }
        /*
         * listen on socket for connections.
         */
        if (listen(external_port[i].fd, 128) == -1) {
            socket_perror("init_user_conn: listen", 0);
            if (i != fd6_which) {
                exit(10);
            }
        }
    }
    if (have_fd6) {
        debug_message("No more ports available; fd #6 ignored.\n");
    }
    /*
     * register signal handler for SIGPIPE.
     */
#if defined(SIGPIPE) && defined(SIGNAL_ERROR)
#ifdef SIG_IGN
    if (signal(SIGPIPE, SIG_IGN) == SIGNAL_ERROR) {
            debug_perror("init_user_conn: signal SIGPIPE",0);
            exit(5);
        }
#else
    if (signal(SIGPIPE, sigpipe_handler) == SIGNAL_ERROR) {
        debug_perror("init_user_conn: signal SIGPIPE",0);
        exit(5);
    }
#endif
#endif
}

/*
 * Shut down new user accept file descriptor.
 */
void ipc_remove()
{
    int i;

    for (i = 0; i < 5; i++) {
        if (!external_port[i].port) continue;
        if (OS_socket_close(external_port[i].fd) == -1) {
            socket_perror("ipc_remove: close", 0);
        }
    }

    debug_message("closed external ports\n");
}

void init_addr_server (char * hostname, int addr_server_port)
{
#ifdef WIN32
    WORD wVersionRequested = MAKEWORD(1,1);
    WSADATA wsaData;
    WSAStartup(wVersionRequested, &wsaData);
#endif
#ifdef IPV6
    struct sockaddr_in6 server;
#else
    struct sockaddr_in server;
#endif
#ifndef IPV6
    struct hostent *hp;
    int addr;
#endif
    int server_fd;
    int optval;

    if (addr_server_fd >= 0)
        return;

    if (!hostname) return;
#ifdef IPV6
    /*
     * get network host data for hostname.
     */
    struct addrinfo hints, *res;
    hints.ai_family = AF_INET6;
    hints.ai_socktype = 0;
    hints.ai_protocol = 0;
#ifndef AI_V4MAPPED
    hints.ai_flags = AI_CANONNAME;
#else
    hints.ai_flags = AI_CANONNAME| AI_V4MAPPED;
#endif

    if(getaddrinfo(hostname, "1234", &hints, &res)){
    	//failed
    	socket_perror("init_addr_server: getaddrinfo", 0);
    	        return;
    }

    memcpy(&server, res->ai_addr, sizeof(server));
    freeaddrinfo(res);
    server.sin6_port = htons((u_short) addr_server_port);
    //inet_pton(AF_INET6, hostname, &(server.sin6_addr));
    //memcpy((char *) &server.sin6_addr, (char *) hp->h_addr, hp->h_length);
     /*
      * create socket of proper type.
      */
    server_fd = socket(AF_INET6, SOCK_STREAM, 0);
#else
    /*
     * get network host data for hostname.
     */
    if (hostname[0] >= '0' && hostname[0] <= '9' &&
          (addr = inet_addr(hostname)) != -1) {
        hp = gethostbyaddr((char *)&addr, sizeof(addr), AF_INET);
    } else {
        hp = gethostbyname(hostname);
    }
    if (hp == NULL) {
        socket_perror("init_addr_server: gethostbyname", 0);
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
#endif
    if (server_fd == INVALID_SOCKET) {  /* problem opening socket */
        socket_perror("init_addr_server: socket", 0);
        return;
    }
    /*
     * enable local address reuse.
     */
    optval = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &optval,
                   sizeof(optval)) == -1) {
        socket_perror("init_addr_server: setsockopt", 0);
        return;
    }
    /*
     * connect socket to server address.
     */
    if (connect(server_fd, (struct sockaddr *) & server, sizeof(server)) == -1) {
    	if(ADDRFAIL_NOTIFY){
    		if (socket_errno == ECONNREFUSED && ADDRFAIL_NOTIFY)
    			debug_message("Connection to address server (%s %d) refused.\n",
    					hostname, addr_server_port);
    		else
    			socket_perror("init_addr_server: connect", 0);
    	}
        OS_socket_close(server_fd);
        return;
    }
    addr_server_fd = server_fd;
    debug_message("Connected to address server on %s port %d\n", hostname,
            addr_server_port);
    /*
     * set socket non-blocking.
     */
    if (set_socket_nonblocking(server_fd, 1) == -1) {
        socket_perror("init_addr_server: set_socket_nonblocking 1", 0);
        return;
    }
#ifdef WIN32
        WSACleanup();
#endif
}

/*
 * If there is a shadow for this object, then the message should be
 * sent to it. But only if catch_tell() is defined. Beware that one of the
 * shadows may be the originator of the message, which means that we must
 * not send the message to that shadow, or any shadows in the linked list
 * before that shadow.
 *
 * Also note that we don't need to do this in the case of
 * INTERACTIVE_CATCH_TELL, since catch_tell() was already called
 * _instead of_ add_message(), and shadows got their chance then.
 */
#if !defined(INTERACTIVE_CATCH_TELL) && !defined(NO_SHADOWS)
#define SHADOW_CATCH_MESSAGE
#endif

#ifdef SHADOW_CATCH_MESSAGE
static int shadow_catch_message (object_t * ob, const char * str)
{
    if (!ob->shadowed)
        return 0;
    while (ob->shadowed != 0 && ob->shadowed != current_object)
        ob = ob->shadowed;
    while (ob->shadowing) {
        copy_and_push_string(str);
        if (apply(APPLY_CATCH_TELL, ob, 1, ORIGIN_DRIVER))
            /* this will work, since we know the */
            /* function is defined */
            return 1;
        ob = ob->shadowing;
    }
    return 0;
}
#endif

/*
 * Send a message to an interactive object. If that object is shadowed,
 * special handling is done.
 */
void add_message (object_t * who, const char * data, int len)
{
    interactive_t *ip;
    const char *cp;
    const char *end;
    char *trans;
    int translen;
    /*
     * if who->interactive is not valid, write message on stderr.
     * (maybe)
     */
    if (!who || (who->flags & O_DESTRUCTED) || !who->interactive ||
        (who->interactive->iflags & (NET_DEAD | CLOSING))) {
#ifdef NONINTERACTIVE_STDERR_WRITE
        putc(']', stderr);
        fwrite(data, len, 1, stderr);
#endif
        return;
    }
    ip = who->interactive;
    trans = translate(ip->trans->outgoing, data, len, &translen);
#ifdef SHADOW_CATCH_MESSAGE
    /*
     * shadow handling.
     */
    if (shadow_catch_message(who, data)) {
#ifdef SNOOP_SHADOWED
        handle_snoop(data, len, ip);
#endif
        return;
    }
#endif                          /* NO_SHADOWS */

    /*
     * write message into ip->message_buf.
     */
    end = trans + translen;
    for (cp = trans; cp < end; cp++) {
        if (ip->message_length == MESSAGE_BUF_SIZE) {
            if (!flush_message(ip)) {
                debug(connections, ("Broken connection during add_message."));
                return;
            }
            if (ip->message_length == MESSAGE_BUF_SIZE)
                break;
        }
        if ((*cp == '\n' || *cp == -1)
#ifndef NO_BUFFER_TYPE
            && ip->connection_type != PORT_BINARY
#endif
            ) {
            if (ip->message_length == (MESSAGE_BUF_SIZE - 1)) {
                if (!flush_message(ip)) {
                    debug(connections, ("Broken connection during add_message."));
                    return;
                }
                if (ip->message_length == (MESSAGE_BUF_SIZE - 1))
                    break;
            }
            ip->message_buf[ip->message_producer] = (*cp == '\n')?'\r':-1;
            ip->message_producer = (ip->message_producer + 1)
                % MESSAGE_BUF_SIZE;
            ip->message_length++;
        }
        ip->message_buf[ip->message_producer] = *cp;
        ip->message_producer = (ip->message_producer + 1) % MESSAGE_BUF_SIZE;
        ip->message_length++;
    }

    handle_snoop(data, len, ip);

#ifdef FLUSH_OUTPUT_IMMEDIATELY
    flush_message(ip);
#endif

    add_message_calls++;
}                               /* add_message() */

/* WARNING: this can only handle results < LARGEST_PRINTABLE_STRING in size */
void add_vmessage (object_t *who, const char *format, ...)
{
    int len;
    interactive_t *ip;
    char *cp, new_string_data[LARGEST_PRINTABLE_STRING + 1];
    va_list args;

    V_START(args, format);
    V_VAR(object_t *, who, args);
    V_VAR(char *, format, args);
    /*
     * if who->interactive is not valid, write message on stderr.
     * (maybe)
     */
    if (!who || (who->flags & O_DESTRUCTED) || !who->interactive ||
        (who->interactive->iflags & (NET_DEAD | CLOSING))) {
#ifdef NONINTERACTIVE_STDERR_WRITE
        putc(']', stderr);
        vfprintf(stderr, format, args);
#endif
        va_end(args);
        return;
    }
    ip = who->interactive;
    new_string_data[0] = '\0';

    vsnprintf(new_string_data, LARGEST_PRINTABLE_STRING, format, args);
    va_end(args);
    len = strlen(new_string_data);
#ifdef SHADOW_CATCH_MESSAGE
    /*
     * shadow handling.
     */
    if (shadow_catch_message(who, new_string_data)) {
#ifdef SNOOP_SHADOWED
        handle_snoop(new_string_data, len, ip);
#endif
        return;
    }
#endif                          /* NO_SHADOWS */

    /*
     * write message into ip->message_buf.
     */
    for (cp = new_string_data; *cp != '\0'; cp++) {
        if (ip->message_length == MESSAGE_BUF_SIZE) {
            if (!flush_message(ip)) {
                debug(connections, ("Broken connection during add_message."));
                return;
            }
            if (ip->message_length == MESSAGE_BUF_SIZE)
                break;
        }
        if (*cp == '\n') {
            if (ip->message_length == (MESSAGE_BUF_SIZE - 1)) {
                if (!flush_message(ip)) {
                    debug(connections, ("Broken connection during add_message.\n"));
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
        if (!flush_message(ip)) {
            debug(connections, ("Broken connection during add_message.\n"));
            return;
        }
    }

    handle_snoop(new_string_data, len, ip);

#ifdef FLUSH_OUTPUT_IMMEDIATELY
    flush_message(ip);
#endif

    add_message_calls++;
}                               /* add_message() */

void add_binary_message (object_t * who, const unsigned char * data, int len)
{
    interactive_t *ip;
    const unsigned char *cp, *end;

    /*
     * if who->interactive is not valid, bail
     */
    if (!who || (who->flags & O_DESTRUCTED) || !who->interactive ||
        (who->interactive->iflags & (NET_DEAD | CLOSING))) {
        return;
    }
    ip = who->interactive;

    /*
     * write message into ip->message_buf.
     */
    end = data + len;
    for (cp = data; cp < end; cp++) {
        if (ip->message_length == MESSAGE_BUF_SIZE) {
            if (!flush_message(ip)) {
                debug(connections, ("Broken connection during add_message."));
                return;
            }
            if (ip->message_length == MESSAGE_BUF_SIZE)
                break;
        }
        ip->message_buf[ip->message_producer] = *cp;
        ip->message_producer = (ip->message_producer + 1) % MESSAGE_BUF_SIZE;
        ip->message_length++;
    }

    flush_message(ip);
    add_message_calls++;
}

/*
 * Flush outgoing message buffer of current interactive object.
 */
int flush_message (interactive_t * ip)
{
    int length, num_bytes;

    /*
     * if ip is not valid, do nothing.
     */
    if (!ip || !ip->ob || !IP_VALID(ip, ip->ob) ||
        (ip->ob->flags & O_DESTRUCTED) || (ip->iflags & (NET_DEAD | CLOSING))){
      //debug(connections, ("flush_message: invalid target!\n"));
        return 0;
    }
    /*
     * write ip->message_buf[] to socket.
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
#ifdef HAVE_ZLIB
        if (ip->compressed_stream) {
          num_bytes = send_compressed(ip, (unsigned char *)ip->message_buf +
                                      ip->message_consumer,  length);
        } else {
#endif
        num_bytes = send(ip->fd, ip->message_buf + ip->message_consumer,
                         length, ip->out_of_band | MSG_NOSIGNAL);
#ifdef HAVE_ZLIB
        }
#endif
        if (!num_bytes) {
            ip->iflags |= NET_DEAD;
            return 0;
        }
        if (num_bytes == -1) {
#ifdef EWOULDBLOCK
            if (socket_errno == EWOULDBLOCK) {
              //debug(connections, ("flush_message: write: Operation would block\n"));
                return 1;
#else
            if (0) {
                ;
#endif
            } else if (socket_errno == EINTR) {
              //debug(connections, ("flush_message: write: Interrupted system call"));
                return 1;
            } else {
              //socket_perror("flush_message: write", 0);
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
#ifdef F_NETWORK_STATS
        inet_out_packets++;
        inet_out_volume += num_bytes;
        external_port[ip->external_port].out_packets++;
        external_port[ip->external_port].out_volume += num_bytes;
#endif
    }
    return 1;
}                               /* flush_message() */

static int send_mssp_val(mapping_t *map, mapping_node_t *el, void *obp){
	object_t *ob = (object_t *)obp;
	if(el->values[0].type == T_STRING && el->values[1].type == T_STRING){
		char buf[1024];
		int len = sprintf(buf, (char *)telnet_mssp_value, el->values[0].u.string, el->values[1].u.string);
		add_binary_message(ob, (unsigned char *)buf, len);
	} else if (el->values[0].type == T_STRING && el->values[1].type == T_ARRAY && el->values[1].u.arr->size > 0 && el->values[1].u.arr->item[0].type == T_STRING){
		char buf[10240];
		int len = sprintf(buf, (char *)telnet_mssp_value, el->values[0].u.string, el->values[1].u.arr->item[0].u.string);
		add_binary_message(ob, (unsigned char *)buf, len);
		array_t *ar = el->values[1].u.arr;
		int i;
		unsigned char val = MSSP_VAL;
		for(i=1; i < ar->size; i++){
			if(ar->item[i].type == T_STRING){
				add_binary_message(ob, &val, 1);
				add_binary_message(ob, (const unsigned char *)ar->item[i].u.string, strlen(ar->item[i].u.string));
			}
		}

	}
	return 0;
}

static void copy_chars (interactive_t * ip, char * from, int num_bytes)
{
    int i, start, x;
    unsigned char dont_response[3] = { IAC, DONT, 0 };
    unsigned char wont_response[3] = { IAC, WONT, 0 };

    start = ip->text_end;
    for (i = 0;  i < num_bytes;  i++) {
        switch (ip->state) {
            case TS_DATA:
                switch ((unsigned char)from[i]) {
                    case IAC:
                        ip->state = TS_IAC;
                        break;

#if defined(NO_ANSI) && defined(STRIP_BEFORE_PROCESS_INPUT)
                    case 0x1b:
                        ip->text[ip->text_end++] = ANSI_SUBSTITUTE;
                        break;
#endif

                    case 0x08:
                    case 0x7f:
                        if (ip->iflags & SINGLE_CHAR)
                            ip->text[ip->text_end++] = from[i];
                        else {
                            if (ip->text_end > 0)
                                ip->text_end--;
                        }
                        break;

                    default:
                        ip->text[ip->text_end++] = from[i];
                        break;
                }
                break;

            case TS_IAC:
                switch ((unsigned char)from[i]) {
                    case IAC:
                        ip->state = TS_DATA;
                        ip->text[ip->text_end++] = from[i];
                        break;

                    case WILL:
                        ip->state = TS_WILL;
                        break;

                    case WONT:
                        ip->state = TS_WONT;
                        break;

                    case DO:
                        ip->state = TS_DO;
                        break;

                    case DONT:
                        ip->state = TS_DONT;
                        break;

                    case SB:
                        ip->state = TS_SB;
                        ip->sb_pos = 0;
                        break;

                    case BREAK:
                        add_binary_message(ip->ob, telnet_break_response, sizeof(telnet_break_response));
                        break;

                    case IP:    /* interrupt process */
                        add_binary_message(ip->ob, telnet_ip_response, sizeof(telnet_ip_response));
                        break;

                    case AYT:   /* are you there?  you bet */
                        add_binary_message(ip->ob, telnet_ayt_response, sizeof(telnet_ayt_response));
                        break;

                    case AO:    /* abort output */
                        flush_message(ip);
                        ip->out_of_band = MSG_OOB;
                        add_binary_message(ip->ob, telnet_abort_response, sizeof(telnet_abort_response));
                        break;

                    default:
                        ip->state = TS_DATA;
                        break;
                }
                break;

            case TS_WILL:
                ip->iflags |= USING_TELNET;
                switch ((unsigned char)from[i]) {
                    case TELOPT_TTYPE:
                        add_binary_message(ip->ob, telnet_term_query, sizeof(telnet_term_query));
                        break;

                    case TELOPT_LINEMODE:
                        /* Do linemode and set the mode: EDIT + TRAPSIG */
                        ip->iflags |= USING_LINEMODE;
                        set_linemode(ip);
                        break;

                    case TELOPT_ECHO:
                    case TELOPT_NAWS:
                        /* do nothing, but don't send a dont response */
                        break;

                    case TELOPT_MXP :
                        /* Mxp is enabled, tell the mudlib about it. */
                        apply(APPLY_MXP_ENABLE, ip->ob, 0, ORIGIN_DRIVER);
                        ip->iflags |= USING_MXP;
                        break;
                    case TELOPT_GMCP:
                    	apply(APPLY_GMCP_ENABLE, ip->ob, 0, ORIGIN_DRIVER);
                    	ip->iflags |= USING_GMCP;
                    	break;
                    case TELOPT_NEW_ENVIRON :
                        add_binary_message(ip->ob, telnet_send_uservar, sizeof(telnet_send_uservar));
                        break;

                    default:
                        dont_response[2] = from[i];
                        add_binary_message(ip->ob, dont_response, sizeof(dont_response));
                        break;
                }
                ip->state = TS_DATA;
                break;

            case TS_WONT:
                ip->iflags |= USING_TELNET;
                switch ((unsigned char)from[i]) {
                    case TELOPT_LINEMODE:
                        /* If we're in single char mode, we just requested for
                         * linemode to be disabled, so don't remove our flag.
                         */
                        if (!(ip->iflags & SINGLE_CHAR))
                            ip->iflags &= ~USING_LINEMODE;
                        break;
                }
                ip->state = TS_DATA;
                break;

            case TS_DO:
                switch ((unsigned char)from[i]) {
                    case TELOPT_TM:
                        add_binary_message(ip->ob, telnet_do_tm_response, sizeof(telnet_do_tm_response));
                        break;

                    case TELOPT_SGA:
                        if (ip->iflags & USING_LINEMODE) {
                            ip->iflags |= SUPPRESS_GA;
                            add_binary_message(ip->ob, telnet_yes_single, sizeof(telnet_yes_single));
                        } else {
                            if (ip->iflags & SINGLE_CHAR)
                                add_binary_message(ip->ob, telnet_yes_single, sizeof(telnet_yes_single));
                            else
                                add_binary_message(ip->ob, telnet_no_single, sizeof(telnet_no_single));
                        }
                        break;

                    case TELOPT_ECHO:
                        /* do nothing, but don't send a wont response */
                        break;
                    case TELOPT_MSSP:
                    {
                    	add_binary_message(ip->ob, telnet_start_mssp, sizeof(telnet_start_mssp));
                    	svalue_t *res = apply_master_ob(APPLY_GET_MUD_STATS, 0);
                    	mapping_t *map;
                    	if(res <= 0 || res->type != T_MAPPING) {
                    		map = allocate_mapping(0);
                    		free_svalue(&apply_ret_value, "telnet neg");
                    		apply_ret_value.type = T_MAPPING;
                    		apply_ret_value.u.map = map;
                    	} else
                    		map = res->u.map;
                    	//ok, so we have a mapping, first make sure we send the required values
                    	char *tmp = findstring("NAME");
                    	if(tmp){
                    		svalue_t *name = find_string_in_mapping(map, tmp);
                    		if(!name || name->type != T_STRING)
                    			tmp = 0;
                    	}
                    	if(!tmp){
                    		char buf[1024];
                    		int len = sprintf(buf, (char *)telnet_mssp_value, "NAME", MUD_NAME);
                    		add_binary_message(ip->ob, (unsigned char *)buf, len);
                    	}
                    	tmp = findstring("PLAYERS");
                    	if(tmp){
                    		svalue_t *players = find_string_in_mapping(map, tmp);
                    		if(!players || players->type != T_STRING)
                    			tmp = 0;
                    	}
                    	if(!tmp){
                    		char buf[1024];
                    		char num[5];
                    		sprintf(num, "%d", num_user);
                    		int len = sprintf(buf, (char *)telnet_mssp_value, "PLAYERS", num);
                    		add_binary_message(ip->ob, (unsigned char *)buf, len);
                    	}
                    	tmp = findstring("UPTIME");
                    	if(tmp){
                    		svalue_t *upt = find_string_in_mapping(map, tmp);
                    		if(!upt || upt->type != T_STRING)
                    			tmp = 0;
                    	}
                    	if(!tmp){
                    		char buf[1024];
                    		char num[20];

                    		sprintf(num, "%ld", boot_time);
                    		int len = sprintf(buf, (char *)telnet_mssp_value, "UPTIME", num);
                    		add_binary_message(ip->ob, (unsigned char *)buf, len);
                    	}
                    	//now send the rest
                    	mapTraverse(map, send_mssp_val, ip->ob);
                    	add_binary_message(ip->ob, telnet_end_sub, sizeof(telnet_end_sub));
                    }
#ifdef HAVE_ZLIB
                    case TELOPT_COMPRESS :
                      if(!ip->compressed_stream){
                    	  add_binary_message(ip->ob, telnet_compress_v1_response,
                    			  sizeof(telnet_compress_v1_response));
                    	  start_compression(ip);
                      }
                      break;
                    case TELOPT_COMPRESS2 :
                      if(!ip->compressed_stream){
                    	  add_binary_message(ip->ob, telnet_compress_v2_response,
                    			  sizeof(telnet_compress_v2_response));
                    	  start_compression(ip);
                      }
                      break;
#endif
                    case TELOPT_ZMP :
                    	ip->iflags |= USING_ZMP;
                    	break;
                    default:
                        wont_response[2] = from[i];
                        add_binary_message(ip->ob, wont_response, sizeof(wont_response));
                        break;
                }
                ip->state = TS_DATA;
                break;

            case TS_DONT:
                switch ((unsigned char)from[i]) {
                    case TELOPT_SGA:
                        if (ip->iflags & USING_LINEMODE) {
                            ip->iflags &= ~SUPPRESS_GA;
                            add_binary_message(ip->ob, telnet_no_single, sizeof(telnet_no_single));
                        }
                        break;
#ifdef HAVE_ZLIB
                    case TELOPT_COMPRESS2:
                        // If we are told not to use v2, then try v1.
                        add_binary_message(ip->ob, telnet_compress_send_request_v1,
                                    sizeof(telnet_compress_send_request_v1));
                        break;
#endif
                }
                ip->state = TS_DATA;
                break;

            case TS_SB:
                if ((unsigned char)from[i] == IAC) {
                    ip->state = TS_SB_IAC;
                    break;
                }
                if (ip->sb_pos < ip->sb_size - 1)
                    ip->sb_buf[ip->sb_pos++] = from[i];
                else if(ip->sb_size < MAX_STRING_LENGTH){
                	ip->sb_size*=2;
                	if(ip->sb_size > MAX_STRING_LENGTH)
                		ip->sb_size = MAX_STRING_LENGTH;
                	ip->sb_buf = (char *)REALLOC(ip->sb_buf, ip->sb_size);
                    if (ip->sb_pos < ip->sb_size - 1)
                        ip->sb_buf[ip->sb_pos++] = from[i];
                }
                break;

            case TS_SB_IAC:
                switch ((unsigned char)from[i]) {
                    case IAC:
                        if (ip->sb_pos < SB_SIZE - 1) {
                            ip->sb_buf[ip->sb_pos++] = from[i];
                            ip->state = TS_SB;
                        }
                        break;

                    case SE:
                        ip->state = TS_DATA;
                        ip->sb_buf[ip->sb_pos] = 0;
                        switch ((unsigned char)ip->sb_buf[0]) {

                        case TELOPT_NEW_ENVIRON :
                        {
                        	    int j, k;
                        	    char env_buf[BUF_SIZE];
                                j = 0;
                                k = 1;
                                while(ip->sb_buf[k] > -1 && k < (ip->sb_pos - 1)){
                                    k++;
                                    if(!(ip->sb_buf[k])) env_buf[j] = ENV_FILLER;
                                    if(ip->sb_buf[k] == 1) env_buf[j] = 1;
                                    if((ip->sb_buf[k] > 31)){
                                        env_buf[j] = ip->sb_buf[k];
                                    }
                                    if(env_buf[j]) j++;
                                }
                                env_buf[j] = 0;
                                copy_and_push_string(env_buf);
                                apply(APPLY_RECEIVE_ENVIRON, ip->ob, 1, ORIGIN_DRIVER);
                                break;

                        }
                            case TELOPT_LINEMODE:
                                switch ((unsigned char)ip->sb_buf[1]) {
                                    case LM_MODE:
                                        /* Don't do anything with an ACK */
                                        if (!(ip->sb_buf[2] & MODE_ACK)) {
                                            unsigned char sb_ack[] = { IAC, SB, TELOPT_LINEMODE, LM_MODE, MODE_EDIT | MODE_TRAPSIG | MODE_ACK, IAC, SE };

                                            /* Accept only EDIT and TRAPSIG && force them too */
                                            add_binary_message(ip->ob, sb_ack, sizeof(sb_ack));
                                        }
                                        break;

                                    case LM_SLC:
                                        {
                                            int slc_length = 4;
                                            unsigned char slc_response[SB_SIZE + 6] = { IAC, SB, TELOPT_LINEMODE, LM_SLC };

                                            for (x = 2;  x < ip->sb_pos;  x += 3) {
                                                /* no response for an ack */
                                                if (ip->sb_buf[x + 1] & SLC_ACK)
                                                    continue;

                                                /* If we get { 0, SLC_DEFAULT, 0 } or { 0, SLC_VARIABLE, 0 } return a list of values */
                                                /* If it's SLC_DEFAULT, reset to defaults first */
                                                if (!ip->sb_buf[x] && !ip->sb_buf[x + 2]) {
                                                    if (ip->sb_buf[x + 1] == SLC_DEFAULT || ip->sb_buf[x + 1] == SLC_VARIABLE) {
                                                        int n;

                                                        for (n = 0;  n < NSLC;  n++) {
                                                            slc_response[slc_length++] = n + 1;
                                                            if (ip->sb_buf[x + 1] == SLC_DEFAULT) {
                                                                ip->slc[n][0] = slc_default_flags[n];
                                                                ip->slc[n][1] = slc_default_chars[n];
                                                                slc_response[slc_length++] = SLC_DEFAULT;
                                                            } else {
                                                                slc_response[slc_length++] = ip->slc[n][0];
                                                            }
                                                            slc_response[slc_length++] = ip->slc[n][1];
                                                        }
                                                        break;
                                                    }
                                                }

                                                slc_response[slc_length++] = ip->sb_buf[x]--;

                                                /* If the first octet is out of range, we don't support it */
                                                /* If the default flag is not supported, we don't support it */
                                                if (ip->sb_buf[x] >= NSLC || slc_default_flags[ip->sb_buf[x]] == SLC_NOSUPPORT) {
                                                    slc_response[slc_length++] = SLC_NOSUPPORT;
                                                    slc_response[slc_length++] = ip->sb_buf[x + 2];
                                                    if ((unsigned char)ip->sb_buf[x + 2] == IAC)
                                                       slc_response[slc_length++] = IAC;
                                                    continue;
                                                }

                                                switch ((ip->sb_buf[x + 1] & SLC_LEVELBITS)) {
                                                    case SLC_NOSUPPORT:
                                                        if (slc_default_flags[ip->sb_buf[x]] == SLC_CANTCHANGE) {
                                                            slc_response[slc_length++] = SLC_CANTCHANGE;
                                                            slc_response[slc_length++] = ip->slc[ip->sb_buf[x]][1];
                                                            break;
                                                        }
                                                        slc_response[slc_length++] = SLC_ACK | SLC_NOSUPPORT;
                                                        slc_response[slc_length++] = ip->sb_buf[x + 2];
                                                        ip->slc[ip->sb_buf[x]][0] = SLC_NOSUPPORT;
                                                        ip->slc[ip->sb_buf[x]][1] = 0;
                                                        break;

                                                    case SLC_VARIABLE:
                                                        if (slc_default_flags[ip->sb_buf[x]] == SLC_CANTCHANGE) {
                                                            slc_response[slc_length++] = SLC_CANTCHANGE;
                                                            slc_response[slc_length++] = ip->slc[ip->sb_buf[x]][1];
                                                            break;
                                                        }
                                                        slc_response[slc_length++] = SLC_ACK | SLC_VARIABLE;
                                                        slc_response[slc_length++] = ip->sb_buf[x + 2];
                                                        ip->slc[ip->sb_buf[x]][0] = ip->sb_buf[x + 1];
                                                        ip->slc[ip->sb_buf[x]][1] = ip->sb_buf[x + 2];
                                                        break;

                                                    case SLC_CANTCHANGE:
                                                        slc_response[slc_length++] = SLC_ACK | SLC_CANTCHANGE;
                                                        slc_response[slc_length++] = ip->sb_buf[x + 2];
                                                        ip->slc[ip->sb_buf[x]][0] = ip->sb_buf[x + 1];
                                                        ip->slc[ip->sb_buf[x]][1] = ip->sb_buf[x + 2];
                                                        break;

                                                    case SLC_DEFAULT:
                                                        slc_response[slc_length++] = slc_default_flags[ip->sb_buf[x]];
                                                        slc_response[slc_length++] = slc_default_flags[ip->sb_buf[x]];
                                                        ip->slc[ip->sb_buf[x]][0] = slc_default_flags[ip->sb_buf[x]];
                                                        ip->slc[ip->sb_buf[x]][1] = slc_default_chars[ip->sb_buf[x]];
                                                        break;

                                                    default:
                                                        slc_response[slc_length++] = SLC_NOSUPPORT;
                                                        slc_response[slc_length++] = ip->sb_buf[x + 2];
                                                        if ((unsigned char)slc_response[slc_length - 1] == IAC)
                                                           slc_response[slc_length++] = IAC;
                                                        break;
                                                }
                                            }

                                            if (slc_length > 4) {
                                                /* send our response */
                                                slc_response[slc_length++] = IAC;
                                                slc_response[slc_length++] = SE;
                                                add_binary_message(ip->ob, slc_response, slc_length);
                                            }
                                        }
                                        break;

                                    case DO:
                                        {
                                            unsigned char sb_wont[] = { IAC, SB, TELOPT_LINEMODE, WONT, 0, IAC, SE };

                                            /* send back IAC SB TELOPT_LINEMODE WONT x IAC SE */
                                            sb_wont[4] = ip->sb_buf[2];
                                            add_binary_message(ip->ob, sb_wont, sizeof(sb_wont));
                                        }
                                        break;

                                    case WILL:
                                        {
                                            unsigned char sb_dont[] = { IAC, SB, TELOPT_LINEMODE, DONT, 0, IAC, SE };

                                            /* send back IAC SB TELOPT_LINEMODE DONT x IAC SE */
                                            sb_dont[4] = ip->sb_buf[2];
                                            add_binary_message(ip->ob, sb_dont, sizeof(sb_dont));
                                        }
                                        break;
                                }
                                break;

                            case TELOPT_NAWS:
                                if (ip->sb_pos >= 5) {
                                    push_number(((unsigned char)ip->sb_buf[1] << 8) | (unsigned char)ip->sb_buf[2]);
                                    push_number(((unsigned char)ip->sb_buf[3] << 8) | (unsigned char)ip->sb_buf[4]);
                                    apply(APPLY_WINDOW_SIZE, ip->ob, 2, ORIGIN_DRIVER);
                                }
                                break;

                            case TELOPT_TTYPE:
                                if (!ip->sb_buf[1]) {
                                    copy_and_push_string(ip->sb_buf + 2);
                                    apply(APPLY_TERMINAL_TYPE, ip->ob, 1, ORIGIN_DRIVER);
                                }
                                break;
                            case TELOPT_ZMP:
                            {
                            	array_t *arr = allocate_array(max_array_size);
                            	ip->sb_buf = (char *)REALLOC(ip->sb_buf, MAX(ip->sb_pos + 2, SB_SIZE));
                            	ip->sb_size = MAX(ip->sb_pos + 2, SB_SIZE);
                            	ip->sb_buf[ip->sb_pos] = 0;
                            	copy_and_push_string(ip->sb_buf+1);
                            	int off=0;
                            	int aro = 0;
                            	while(1){
                            		off += strlen(ip->sb_buf+1+off)+2;
                            		if(off >= ip->sb_pos-1)
                            			break;
                            		arr->item[aro].u.string = string_copy(&ip->sb_buf[off], "ZMP");
                            		arr->item[aro].type = T_STRING;
                            		arr->item[aro++].subtype = STRING_MALLOC;
                            	}
                            	arr = resize_array(arr, aro);
                            	push_refed_array(arr);
                            	apply(APPLY_ZMP, ip->ob, 2, ORIGIN_DRIVER);

                            }
							break;
                            case TELOPT_GMCP:
                            	ip->sb_buf[ip->sb_pos] = 0;
                            	copy_and_push_string(&ip->sb_buf[1]);
                            	apply(APPLY_GMCP, ip->ob, 1, ORIGIN_DRIVER);
                            	break;
                            default:
                                for (x = 0;  x < ip->sb_pos;  x++)
                                    ip->sb_buf[x] = (ip->sb_buf[x] ? ip->sb_buf[x] : 'I');
                                copy_and_push_string(ip->sb_buf);
                                apply(APPLY_TELNET_SUBOPTION, ip->ob, 1, ORIGIN_DRIVER);
                                break;
                        }
                        break;

                    default:
                        /*
                         * Apparently, old versions of MudOS would revert to TS_DATA here.
                         * Later versions handle the IAC, and then go back to TS_DATA mode.
                         * I don't think either is proper, but the related RFC documents
                         * aren't clear on what to do (854, 855).  It is my feeling, that
                         * the safest thing to do here is to ignore the option.
                         * -- Marius, 6-Jun-2000
                         */
                        ip->state = TS_SB;
                        break;
                }
                break;
        }
    }

    if (ip->text_end > start) {
        /* handle snooping - snooper does not see type-ahead due to
           telnet being in linemode */
        if (!(ip->iflags & NOECHO))
            handle_snoop(ip->text + start, ip->text_end - start, ip);
    }
}

/*
 * Read pending data for a user into user->interactive->text.
 * This also does telnet negotiation.
 */
static void get_user_data (interactive_t * ip)
{
    int  num_bytes, text_space;
    char buf[MAX_TEXT];

    /* compute how much data we can read right now */
    switch (ip->connection_type)
    {
        case PORT_TELNET:
            text_space = MAX_TEXT - ip->text_end;

            /* check if we need more space */
            if (text_space < MAX_TEXT / 16) {
                if (ip->text_start > 0) {
                    memmove(ip->text, ip->text + ip->text_start, ip->text_end - ip->text_start);
                    text_space += ip->text_start;
                    ip->text_end -= ip->text_start;
                    ip->text_start = 0;
                }

                if (text_space < MAX_TEXT / 16) {
                    /* the user is sending too much data.  flush it */
                    ip->iflags |= SKIP_COMMAND;
                    ip->text_start = ip->text_end = 0;
                    text_space = MAX_TEXT;
                }
            }
            break;

        case PORT_MUD:
            if (ip->text_end < 4)
                text_space = 4 - ip->text_end;
            else
                text_space = *(volatile int *)ip->text - ip->text_end + 4;
            break;

        default:
            text_space = sizeof(buf);
            break;
    }

    /* read the data from the socket */
    //debug(connections, ("get_user_data: read on fd %d\n", ip->fd));
    num_bytes = OS_socket_read(ip->fd, buf, text_space);

    if (!num_bytes) {
      //if (ip->iflags & CLOSING)
      //    debug_message("get_user_data: tried to read from closing fd.\n");
        ip->iflags |= NET_DEAD;
        remove_interactive(ip->ob, 0);
        return;
    }

    if (num_bytes == -1) {
#ifdef EWOULDBLOCK
        if (socket_errno == EWOULDBLOCK) {
          //        debug(connections, ("get_user_data: read on fd %d: Operation would block.\n", ip->fd));
            return;
        }
#endif
        //      debug_message("get_user_data: read on fd %d\n", ip->fd);
        //      socket_perror("get_user_data: read", 0);
        ip->iflags |= NET_DEAD;
        remove_interactive(ip->ob, 0);
        return;
    }

#ifdef F_NETWORK_STATS
    inet_in_packets++;
    inet_in_volume += num_bytes;
    external_port[ip->external_port].in_packets++;
    external_port[ip->external_port].in_volume += num_bytes;
#endif

    /* process the data that we've just read */
    switch (ip->connection_type)
    {
        case PORT_TELNET:
            copy_chars(ip, buf, num_bytes);
            if (cmd_in_buf(ip))
                ip->iflags |= CMD_IN_BUF;
            break;

        case PORT_MUD:
            memcpy(ip->text + ip->text_end, buf, num_bytes);
            ip->text_end += num_bytes;

            if (num_bytes == text_space) {
                if (ip->text_end == 4) {
                    *(volatile int *)ip->text = ntohl(*(int *)ip->text);
                    if (*(volatile int *)ip->text > MAX_TEXT - 5)
                        remove_interactive(ip->ob, 0);
                } else {
                    svalue_t value;

                    ip->text[ip->text_end] = 0;
                    if (restore_svalue(ip->text + 4, &value) == 0) {
                        STACK_INC;
                        *sp = value;
                    } else {
                        push_undefined();
                    }
                    ip->text_end = 0;
                    apply(APPLY_PROCESS_INPUT, ip->ob, 1, ORIGIN_DRIVER);
                }
            }
            break;

        case PORT_ASCII:
            {
                char *nl, *p;

                memcpy(ip->text + ip->text_end, buf, num_bytes);
                ip->text_end += num_bytes;

                p = ip->text + ip->text_start;
                while ((nl = ( char *)memchr(p, '\n', ip->text_end - ip->text_start))) {
                    ip->text_start = (nl + 1) - ip->text;

                    *nl = 0;
                    if (*(nl - 1) == '\r')
                        *--nl = 0;

                    if (!(ip->ob->flags & O_DESTRUCTED)) {
                        char *str;

                        str = new_string(nl - p, "PORT_ASCII");
                        memcpy(str, p, nl - p + 1);
                        push_malloced_string(str);
                        apply(APPLY_PROCESS_INPUT, ip->ob, 1, ORIGIN_DRIVER);
                    }

                    if (ip->text_start == ip->text_end) {
                        ip->text_start = ip->text_end = 0;
                        break;
                    }

                    p = nl + 1;
                }
            }
            break;

#ifndef NO_BUFFER_TYPE
        case PORT_BINARY:
            {
                buffer_t *buffer;

                buffer = allocate_buffer(num_bytes);
                memcpy(buffer->item, buf, num_bytes);

                push_refed_buffer(buffer);
                apply(APPLY_PROCESS_INPUT, ip->ob, 1, ORIGIN_DRIVER);
            }
            break;
#endif
    }
}

static int clean_buf (interactive_t * ip)
{
    /* skip null input */
    while (ip->text_start < ip->text_end && !*(ip->text + ip->text_start))
        ip->text_start++;

    /* if we've advanced beyond the end of the buffer, reset it */
    if (ip->text_start >= ip->text_end) {
        ip->text_start = ip->text_end = 0;
    }

    /* if we're skipping the current command, check to see if it has been
       completed yet.  if it has, flush it and clear the skip bit */
    if (ip->iflags & SKIP_COMMAND) {
        char *p;

        for (p = ip->text + ip->text_start;  p < ip->text + ip->text_end;  p++) {
            if (*p == '\r' || *p == '\n') {
                ip->text_start += p - (ip->text + ip->text_start) + 1;
                ip->iflags &= ~SKIP_COMMAND;
                return clean_buf(ip);
            }
        }
    }

    return (ip->text_end > ip->text_start);
}

static int cmd_in_buf (interactive_t * ip)
{
    char *p;

    /* do standard input buffer cleanup */
    if (!clean_buf(ip))
        return 0;

    /* if we're in single character mode, we've got input */
    if (ip->iflags & SINGLE_CHAR)
        return 1;

    /* search for a newline.  if found, we have a command */
    for (p = ip->text + ip->text_start;  p < ip->text + ip->text_end;  p++) {
        if (*p == '\r' || *p == '\n')
            return 1;
    }

    /* duh, no command */
    return 0;
}

static char *first_cmd_in_buf (interactive_t * ip)
{
    char *p;
#ifdef GET_CHAR_IS_BUFFERED
    static char tmp[2];
#endif

    /* do standard input buffer cleanup */
    if (!clean_buf(ip))
        return 0;

    p = ip->text + ip->text_start;

    /* if we're in single character mode, we've got input */
    if (ip->iflags & SINGLE_CHAR) {
        if (*p == 8 || *p == 127)
            *p = 0;
#ifndef GET_CHAR_IS_BUFFERED
        ip->text_start++;
        if (!clean_buf(ip))
            ip->iflags &= ~CMD_IN_BUF;
        return p;
#else
        tmp[0] = *p;
        ip->text[ip->text_start++] = 0;
        if (!clean_buf(ip))
            ip->iflags &= ~CMD_IN_BUF;
        return tmp;
#endif
    }

    /* search for the newline */
    while (ip->text[ip->text_start] != '\n' && ip->text[ip->text_start] != '\r')
        ip->text_start++;

    /* check for "\r\n" or "\n\r" */
    if (ip->text_start + 1 < ip->text_end &&
        ((ip->text[ip->text_start] == '\r' && ip->text[ip->text_start + 1] == '\n') ||
         (ip->text[ip->text_start] == '\n' && ip->text[ip->text_start + 1] == '\r'))) {
        ip->text[ip->text_start++] = 0;
    }

    ip->text[ip->text_start++] = 0;
    if (!cmd_in_buf(ip))
        ip->iflags &= ~CMD_IN_BUF;

    return p;
}
/*
 * SIGPIPE handler -- does very little for now.
 */
#ifndef SIG_IGN
#ifdef SIGNAL_FUNC_TAKES_INT
void sigpipe_handler (int sig)
#else
void sigpipe_handler()
#endif
{
    debug(connections, ("SIGPIPE received."));
    //don't comment the next line out, i'm pretty sure we'd crash on the next SIGPIPE, they're not worth it
    signal(SIGPIPE, sigpipe_handler);
}                               /* sigpipe_handler() */
#endif
/*
 * SIGALRM handler.
 */
#ifdef SIGNAL_FUNC_TAKES_INT
void sigalrm_handler (int sig)
#else
void sigalrm_handler()
#endif
{
  outoftime = 1;
}                               /* sigalrm_handler() */

int max_fd;

INLINE void make_selectmasks()
{
    int i;
    max_fd = addr_server_fd;
    /*
     * generate readmask and writemask for select() call.
     */
    FD_ZERO(&readmask);
    FD_ZERO(&writemask);
#ifdef HAS_CONSOLE
    /* set up a console */
    if(has_console > 0)
      FD_SET(STDIN_FILENO, &readmask);
#endif
    /*
     * set new user accept fd in readmask.
     */
    for (i = 0; i < 5; i++) {
        if (!external_port[i].port) continue;
        FD_SET(external_port[i].fd, &readmask);
        if(external_port[i].fd > max_fd)
        	max_fd = external_port[i].fd;
    }
    /*
     * set user fds in readmask.
     */
    for (i = 0; i < max_users; i++) {
        if (!all_users[i] || (all_users[i]->iflags & (CLOSING | CMD_IN_BUF)))
            continue;
        /*
         * if this user needs more input to make a complete command, set his
         * fd so we can get it.
         */
        FD_SET(all_users[i]->fd, &readmask);
        if(all_users[i]->fd > max_fd)
        	max_fd = all_users[i]->fd;
        if (all_users[i]->message_length != 0)
            FD_SET(all_users[i]->fd, &writemask);
    }
    /*
     * if addr_server_fd is set, set its fd in readmask.
     */
    if (addr_server_fd >= 0) {
        FD_SET(addr_server_fd, &readmask);

    }
#if defined(PACKAGE_SOCKETS) || defined(PACKAGE_EXTERNAL)
    /*
     * set fd's for efun sockets.
     */
    for (i = 0; i < max_lpc_socks; i++) {
        if (lpc_socks[i].state != STATE_CLOSED) {
            if (lpc_socks[i].state != STATE_FLUSHING &&
                (lpc_socks[i].flags & S_WACCEPT) == 0){
                FD_SET(lpc_socks[i].fd, &readmask);
                if(lpc_socks[i].fd > max_fd)
                	max_fd = lpc_socks[i].fd;
            }
            if (lpc_socks[i].flags & S_BLOCKED){
                FD_SET(lpc_socks[i].fd, &writemask);
                if(lpc_socks[i].fd > max_fd)
                	max_fd = lpc_socks[i].fd;
            }
        }
    }
#endif
}                               /* make_selectmasks() */

/*
 * Process I/O.
 */
INLINE void process_io()
{
    int i;

    /*
     * check for new user connection.
     */
    for (i = 0; i < 5; i++) {
        if (!external_port[i].port) continue;
        if (FD_ISSET(external_port[i].fd, &readmask)) {
            debug(connections, ("process_io: NEW_USER\n"));
            new_user_handler(i);
        }
    }
    /*
     * check for data pending on user connections.
     */
    for (i = 0; i < max_users; i++) {
        if (!all_users[i] || (all_users[i]->iflags & (CLOSING | CMD_IN_BUF)))
            continue;
        if (all_users[i]->iflags & NET_DEAD) {
            remove_interactive(all_users[i]->ob, 0);
            continue;
        }
        if (FD_ISSET(all_users[i]->fd, &readmask)) {
            debug(connections, ("process_io: USER %d\n", i));
            get_user_data(all_users[i]);
            if (!all_users[i])
                continue;
        }
        if (FD_ISSET(all_users[i]->fd, &writemask))
            flush_message(all_users[i]);
    }
#if defined(PACKAGE_SOCKETS) || defined(PACKAGE_EXTERNAL)
    /*
     * check for data pending on efun socket connections.
     */
    for (i = 0; i < max_lpc_socks; i++) {
        if (lpc_socks[i].state != STATE_CLOSED)
            if (FD_ISSET(lpc_socks[i].fd, &readmask))
                socket_read_select_handler(i);
        if (lpc_socks[i].state != STATE_CLOSED)
            if (FD_ISSET(lpc_socks[i].fd, &writemask))
                socket_write_select_handler(i);
    }
#endif
    /*
     * check for data pending from address server.
     */
    if (addr_server_fd >= 0) {
        if (FD_ISSET(addr_server_fd, &readmask)) {
            debug(connections, ("process_io: IP_DAEMON\n"));
            hname_handler();
        }
    }
#ifdef HAS_CONSOLE
    /* Process console input */
    /* Note: need the has_console on the next line because linux (at least)
             recycles fds, even STDIN_FILENO
     */
    if((has_console > 0) && FD_ISSET(STDIN_FILENO, &readmask)) {
    	char s[1024];
    	int sz;

    	if((sz = read(STDIN_FILENO, s, 1023)) > 0) {
    		s[sz-1] = '\0';
    		console_command(s);
    	}
    	else if(sz == 0) {
    		printf("Console exiting.  The MUD remains.\n");
    		has_console = 0;
    	}
    	else {
    		printf("Console read error: %d %d.  Closing console.\n", sz, errno);
    		has_console = 0;
    		restore_sigttin();
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
static void new_user_handler (int which)
{
    int new_socket_fd;
#ifdef IPV6
    struct sockaddr_in6 addr;
#else
    struct sockaddr_in addr;
#endif
    socklen_t length;
    int i, x;
    object_t *master, *ob;
    svalue_t *ret;

    length = sizeof(addr);
    debug(connections, ("new_user_handler: accept on fd %d\n", external_port[which].fd));
    new_socket_fd = accept(external_port[which].fd,
                           (struct sockaddr *) & addr, &length);
    if (new_socket_fd < 0) {
#ifdef EWOULDBLOCK
        if (socket_errno == EWOULDBLOCK) {
            debug(connections, ("new_user_handler: accept: Operation would block\n"));
        } else {
#else
        if (1) {
#endif
            socket_perror("new_user_handler: accept", 0);
        }
        return;
    }

    /*
     * according to Amylaar, 'accepted' sockets in Linux 0.99p6 don't
     * properly inherit the nonblocking property from the listening socket.
     * Marius, 19-Jun-2000: this happens on other platforms as well, so just
     * do it for everyone
     */
    if (set_socket_nonblocking(new_socket_fd, 1) == -1) {
        socket_perror("new_user_handler: set_socket_nonblocking 1", 0);
        OS_socket_close(new_socket_fd);
        return;
    }
#if defined(SO_NOSIGPIPE)
    i = 1;

    if (setsockopt(new_socket_fd, 1, SO_NOSIGPIPE, &i, sizeof(i)) == -1)
    {
        socket_perror("new_user_handler: setsockopt SO_NOSIGPIPE", 0);
        /* it's ok if this fails */
    }
#endif
    /* find the first available slot */
    for (i = 0; i < max_users; i++)
        if (!all_users[i]) break;

    if (i == max_users) {
        if (all_users) {
            all_users = RESIZE(all_users, max_users + 10, interactive_t *,
                               TAG_USERS, "new_user_handler");
        } else {
            all_users = CALLOCATE(10, interactive_t *,
                                  TAG_USERS, "new_user_handler");
        }
        while (max_users < i + 10)
            all_users[max_users++] = 0;
    }

    set_command_giver(master_ob);
    master_ob->interactive =
        (interactive_t *)
            DXALLOC(sizeof(interactive_t), TAG_INTERACTIVE,
                    "new_user_handler");
#ifndef NO_ADD_ACTION
    master_ob->interactive->default_err_message.s = 0;
#endif
    master_ob->interactive->connection_type = external_port[which].kind;
    master_ob->interactive->sb_buf = (char *)MALLOC(SB_SIZE);
    master_ob->interactive->sb_size = SB_SIZE;
    master_ob->flags |= O_ONCE_INTERACTIVE;
    /*
     * initialize new user interactive data structure.
     */
    master_ob->interactive->ob = master_ob;
#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
    master_ob->interactive->input_to = 0;
#endif
    master_ob->interactive->iflags = 0;
    master_ob->interactive->text[0] = '\0';
    master_ob->interactive->text_end = 0;
    master_ob->interactive->text_start = 0;
#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
    master_ob->interactive->carryover = NULL;
    master_ob->interactive->num_carry = 0;
#endif
#ifndef NO_SNOOP
    master_ob->interactive->snooped_by = 0;
#endif
    master_ob->interactive->last_time = current_time;
#ifdef TRACE
    master_ob->interactive->trace_level = 0;
    master_ob->interactive->trace_prefix = 0;
#endif
#ifdef OLD_ED
    master_ob->interactive->ed_buffer = 0;
#endif
#ifdef HAVE_ZLIB
    master_ob->interactive->compressed_stream = NULL;
#endif

    master_ob->interactive->message_producer = 0;
    master_ob->interactive->message_consumer = 0;
    master_ob->interactive->message_length = 0;
    master_ob->interactive->state = TS_DATA;
    master_ob->interactive->out_of_band = 0;
#ifdef USE_ICONV
    master_ob->interactive->trans = get_translator("UTF-8");
#else
    master_ob->interactive->trans = (struct translation *) master_ob;
    //never actually used, but avoids multiple ifdefs later on!
#endif
    for (x = 0;  x < NSLC;  x++) {
        master_ob->interactive->slc[x][0] = slc_default_flags[x];
        master_ob->interactive->slc[x][1] = slc_default_chars[x];
    }
    all_users[i] = master_ob->interactive;
    all_users[i]->fd = new_socket_fd;
#ifdef F_QUERY_IP_PORT
    all_users[i]->local_port = external_port[which].port;
#endif
#ifdef F_NETWORK_STATS
    all_users[i]->external_port = which;
#endif
    set_prompt("> ");

    memcpy((char *) &all_users[i]->addr, (char *) &addr, length);
#ifdef IPV6
    char tmp[INET6_ADDRSTRLEN];
    debug(connections, ("New connection from %s.\n", inet_ntop(AF_INET6, &addr.sin6_addr, &tmp, INET6_ADDRSTRLEN)));
#else
    debug(connections, ("New connection from %s.\n", inet_ntoa(addr.sin_addr)));
#endif
    num_user++;
    /*
     * The user object has one extra reference. It is asserted that the
     * master_ob is loaded.  Save a pointer to the master ob incase it
     * changes during APPLY_CONNECT.  We want to free the reference on
     * the right copy of the object.
     */
    master = master_ob;
    add_ref(master_ob, "new_user");
    push_number(external_port[which].port);
    ret = apply_master_ob(APPLY_CONNECT, 1);
    /* master_ob->interactive can be zero if the master object self
       destructed in the above (don't ask) */
    set_command_giver(0);
    if (ret == 0 || ret == (svalue_t *)-1 || ret->type != T_OBJECT
        || !master_ob->interactive) {
        if (master_ob->interactive)
            remove_interactive(master_ob, 0);
        else
            free_object(&master, "new_user");
#ifdef IPV6
        debug_message("Connection from %s aborted.\n", inet_ntop(AF_INET6, &addr.sin6_addr, tmp, INET6_ADDRSTRLEN));
#else
        debug_message("Connection from %s aborted.\n", inet_ntoa(addr.sin_addr));
#endif
        return;
    }
    /*
     * There was an object returned from connect(). Use this as the user
     * object.
     */
    ob = ret->u.ob;
#ifdef F_SET_HIDE
    if (ob->flags & O_HIDDEN)
        num_hidden_users++;
#endif
    ob->interactive = master_ob->interactive;
    ob->interactive->ob = ob;
    ob->flags |= O_ONCE_INTERACTIVE;
    /*
     * assume the existance of write_prompt and process_input in user.c
     * until proven wrong (after trying to call them).
     */
    ob->interactive->iflags |= (HAS_WRITE_PROMPT | HAS_PROCESS_INPUT);

    free_object(&master, "new_user");

    master_ob->flags &= ~O_ONCE_INTERACTIVE;
    master_ob->interactive = 0;
    add_ref(ob, "new_user");
    set_command_giver(ob);
    if (addr_server_fd >= 0) {
        query_addr_name(ob);
    }

    if (external_port[which].kind == PORT_TELNET) {
        /* Ask permission to ask them for their terminal type */
        add_binary_message(ob, telnet_do_ttype, sizeof(telnet_do_ttype));
        /* Ask them for their window size */
        add_binary_message(ob, telnet_do_naws, sizeof(telnet_do_naws));
#ifdef HAVE_ZLIB
        add_binary_message(ob, telnet_compress_send_request_v2,
                    sizeof(telnet_compress_send_request_v2));
#endif
        // Ask them if they support mxp.
        add_binary_message(ob, telnet_do_mxp, sizeof(telnet_do_mxp));
        // And mssp
        add_binary_message(ob, telnet_will_mssp, sizeof(telnet_will_mssp));
        // May as well ask for zmp while we're there!
        add_binary_message(ob, telnet_will_zmp, sizeof(telnet_will_zmp));
        // Also newenv
        add_binary_message(ob, telnet_do_newenv, sizeof(telnet_do_newenv));
        // gmcp *yawn*
        add_binary_message(ob, telnet_do_gmcp, sizeof(telnet_do_gmcp));
    }

    logon(ob);
    debug(connections, ("new_user_handler: end\n"));
    set_command_giver(0);
}                               /* new_user_handler() */

/*
 * Return the first command of the next user in sequence that has a complete
 * command in their buffer.  A command is defined to be a single character
 * when SINGLE_CHAR is set, or a newline terminated string otherwise.
 */
static char *get_user_command()
{
    static int NextCmdGiver = 0;

    int i;
    interactive_t *ip;
    char *user_command = 0;

    /* find and return a user command */
    for (i = 0; i < max_users; i++) {
        ip = all_users[NextCmdGiver++];
        NextCmdGiver %= max_users;

        if (!ip || !ip->ob || ip->ob->flags & O_DESTRUCTED)
            continue;

        /* if we've got text to send, try to flush it, could lose the link here */
        if (ip->message_length) {
            object_t *ob = ip->ob;
            flush_message(ip);
            if (!IP_VALID(ip, ob) || (ip->iflags & NET_DEAD))
                continue;
        }

        /* if there's a command in the buffer, pull it out! */
        if (ip->iflags & CMD_IN_BUF) {
            NextCmdGiver++;
            NextCmdGiver %= max_users;
            user_command = first_cmd_in_buf(ip);
            break;
        }
    }

    /* no command found - return 0 */
    if (!user_command)
        return 0;

    /* got a command - return it and set command_giver */
    debug(connections, ("get_user_command: user_command = (%s)\n", user_command));
    save_command_giver(ip->ob);

#ifndef GET_CHAR_IS_BUFFERED
    if (ip->iflags & NOECHO) {
#else
    if ((ip->iflags & NOECHO) && !(ip->iflags & SINGLE_CHAR)) {
#endif
        /* must not enable echo before the user input is received */
        add_binary_message(command_giver, telnet_no_echo, sizeof(telnet_no_echo));
        ip->iflags &= ~NOECHO;
    }

    ip->last_time = current_time;
    return user_command;
}                               /* get_user_command() */

static int escape_command (interactive_t * ip, char * user_command)
{
    if (user_command[0] != '!')
        return 0;
#ifdef OLD_ED
    if (ip->ed_buffer)
        return 1;
#endif
#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
    if (ip->input_to && ( !(ip->iflags & NOESC) && !(ip->iflags & I_SINGLE_CHAR) ) )
        return 1;
#endif
    return 0;
}

static void process_input (interactive_t * ip, char * user_command)
{
    svalue_t *ret;

    if (!(ip->iflags & HAS_PROCESS_INPUT)) {
        parse_command(user_command, command_giver);
        return;
    }

    /*
     * send a copy of user input back to user object to provide
     * support for things like command history and mud shell
     * programming languages.
     */
    copy_and_push_string(user_command);
    ret = apply(APPLY_PROCESS_INPUT, command_giver, 1, ORIGIN_DRIVER);
    if (!IP_VALID(ip, command_giver))
        return;
    if (!ret) {
        ip->iflags &= ~HAS_PROCESS_INPUT;
        parse_command(user_command, command_giver);
        return;
    }

#ifndef NO_ADD_ACTION
    if (ret->type == T_STRING) {
        static char buf[MAX_TEXT];

        strncpy(buf, ret->u.string, MAX_TEXT - 1);
        parse_command(buf, command_giver);
    } else {
        if (ret->type != T_NUMBER || !ret->u.number)
            parse_command(user_command, command_giver);
    }
#endif
}

/*
 * This is the user command handler. This function is called when
 * a user command needs to be processed.
 * This function calls get_user_command() to get a user command.
 * One user command is processed per execution of this function.
 */
int process_user_command()
{
    char *user_command;
    interactive_t *ip=NULL;//for if(ip) below

    /*
     * WARNING: get_user_command() sets command_giver via
     * save_command_giver(), but only when the return is non-zero!
     */
    if (!(user_command = get_user_command()))
        return 0;

    if(command_giver) ip = command_giver->interactive;
    current_interactive = command_giver;    /* this is yuck phooey, sigh */
    if(ip) clear_notify(ip->ob);
    update_load_av();
    debug(connections, ("process_user_command: command_giver = /%s\n", command_giver->obname));

    if(!ip)
      goto exit;

    user_command = translate_easy(ip->trans->incoming, user_command);

    if(ip->iflags & USING_MXP && user_command[0] == ' ' && user_command[1] == '[' && user_command[3] == 'z' ){
      svalue_t *ret;
      copy_and_push_string(user_command);

      ret=apply(APPLY_MXP_TAG, ip->ob, 1, ORIGIN_DRIVER);
      if(ret && ret->type==T_NUMBER && ret->u.number){
	goto exit;
      }
    }

    if (escape_command(ip, user_command)) {
        if (ip->iflags & SINGLE_CHAR) {
            /* only 1 char ... switch to line buffer mode */
            ip->iflags |= WAS_SINGLE_CHAR;
            ip->iflags &= ~SINGLE_CHAR;
#ifdef GET_CHAR_IS_BUFFERED
            ip->text_start = ip->text_end = *ip->text = 0;
#endif
            set_linemode(ip);
        } else {
            if (ip->iflags & WAS_SINGLE_CHAR) {
                /* we now have a string ... switch back to char mode */
                ip->iflags &= ~WAS_SINGLE_CHAR;
                ip->iflags |= SINGLE_CHAR;
                set_charmode(ip);
                if (!IP_VALID(ip, command_giver)) {
                    goto exit;
                }
            }

            process_input(ip, user_command + 1);
        }

        goto exit;
    }

#ifdef OLD_ED
    if (ip->ed_buffer) {
        ed_cmd(user_command);
        goto exit;
    }
#endif

#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
    if (call_function_interactive(ip, user_command)) {
        goto exit;
    }
#endif

    process_input(ip, user_command);

exit:
    /*
     * Print a prompt if user is still here.
     */
    if (IP_VALID(ip, command_giver))
        print_prompt(ip);

    current_interactive = 0;
    restore_command_giver();
    return 1;
}

#define HNAME_BUF_SIZE 200
/*
 * This is the hname input data handler. This function is called by the
 * master handler when data is pending on the hname socket (addr_server_fd).
 */

static void hname_handler()
{
    static char hname_buf[HNAME_BUF_SIZE];
    static int hname_buf_pos;
    int num_bytes;

    num_bytes = HNAME_BUF_SIZE - hname_buf_pos - 1; /* room for nul */
    num_bytes = OS_socket_read(addr_server_fd, hname_buf + hname_buf_pos, num_bytes);
    if (num_bytes <= 0) {
        if (num_bytes == -1) {
#ifdef EWOULDBLOCK
            if (socket_errno == EWOULDBLOCK) {
                debug(connections, ("hname_handler: read on fd %d: Operation would block.\n",
                            addr_server_fd));
                return;
            }
#endif
            debug_message("hname_handler: read on fd %d\n", addr_server_fd);
            socket_perror("hname_handler: read", 0);
        } else {
            debug_message("hname_handler: closing address server connection.\n");
        }
        OS_socket_close(addr_server_fd);
        addr_server_fd = -1;
        return;
    }

    hname_buf_pos += num_bytes;
    hname_buf[hname_buf_pos] = 0;
    debug(connections, ("hname_handler: address server replies: %s", hname_buf));

    while (hname_buf_pos) {
        char *nl, *pp;

        /* if there's no newline, there's more data to come */
        if (!(nl = strchr(hname_buf, '\n')))
            break;
        *nl++ = 0;

        if ((pp = strchr(hname_buf, ' ')) != 0) {
            *pp++ = 0;
            got_addr_number(pp, hname_buf);

            if (isdigit(hname_buf[0]) || hname_buf[0] == ':') {

#ifdef IPV6
            	struct in6_addr addr;
            	int ret;
            	if(1 ==(ret = inet_pton(AF_INET6, hname_buf, &addr))) {
            		if (strcmp(pp, "0") != 0)
            		     add_ip_entry(addr, pp);
            	}
#else
            	unsigned long laddr;

                if ((laddr = inet_addr(hname_buf)) != INADDR_NONE) {
                    if (strcmp(pp, "0") != 0)
                        add_ip_entry(laddr, pp);
                }
#endif
            }
        }

        hname_buf_pos -= (nl - hname_buf);
        if (hname_buf_pos)
            memmove(hname_buf, nl, hname_buf_pos + 1); /* be sure to get the nul */
    }
}

/*
 * Remove an interactive user immediately.
 */
void remove_interactive (object_t * ob, int dested)
{
    int idx;
    /* don't have to worry about this dangling, since this is the routine
     * that causes this to dangle elsewhere, and we are protected from
     * getting called recursively by CLOSING.  safe_apply() should be
     * used here, since once we start this process we can't back out,
     * so jumping out with an error would be bad.
     */
    interactive_t *ip = ob->interactive;

    if (!ip) return;

    if (ip->iflags & CLOSING) {
        if (!dested)
            debug_message("Double call to remove_interactive()\n");
        return;
    }

    debug(connections, ("Closing connection from %s.\n",
                        inet_ntoa(ip->addr.sin_addr)));

    flush_message(ip);
    ip->iflags |= CLOSING;

#ifdef OLD_ED
    if (ip->ed_buffer) {
        save_ed_buffer(ob);
    }
#else
    if (ob->flags & O_IN_EDIT) {
        object_save_ed_buffer(ob);
        ob->flags &= ~O_IN_EDIT;
    }
#endif

    if (!dested) {
        /*
         * auto-notification of net death
         */
        save_command_giver(ob);
        safe_apply(APPLY_NET_DEAD, ob, 0, ORIGIN_DRIVER);
        restore_command_giver();
    }

#ifndef NO_SNOOP
    if (ip->snooped_by) {
        ip->snooped_by->flags &= ~O_SNOOP;
        ip->snooped_by = 0;
    }
#endif

#ifdef HAVE_ZLIB
    if (ip->compressed_stream) {
      end_compression(ip);
    }
#endif

    debug(connections, ("remove_interactive: closing fd %d\n", ip->fd));
    if (OS_socket_close(ip->fd) == -1) {
        socket_perror("remove_interactive: close", 0);
    }
#ifdef F_SET_HIDE
    if (ob->flags & O_HIDDEN)
        num_hidden_users--;
#endif
    num_user--;
    clear_notify(ip->ob);
#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
    if (ip->input_to) {
        free_object(&ip->input_to->ob, "remove_interactive");
        free_sentence(ip->input_to);
        if (ip->num_carry > 0)
            free_some_svalues(ip->carryover, ip->num_carry);
        ip->carryover = NULL;
        ip->num_carry = 0;
        ip->input_to = 0;
    }
#endif
    for (idx = 0; idx < max_users; idx++)
        if (all_users[idx] == ip) break;
    DEBUG_CHECK(idx == max_users, "remove_interactive: could not find and remove user!\n");
    FREE(ip->sb_buf);
    FREE(ip);
    ob->interactive = 0;
    all_users[idx] = 0;
    free_object(&ob, "remove_interactive");
    return;
}                               /* remove_interactive() */

#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
static int call_function_interactive (interactive_t * i, char * str)
{
    object_t *ob;
    funptr_t *funp;
    char *function;
    svalue_t *args;
    sentence_t *sent;
    int num_arg;
#ifdef GET_CHAR_IS_BUFFERED
    int was_single = 0;
    int was_noecho = 0;
#endif

    i->iflags &= ~NOESC;
    if (!(sent = i->input_to))
        return (0);

    /*
     * Special feature: input_to() has been called to setup a call to a
     * function.
     */
    if (sent->ob->flags & O_DESTRUCTED) {
        /* Sorry, the object has selfdestructed ! */
        free_object(&sent->ob, "call_function_interactive");
        free_sentence(sent);
        i->input_to = 0;
        if (i->num_carry)
            free_some_svalues(i->carryover, i->num_carry);
        i->carryover = NULL;
        i->num_carry = 0;
        i->input_to = 0;
        if (i->iflags & SINGLE_CHAR) {
            /*
             * clear single character mode
             */
            i->iflags &= ~SINGLE_CHAR;
#ifndef GET_CHAR_IS_BUFFERED
            set_linemode(i);
#else
            was_single = 1;
            if (i->iflags & NOECHO) {
                was_noecho = 1;
                i->iflags &= ~NOECHO;
            }
#endif
        }

        return (0);
    }
    /*
     * We must all references to input_to fields before the call to apply(),
     * because someone might want to set up a new input_to().
     */

    /* we put the function on the stack in case of an error */
    STACK_INC;
    if (sent->flags & V_FUNCTION) {
      function = 0;
      sp->type = T_FUNCTION;
      sp->u.fp = funp = sent->function.f;
      funp->hdr.ref++;
    } else {
      sp->type = T_STRING;
      sp->subtype = STRING_SHARED;
      sp->u.string = function = sent->function.s;
      ref_string(function);
    }
    ob = sent->ob;

    free_object(&sent->ob, "call_function_interactive");
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
#ifndef GET_CHAR_IS_BUFFERED
        set_linemode(i);
#else
        was_single = 1;
        if (i->iflags & NOECHO) {
            was_noecho = 1;
            i->iflags &= ~NOECHO;
        }
#endif
    }

    copy_and_push_string(str);
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
    if (function) {
        if (function[0] == APPLY___INIT_SPECIAL_CHAR)
            error("Illegal function name.\n");
       (void) apply(function, ob, num_arg + 1, ORIGIN_INTERNAL);
    } else
       call_function_pointer(funp, num_arg + 1);

    pop_stack();                /* remove `function' from stack */

#ifdef GET_CHAR_IS_BUFFERED
    if (IP_VALID(i, ob)) {
        if (was_single && !(i->iflags & SINGLE_CHAR)) {
            i->text_start = i->text_end = 0;
            i->text[0] = '\0';
            i->iflags &= ~CMD_IN_BUF;
            set_linemode(i);
        }
        if (was_noecho && !(i->iflags & NOECHO))
            add_binary_message(i->ob, telnet_no_echo, sizeof(telnet_no_echo));
    }
#endif

    return (1);
}                               /* call_function_interactive() */

int set_call (object_t * ob, sentence_t * sent, int flags)
{
    if (ob == 0 || sent == 0)
        return (0);
    if (ob->interactive == 0 || ob->interactive->input_to)
        return (0);
    ob->interactive->input_to = sent;
    ob->interactive->iflags |= (flags & (I_NOECHO | I_NOESC | I_SINGLE_CHAR));
    if (flags & I_NOECHO)
        add_binary_message(ob, telnet_yes_echo, sizeof(telnet_yes_echo));
    if (flags & I_SINGLE_CHAR)
        set_charmode(ob->interactive);
    return (1);
}                               /* set_call() */
#endif

void set_prompt (const char * str)
{
    if (command_giver && command_giver->interactive) {
        command_giver->interactive->prompt = str;
    }
}                               /* set_prompt() */

/*
 * Print the prompt, but only if input_to not is disabled.
 */
static void print_prompt (interactive_t* ip)
{
    object_t *ob = ip->ob;

#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
    if (ip->input_to == 0) {
#endif
        /* give user object a chance to write its own prompt */
        if (!(ip->iflags & HAS_WRITE_PROMPT))
            tell_object(ip->ob, ip->prompt, strlen(ip->prompt));
#ifdef OLD_ED
        else if (ip->ed_buffer)
            tell_object(ip->ob, ip->prompt, strlen(ip->prompt));
#endif
        else if (!apply(APPLY_WRITE_PROMPT, ip->ob, 0, ORIGIN_DRIVER)) {
            if (!IP_VALID(ip, ob)) return;
            ip->iflags &= ~HAS_WRITE_PROMPT;
            tell_object(ip->ob, ip->prompt, strlen(ip->prompt));
        }
#if defined(F_INPUT_TO) || defined(F_GET_CHAR)
    }
#endif
    if (!IP_VALID(ip, ob)) return;
    /*
     * Put the IAC GA thing in here... Moved from before writing the prompt;
     * vt src says it's a terminator. Should it be inside the no-input_to
     * case? We'll see, I guess.
     */
    if ((ip->iflags & USING_TELNET) && !(ip->iflags & SUPPRESS_GA))
        add_binary_message(command_giver, telnet_ga, sizeof(telnet_ga));
    if (!IP_VALID(ip, ob)) return;
}                               /* print_prompt() */

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
#ifndef NO_SNOOP
int new_set_snoop (object_t * by, object_t * victim)
{
    interactive_t *ip;
    object_t *tmp;

    if (by->flags & O_DESTRUCTED)
        return 0;
    if (victim && (victim->flags & O_DESTRUCTED))
        return 0;

    if (victim) {
        if (!victim->interactive)
            error("Second argument of snoop() is not interactive!\n");
        ip = victim->interactive;
    } else {
        /*
         * Stop snoop.
         */
        if (by->flags & O_SNOOP) {
            int i;

            for (i = 0; i < max_users; i++) {
                if (all_users[i] && all_users[i]->snooped_by == by)
                    all_users[i]->snooped_by = 0;
            }
            by->flags &= ~O_SNOOP;
        }
        return 1;
    }

    /*
     * Protect against snooping loops.
     */
    tmp = by;
    while (tmp) {
        if (tmp == victim)
            return 0;

        /* the person snooping us, if any */
        tmp = (tmp->interactive ? tmp->interactive->snooped_by : 0);
    }

    /*
     * Terminate previous snoop, if any.
     */
    if (by->flags & O_SNOOP) {
        int i;

        for (i = 0; i < max_users; i++) {
            if (all_users[i] && all_users[i]->snooped_by == by)
                all_users[i]->snooped_by = 0;
        }
    }
    if (ip->snooped_by)
        ip->snooped_by->flags &= ~O_SNOOP;
    by->flags |= O_SNOOP;
    ip->snooped_by = by;

    return 1;
}                               /* set_new_snoop() */
#endif

static void query_addr_name (object_t * ob)
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
    debug(connections, ("query_addr_name: sent address server %s\n", dbuf));

    if (OS_socket_write(addr_server_fd, buf, msglen + sizeof(int) + sizeof(int)) == -1) {
        switch (socket_errno) {
        case EBADF:
            debug_message("Address server has closed connection.\n");
            addr_server_fd = -1;
            break;
        default:
            socket_perror("query_addr_name: write", 0);
            break;
        }
    }
}                               /* query_addr_name() */

#define IPSIZE 200
typedef struct {
    char *name;
    svalue_t call_back;
    object_t *ob_to_call;
} ipnumberentry_t;

static ipnumberentry_t ipnumbertable[IPSIZE];

/*
 * Does a call back on the current_object with the function call_back.
 */
int query_addr_number (const char * name, svalue_t * call_back)
{
    static char buf[100];
    static char *dbuf = &buf[sizeof(int) + sizeof(int) + sizeof(int)];
    int msglen;
    int msgtype;
    int i;

    if ((addr_server_fd < 0) || (strlen(name) >=
                  100 - (sizeof(msgtype) + sizeof(msglen) + sizeof(int)))) {
        share_and_push_string(name);
        push_undefined();
        if (call_back->type == T_STRING)
            apply(call_back->u.string, current_object, 2, ORIGIN_INTERNAL);
        else
            call_function_pointer(call_back->u.fp, 2);
        return 0;
    }
    strcpy(dbuf, name);
    msglen = sizeof(int) + strlen(name) +1;

    msgtype = DATALEN;
    memcpy(buf, (char *) &msgtype, sizeof(msgtype));
    memcpy(&buf[sizeof(int)], (char *) &msglen, sizeof(msglen));

    msgtype = NAMEBYIP;
    for (i = 0; i < strlen(name); i++){
      if (isalpha(name[i])) {
	msgtype = IPBYNAME;
	break;
      }
    }

    memcpy(&buf[sizeof(int) + sizeof(int)], (char *) &msgtype, sizeof(msgtype));

    debug(connections, ("query_addr_number: sent address server %s\n", dbuf));

    if (addr_server_fd && OS_socket_write(addr_server_fd, buf, msglen + sizeof(int) + sizeof(int)) == -1) {
        switch (socket_errno) {
        case EBADF:
            debug_message("Address server has closed connection.\n");
            addr_server_fd = -1;
            break;
        default:
            socket_perror("query_addr_name: write", 0);
            break;
        }
        share_and_push_string(name);
        push_undefined();
        if (call_back->type == T_STRING)
            apply(call_back->u.string, current_object, 2, ORIGIN_INTERNAL);
        else
            call_function_pointer(call_back->u.fp, 2);
        return 0;
    } else {
        int i;

/* We put ourselves into the pending name lookup entry table */
/* Find the first free entry */
        for (i = 0; i < IPSIZE && ipnumbertable[i].name; i++)
            ;
        if (i == IPSIZE) {
/* We need to error...  */
            share_and_push_string(name);
            push_undefined();
            if (call_back->type == T_STRING)
                apply(call_back->u.string, current_object, 2, ORIGIN_INTERNAL);
            else
                call_function_pointer(call_back->u.fp, 2);
            return 0;
        }
/* Create our entry... */
        ipnumbertable[i].name = make_shared_string(name);
        assign_svalue_no_free(&ipnumbertable[i].call_back, call_back);
        ipnumbertable[i].ob_to_call = current_object;
        add_ref(current_object, "query_addr_number: ");
        return i + 1;
    }
}                               /* query_addr_number() */

static void got_addr_number (char * number, char * name)
{
    int i;
    char *theName, *theNumber;

    /* First remove all the dested ones... */
    for (i = 0; i < IPSIZE; i++)
        if (ipnumbertable[i].name
            && ipnumbertable[i].ob_to_call->flags & O_DESTRUCTED) {
            free_svalue(&ipnumbertable[i].call_back, "got_addr_number");
            free_string(ipnumbertable[i].name);
            free_object(&ipnumbertable[i].ob_to_call, "got_addr_number: ");
            ipnumbertable[i].name = NULL;
        }
    for (i = 0; i < IPSIZE; i++) {
        if (ipnumbertable[i].name && strcmp(name, ipnumbertable[i].name)== 0) {
            /* Found one, do the call back... */
            theName = ipnumbertable[i].name;
            theNumber = number;

            if (uisdigit(theName[0])) {
                char *tmp;

                tmp = theName;
                theName = theNumber;
                theNumber = tmp;
            }
            if (strcmp(theName, "0")) {
                share_and_push_string(theName);
            } else {
                push_undefined();
            }
            if (strcmp(theNumber, "0")) {
                share_and_push_string(theNumber);
            } else {
                push_undefined();
            }
            push_number(i + 1);
            if (ipnumbertable[i].call_back.type == T_STRING)
                safe_apply(ipnumbertable[i].call_back.u.string,
                           ipnumbertable[i].ob_to_call,
                           3, ORIGIN_INTERNAL);
            else
                safe_call_function_pointer(ipnumbertable[i].call_back.u.fp, 3);
            free_svalue(&ipnumbertable[i].call_back, "got_addr_number");
            free_string(ipnumbertable[i].name);
            free_object(&ipnumbertable[i].ob_to_call, "got_addr_number: ");
            ipnumbertable[i].name = NULL;
        }
    }
}                               /* got_addr_number() */

#undef IPSIZE
#define IPSIZE 200
typedef struct {
#ifdef IPV6
	struct in6_addr addr;
#else
    long addr;
#endif
    char *name;
} ipentry_t;

static ipentry_t iptable[IPSIZE];
static int ipcur;

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_iptable() {
    int i;

    for (i=0; i < IPSIZE; i++)
        if (iptable[i].name)
            EXTRA_REF(BLOCK(iptable[i].name))++;
}
#endif

#ifdef IPV6
char ipv6addr[INET6_ADDRSTRLEN];
#endif

char *query_ip_name (object_t * ob)
{
    int i;

    if (ob == 0)
        ob = command_giver;
    if (!ob || ob->interactive == 0)
        return NULL;
#ifdef IPV6
    for (i = 0; i < IPSIZE; i++) {
        if (!memcmp(&iptable[i].addr, &ob->interactive->addr.sin6_addr, sizeof(ob->interactive->addr.sin6_addr)) &&
            iptable[i].name)
            return (iptable[i].name);
    }

    inet_ntop(AF_INET6, &ob->interactive->addr.sin6_addr, ipv6addr, INET6_ADDRSTRLEN);
    return ipv6addr;
#else
    for (i = 0; i < IPSIZE; i++) {
            if (iptable[i].addr == ob->interactive->addr.sin_addr.s_addr &&
                iptable[i].name)
                return (iptable[i].name);
    }
    return (inet_ntoa(ob->interactive->addr.sin_addr));
#endif
}

#ifdef IPV6
static void add_ip_entry (struct in6_addr addr, char * name)
#else
static void add_ip_entry (long addr, char * name)
#endif
{
    int i;

    for (i = 0; i < IPSIZE; i++) {
        if (!memcmp(&iptable[i].addr, &addr, sizeof(addr)))
            return;
    }
    iptable[ipcur].addr = addr;
    if (iptable[ipcur].name)
        free_string(iptable[ipcur].name);
    iptable[ipcur].name = make_shared_string(name);
    ipcur = (ipcur + 1) % IPSIZE;
}

const char *query_ip_number (object_t * ob)
{
    if (ob == 0)
        ob = command_giver;
    if (!ob || ob->interactive == 0)
        return 0;
#ifdef IPV6
    inet_ntop(AF_INET6, &ob->interactive->addr.sin6_addr, ipv6addr, INET6_ADDRSTRLEN);
        return &ipv6addr[0];
#else
    return (inet_ntoa(ob->interactive->addr.sin_addr));
#endif
}

#ifndef INET_NTOA_OK
/*
 * Note: if the address string is "a.b.c.d" the address number is
 *       a * 256^3 + b * 256^2 + c * 256 + d
 */
char *inet_ntoa (struct in_addr ad)
{
    u_long s_ad;
    int a, b, c, d;
    static char addr[20];       /* 16 + 1 should be enough */

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
#endif                          /* INET_NTOA_OK */

char *query_host_name()
{
    static char name[400];

    gethostname(name, sizeof(name));
    name[sizeof(name) - 1] = '\0';      /* Just to make sure */
    return (name);
}                               /* query_host_name() */

#ifndef NO_SNOOP
object_t *query_snoop (object_t * ob)
{
    if (!ob->interactive)
        return 0;
    return ob->interactive->snooped_by;
}                               /* query_snoop() */

object_t *query_snooping (object_t * ob)
{
    int i;

    if (!(ob->flags & O_SNOOP)) return 0;
    for (i = 0; i < max_users; i++) {
        if (all_users[i] && all_users[i]->snooped_by == ob)
            return all_users[i]->ob;
    }
    fatal("couldn't find snoop target.\n");
    return 0;
}                               /* query_snooping() */
#endif

int query_idle (object_t * ob)
{
    if (!ob->interactive)
        error("query_idle() of non-interactive object.\n");
    return (current_time - ob->interactive->last_time);
}                               /* query_idle() */

#ifdef F_EXEC
int replace_interactive (object_t * ob, object_t * obfrom)
{
    if (ob->interactive) {
        error("Bad argument 1 to exec()\n");
    }
    if (!obfrom->interactive) {
        error("Bad argument 2 to exec()\n");
    }
#ifdef F_SET_HIDE
    if ((ob->flags & O_HIDDEN) != (obfrom->flags & O_HIDDEN)) {
        if (ob->flags & O_HIDDEN) {
            num_hidden_users++;
        } else {
            num_hidden_users--;
        }
    }
#endif
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
    if (obfrom == command_giver) {
        set_command_giver(ob);
    }

    free_object(&obfrom, "exec");
    return (1);
}                               /* replace_interactive() */
#endif

void outbuf_zero (outbuffer_t * outbuf) {
    outbuf->real_size = 0;
    outbuf->buffer = 0;
}

int outbuf_extend (outbuffer_t * outbuf, int l)
{
    int limit;

    DEBUG_CHECK(l < 0, "Negative length passed to outbuf_extend.\n");

    l = (l > MAX_STRING_LENGTH ? MAX_STRING_LENGTH : l);

    if (outbuf->buffer) {
        limit = MSTR_SIZE(outbuf->buffer);
        if (outbuf->real_size + l > limit) {
	  if (outbuf->real_size == MAX_STRING_LENGTH) return 0; /* TRUNCATED */

            /* assume it's going to grow some more */
            limit = (outbuf->real_size + l) * 2;
            if (limit > MAX_STRING_LENGTH) {
                limit = MAX_STRING_LENGTH;
                outbuf->buffer = extend_string(outbuf->buffer, limit);
                return limit - outbuf->real_size;
		}
            outbuf->buffer = extend_string(outbuf->buffer, limit);
        }
    } else {
        outbuf->buffer = new_string(l, "outbuf_extend");
        outbuf->real_size = 0;
    }
    return l;
}

void outbuf_add (outbuffer_t * outbuf, const char * str)
{
    int l, limit;

    if (!outbuf) return;
    l = strlen(str);
    if ((limit = outbuf_extend(outbuf, l)) > 0) {
       strncpy(outbuf->buffer + outbuf->real_size, str, limit);
       outbuf->real_size += (l > limit ? limit : l);
       *(outbuf->buffer + outbuf->real_size) = 0;
    }
}

void outbuf_addchar (outbuffer_t * outbuf, char c)
{
    if(outbuf && (outbuf_extend(outbuf, 1) > 0)) {
      *(outbuf->buffer + outbuf->real_size++) = c;
      *(outbuf->buffer + outbuf->real_size) = 0;
    }
}

void outbuf_addv (outbuffer_t *outbuf, const char *format, ...)
{
    char buf[LARGEST_PRINTABLE_STRING + 1];
    va_list args;

    V_START(args, format);
    V_VAR(outbuffer_t *, outbuf, args);
    V_VAR(char *, format, args);

    vsnprintf(buf, LARGEST_PRINTABLE_STRING, format, args);
    va_end(args);

    if (!outbuf) return;

    outbuf_add(outbuf, buf);
}

void outbuf_fix (outbuffer_t * outbuf) {
    if (outbuf && outbuf->buffer)
        outbuf->buffer = extend_string(outbuf->buffer, outbuf->real_size);
}

void outbuf_push (outbuffer_t * outbuf) {
    STACK_INC;
    sp->type = T_STRING;
    if (outbuf && outbuf->buffer) {
        outbuf->buffer = extend_string(outbuf->buffer, outbuf->real_size);

        sp->subtype = STRING_MALLOC;
        sp->u.string = outbuf->buffer;
    } else {
        sp->subtype = STRING_CONSTANT;
        sp->u.string = "";
    }
}

#ifdef HAVE_ZLIB
void* zlib_alloc(void* opaque, unsigned int items, unsigned int size) {
    return CALLOC(items, size);
}

void zlib_free(void* opaque, void* address) {
    FREE(address);
}

static void end_compression (interactive_t *ip) {
    unsigned char dummy[1];

    if (!ip->compressed_stream) {
        return ;
    }

    ip->compressed_stream->avail_in = 0;
    ip->compressed_stream->next_in = dummy;

    if (deflate(ip->compressed_stream, Z_FINISH) != Z_STREAM_END) {
    }

    deflateEnd(ip->compressed_stream);
    FREE(ip->compressed_stream);
    ip->compressed_stream = NULL;
}

static void start_compression (interactive_t *ip) {
    z_stream* zcompress;

    if (ip->compressed_stream) {
        return ;
    }
    zcompress = (z_stream *) DXALLOC(sizeof(z_stream), TAG_INTERACTIVE,
                                    "start_compression");
    zcompress->next_in = NULL;
    zcompress->avail_in = 0;
    zcompress->next_out = ip->compress_buf;
    zcompress->avail_out = COMPRESS_BUF_SIZE;
    zcompress->zalloc = zlib_alloc;
    zcompress->zfree = zlib_free;
    zcompress->opaque = NULL;

    if (deflateInit(zcompress, 9) != Z_OK) {
        FREE(zcompress);
        fprintf(stderr, "Compression failed.\n");
        return ;
    }

    // Ok, compressing.
    ip->compressed_stream = zcompress;
}

static int flush_compressed_output (interactive_t *ip) {
    int iStart, nBlock, nWrite, len;
    z_stream* zcompress;
    int ret = 1;

    if (!ip->compressed_stream) {
        return ret;
    }

    zcompress = ip->compressed_stream;

    /* Try to write out some data.. */
    len = zcompress->next_out - ip->compress_buf;
    if (len > 0) {
        /* we have some data to write */

        nWrite = 0;
        for (iStart = 0; iStart < len; iStart += nWrite)
            {
                if (len - iStart < 4096) {
                    nBlock =len - iStart;
                } else {
                    nBlock =  4096;
                }
                nWrite = send(ip->fd, &ip->compress_buf[iStart], nBlock,
                              ip->out_of_band);
                if (nWrite < 0) {
                  fprintf(stderr, "Error sending compressed data (%d)\n",
                          errno);

                    if (errno == EAGAIN
#ifndef WIN32
                    		|| errno == ENOSR
#endif
                    		) {
                    	ret = 2;
                        break;
                    }

                    return FALSE; /* write error */
                }

                if (nWrite <= 0) {
                    break;
                }
            }

        if (iStart) {
            /* We wrote "iStart" bytes */
            if (iStart < len) {
                memmove(ip->compress_buf, ip->compress_buf+iStart, len -
                        iStart);

            }

            zcompress->next_out = ip->compress_buf + len - iStart;
        }
    }

    return ret;
}


static int send_compressed (interactive_t *ip, unsigned char* data, int length) {
    z_stream* zcompress;
    int wr = 1;
    int first = 1;

    zcompress = ip->compressed_stream;
    zcompress->next_in = data;
    zcompress->avail_in = length;
    while (zcompress->avail_in && (wr == 1 || first)) {
        if(wr == 2)
	  first = 0;
        zcompress->avail_out = COMPRESS_BUF_SIZE - (zcompress->next_out -
                                                   ip->compress_buf);

        if (zcompress->avail_out) {
            deflate(zcompress, Z_SYNC_FLUSH);
        }

        if(!( wr = flush_compressed_output(ip)))
          return 0;
    }
    return length;
}
#endif

#ifdef F_ACT_MXP
void f_act_mxp(){
  add_binary_message(current_object, telnet_will_mxp, sizeof(telnet_will_mxp));
}
#endif

#ifdef F_SEND_ZMP
void f_send_zmp(){
	add_binary_message(current_object, telnet_start_zmp, sizeof(telnet_start_zmp));
	add_binary_message(current_object, (const unsigned char *)(sp-1)->u.string, strlen((sp-1)->u.string));
	int i;
	unsigned char zero = 0;
	for(i=0; i<sp->u.arr->size; i++){
		if(sp->u.arr->item[i].type == T_STRING){
			add_binary_message(current_object, &zero, 1);
			add_binary_message(current_object, (const unsigned char *)sp->u.arr->item[i].u.string, strlen(sp->u.arr->item[i].u.string));
		}
	}
	add_binary_message(current_object, &zero, 1);
	add_binary_message(current_object, telnet_end_sub, sizeof(telnet_end_sub));
	pop_2_elems();
}
#endif

#ifdef F_SEND_GMCP
void f_send_gmcp(){
	add_binary_message(current_object, telnet_start_gmcp, sizeof(telnet_start_gmcp));
	add_binary_message(current_object, (const unsigned char *)(sp->u.string), strlen(sp->u.string));
	add_binary_message(current_object, telnet_end_sub, sizeof(telnet_end_sub));
}
#endif

#ifdef F_REQUEST_TERM_TYPE
void f_request_term_type(){
  add_binary_message(command_giver, telnet_term_query, sizeof(telnet_term_query));
}
#endif

#ifdef F_START_REQUEST_TERM_TYPE
void f_start_request_term_type(){
  add_binary_message(command_giver, telnet_do_ttype, sizeof(telnet_do_ttype));
}
#endif

#ifdef F_REQUEST_TERM_SIZE
void f_request_term_size(){
  if((st_num_arg == 1) && (sp->u.number == 0))
    add_binary_message(command_giver, telnet_dont_naws,
                       sizeof(telnet_dont_naws));
  else
    add_binary_message(command_giver, telnet_do_naws, sizeof(telnet_do_naws));
  
  if(st_num_arg == 1)
    sp--;
}
#endif
