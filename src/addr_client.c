#include "std.h"
#include "comm.h"
#include "network_incl.h"

#define HNAME_BUF_SIZE 2048
#define IPSIZE 200

typedef struct {
    char *name;
    svalue_t call_back;
    object_t *ob_to_call;
} ipnumberentry_t;

typedef struct {
    long addr;
    char *name;
} ipentry_t;

int addr_server_fd = -1;

static ipnumberentry_t ipnumbertable[IPSIZE];
static ipentry_t iptable[IPSIZE];

static void add_ip_entry PROT((long, char *));
static void got_addr_number PROT((char *, char *));
static void shutdown_addr_server PROT((void));

void init_addr_server P2(char *, hostname, int, addr_server_port)
{
    struct sockaddr_in server;
    struct hostent *hp;
    int server_fd;
    int optval;
    long addr;

    if (addr_server_fd >= 0 || !hostname)
	return;

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
    if (server_fd == INVALID_SOCKET) {	/* problem opening socket */
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
	if (socket_errno == ECONNREFUSED)
	    debug_message("Connection to address server (%s %d) refused.\n",
			  hostname, addr_server_port);
	else 
	    socket_perror("init_addr_server: connect", 0);
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
}

/*
 * This is the hname input data handler. This function is called by the
 * master handler when data is pending on the hname socket (addr_server_fd).
 */
void hname_handler PROT((void))
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
	shutdown_addr_server();
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

	    if (isdigit(hname_buf[0])) {
		unsigned long laddr;

		if ((laddr = inet_addr(hname_buf)) != INADDR_NONE) {
		    if (strcmp(pp, "0") != 0)
			add_ip_entry(laddr, pp);
		}
	    }
	}

	hname_buf_pos -= (nl - hname_buf);
	if (hname_buf_pos)
	    memmove(hname_buf, nl, hname_buf_pos + 1); /* be sure to get the nul */
    }
}

int request_resolution P2(char *, addr, int, type)
{
    static char hname_output_buffer[HNAME_BUF_SIZE];
    static char *dbuf = &hname_output_buffer[sizeof(int) * 3];
    int msglen, msgtype, tosend, sent = 0;

    strcpy(dbuf, addr);
    msglen = sizeof(int) + strlen(dbuf) + 1;

    msgtype = DATALEN;
    memcpy(hname_output_buffer, (char *)&msgtype, sizeof(msgtype));
    memcpy(&hname_output_buffer[sizeof(int)], (char *)&msglen, sizeof(msglen));

    memcpy(&hname_output_buffer[sizeof(int) * 2], (char *)&type, sizeof(type));
    debug(connections, ("query_addr_name: sent address server %s\n", dbuf));

    tosend = msglen + sizeof(int) + sizeof(int);
    while (sent < tosend) {
	int nb;

	nb = OS_socket_write(addr_server_fd, hname_output_buffer + sent, tosend - sent);
	if (nb <= 0) {
	    if (nb == -1 && socket_errno == EINTR
#ifdef EWOULDBLOCK
		|| socket_errno == EWOULDBLOCK
#endif
		) {
		continue;
	    }
	    debug_message("Address server has closed connection.\n");
	    shutdown_addr_server();
	    return 0;
	}

	sent += nb;
    }

    return 1;
}

/*
 * Does a call back on the current_object with the function call_back.
 */
int query_addr_number P2(char *, name, svalue_t *, call_back)
{
    int i, ok = 1, matched = 0, x = -1;

    /* Find the first entry in the pending lookup table */
    for (i = 0;  i < IPSIZE;  i++) {
	if (ipnumbertable[i].name && ipnumbertable[i].ob_to_call->flags & O_DESTRUCTED) {
	    free_svalue(&ipnumbertable[i].call_back, "query_addr_number");
	    free_string(ipnumbertable[i].name);
	    free_object(ipnumbertable[i].ob_to_call, "query_addr_number");
	    ipnumbertable[i].name = 0;
	    if (x != -1) x = i;
	} else if (!ipnumbertable[i].name && x == -1) x = i;
	else if (ipnumbertable[i].name && !strcmp(ipnumbertable[i].name, name))
	    matched = 1;
    }

    if (addr_server_fd < 0 || strlen(name) >= HNAME_BUF_SIZE - (sizeof(int) * 3) || x == -1)
	ok = 0;
    else {
	/* If there's already a request pending, don't request again, we'll hit the callback for
	 * this one with the return from the original request
	 */
	if (!matched)
	    ok = request_resolution(name, (name[0] >= '0' && name[0] <= '9') ? NAMEBYIP : IPBYNAME);
    }

    if (!ok) {
	share_and_push_string(name);
	push_undefined();
	if (call_back->type == T_STRING)
	    apply(call_back->u.string, current_object, 2, ORIGIN_INTERNAL);
	else
	    call_function_pointer(call_back->u.fp, 2);
	return 0;
    }

    ipnumbertable[x].name = make_shared_string(name);
    assign_svalue_no_free(&ipnumbertable[x].call_back, call_back);
    ipnumbertable[x].ob_to_call = current_object;
    add_ref(current_object, "query_addr_number");

    return x + 1;
}

static void got_addr_number P2(char *, number, char *, name)
{
    int i;
    char *theName, *theNumber;

    for (i = 0; i < IPSIZE; i++) {
	if (ipnumbertable[i].name && ipnumbertable[i].ob_to_call->flags & O_DESTRUCTED) {
	    free_svalue(&ipnumbertable[i].call_back, "got_addr_number");
	    free_string(ipnumbertable[i].name);
	    free_object(ipnumbertable[i].ob_to_call, "got_addr_number: ");
	    ipnumbertable[i].name = 0;
	}

	if (ipnumbertable[i].name && strcmp(name, ipnumbertable[i].name)== 0) {
	    /* Found one, do the call back... */
	    if (uisdigit(theName[0])) {
		theName = number;
		theNumber = ipnumbertable[i].name;
	    } else {
		theName = ipnumbertable[i].name;
		theNumber = number;
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
	    if (ipnumbertable[i].call_back.type == T_STRING) {
		safe_apply(ipnumbertable[i].call_back.u.string, 
			   ipnumbertable[i].ob_to_call, 3, ORIGIN_INTERNAL);
	    } else {
		safe_call_function_pointer(ipnumbertable[i].call_back.u.fp, 3);
	    }
	    free_svalue(&ipnumbertable[i].call_back, "got_addr_number");
	    free_string(ipnumbertable[i].name);
	    free_object(ipnumbertable[i].ob_to_call, "got_addr_number: ");
	    ipnumbertable[i].name = 0;
	}
    }
}

static void add_ip_entry P2(long, addr, char *, name)
{
    static int ipcur = 0;
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

static void shutdown_addr_server PROT((void))
{
    int i;

    if (addr_server_fd > -1) {
	OS_socket_close(addr_server_fd);
	addr_server_fd = -1;
    }

    /* Since the server has gone away, pending requests will never be fulfilled.
     * Rather than waste space and hold references we don't need to hold, blow
     * away the pending request table.  In doing so, callback everybody that is
     * waiting to notify them that we've lost the connection.
     */
    for (i = 0;  i < IPSIZE;  i++) {
	if (ipnumbertable[i].name) {
	    char *theName, *theNumber;

	    if (uisdigit(theName[0])) {
		push_undefined();
		push_shared_string(ipnumbertable[i].name);
	    } else {
		push_shared_string(ipnumbertable[i].name);
		push_undefined();
	    }
	    push_number(i + 1);
	    if (ipnumbertable[i].call_back.type == T_STRING) {
		safe_apply(ipnumbertable[i].call_back.u.string, 
			   ipnumbertable[i].ob_to_call, 3, ORIGIN_INTERNAL);
	    } else {
		safe_call_function_pointer(ipnumbertable[i].call_back.u.fp, 3);
	    }

	    free_svalue(&ipnumbertable[i].call_back, "got_addr_number");
	    free_string(ipnumbertable[i].name);
	    free_object(ipnumbertable[i].ob_to_call, "got_addr_number: ");
	    ipnumbertable[i].name = 0;
	}
    }
}

char *query_ip_name P1(object_t *, ob)
{
    int i;

    if (ob == 0)
	ob = command_giver;
    if (!ob || ob->interactive == 0)
	return 0;
    for (i = 0; i < IPSIZE; i++) {
	if (iptable[i].addr == ob->interactive->addr.sin_addr.s_addr &&
	    iptable[i].name)
	    return (iptable[i].name);
    }
    return (inet_ntoa(ob->interactive->addr.sin_addr));
}

char *query_ip_number P1(object_t *, ob)
{
    if (ob == 0)
	ob = command_giver;
    if (!ob || ob->interactive == 0)
	return 0;
    return (inet_ntoa(ob->interactive->addr.sin_addr));
}

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_iptable PROT((void))
{
    int i;

    for (i = 0;  i < IPSIZE;  i++) {
	if (ipnumbertable[i].name) {
	    EXTRA_REF(BLOCK(ipnumbertable[i].name))++;
	    mark_svalue(&ipnumbertable[i].call_back);
	    ipnumbertable[i].ob_to_call->extra_ref++;
	}
	if (iptable[i].name)
	    EXTRA_REF(BLOCK(iptable[i].name))++;
    }
}
#endif
