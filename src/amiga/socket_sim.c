/* hosts/amiga/socket_sim.c
**
** Implements a socket-simulation using the Amiga message ports.
** Also included is the name handling (it can be either read from
** a file or be hardcoded).
**
** The parser communicates with a client via exchanging messages over
** messageports. They know each other's ports just by name, thus a restart
** of one of both doesn't harm the other (simulating netdeath :-).
** To establish a communication, the parser waits at its global port
** '<portnumber>' for connect_messages. If one arrives, the parser takes
** the client's port name from it, and replies the message with the name
** of a newly created port the parser now listens to. All further
** communication takes part between that two ports.
** Both names (of the parser's and of the client's port) are stored as
** 'socket' in a table. The table index gives the fd_number of this socket.
** When dealing with the real port-arrival-signals, the fd_numbers are
** mapped into the real signal numbers, and vice versa.
**
** Note that the parser knows the lifetime of its listening ports, thus
** storing the actual port address into the socket-entry is safe.
**
** This code is based on the UnixLib by Erik van Roode.
**
**   20-Oct-92 [lars]  Done for DICE 2.06.40
**   14-Jan-93 [lars]  Made HOSTFILE optional.
**   24-Feb-93 [lars]  Small fix to support compilation for OS 1.3
**   28-Feb-93 [lars]  Moved to DICE 2.07.53
**   02-Apr-93 [lars]  Searches hostfile now in etc: instead of :etc/
*/

/*-----------------------------------------------------------------------*/

#include <sys/types.h>

#include <exec/types.h>
#include <exec/ports.h>
#ifdef INCLUDE_VERSION
#include <dos/dos.h>
#else
#include <libraries/dos.h>
#endif
#include <devices/timer.h>

#include <stdio.h>
#include <strings.h>
#include <errno.h>

#include "socket.h"
/*#include "telnet.h"*/
#include "mudmsgs.h"
#include "nsignal.h"		/* the timer functions */

/*-----------------------------------------------------------------------*/

extern __regargs __geta4 ULONG catch_exception(__D0 ULONG mask);

 /* The default hardcoded hostname. */
#define HOSTNAME "128.0.0.1 amoeba"

 /*
  * The hostname file to use. * Comment it out if to use the hardcoded
  * HOSTNAME only.
  */
#define HOSTFILE "etc:hosts"

 /* The simulated sockets */

#define MAXSOCKET 32		/* More than we can handle at all */

static struct socket_entry {
    unsigned char in_use;
    char *to_client;		/* The client's listening port */
    char *from_client;		/* Our listening port's name */
    struct MsgPort *port;	/* Our listening port */
}            sockets[MAXSOCKET];


/*-----------------------------------------------------------------------
** int SafePutToPort (struct Message *message, char *portname)
**
**   Multitasking-secure port handling.
*/

int SafePutToPort(struct Message * message, char *portname)
{
    register struct MsgPort *port = 0L;

    if (portname != NULL) {
	Forbid();
	if (port = (struct MsgPort *) FindPort(portname))
	    PutMsg(port, message);
	Permit();
    }
    if (port) {
	errno = 0;
	return 1;
    }
    errno = EPIPE;
    return 0;
}

/*-----------------------------------------------------------------------
** int shutdown (int socket, int number)
**
**   Send the client at <socket> the shutdown codes.
**   It is the last thing the parser sends the client.
*/

int shutdown(int socket, int number)
{
    unsigned char tmp[3];

    tmp[0] = IAC;
    tmp[1] = DO;
    tmp[2] = TELOPT_ECHO;
    write_socket(socket, tmp, 3);
    return 0;
}


/*-----------------------------------------------------------------------
** int write_socket (int socket, char *buffer, int length)
**
**   Write <length> chars from <buffer> to the client at <socket> and
**   wait for the reply.
*/

int write_socket(int socket, char *buffer, int length)
{
    register struct data_message *msg;
    register struct MsgPort *waitport;
    register ULONG sig;

    if (!sockets[socket].in_use) {
	fprintf(stderr, "write_socket: Socket %d is not in use.\n", socket);
	errno = EINTR;
	return -1;
    }
    waitport = (struct MsgPort *) CreatePort(0, 0);
    if (!waitport) {
	fprintf(stderr, "write_socket: Could not allocate replyport.\n");
	errno = EIO;
	return -1;
    }
    if (!(msg = (struct data_message *) malloc(sizeof(struct data_message)))) {
	fprintf(stderr, "write_socket: Could not allocate message.\n");
	errno = EIO;
	DeletePort(waitport);
	return -1;
    }
    msg->Msg.mn_Node.ln_Type = NT_MESSAGE;
    msg->Msg.mn_Length = sizeof(struct data_message);
    msg->Msg.mn_ReplyPort = waitport;

    msg->buffer = buffer;
    msg->length = length;

    if (!SafePutToPort((struct Message *) msg, sockets[socket].to_client)) {
	errno = EPIPE;
	length = -1;
    } else {
	sig = Wait(SIGBREAKF_CTRL_D | 1L << waitport->mp_SigBit);
	if (sig & SIGBREAKF_CTRL_D) {
	    errno = EIO;
	    fprintf(stderr, "write_socket : Aborted waiting for reply.\n");
	    DeletePort(waitport);
	    free((char *) msg);
	    return -1;
	}
	errno = 0;		/* NO ERROR */
    }
    free((char *) msg);
    DeletePort(waitport);
    return length;
}

/*-----------------------------------------------------------------------
** int read_socket (int socket, char *buffer, int length)
**
**   Read from the client at <socket> max. <length> chars into <buffer>.
**   Return the length actual read.
*/

int read_socket(int socket, char *buffer, int length)
{
    register struct data_message *msg = NULL;
    register struct MsgPort *port = NULL;
    register int bufpos = 0;	/* first free position in the buffer */
    register int i;

    if (!sockets[socket].in_use) {
	fprintf(stderr, "read_socket: Socket %d is not in use.\n", socket);
	errno = EINTR;
	return -1;
    }
    if (!sockets[socket].port) {
	fprintf(stderr, "read_socket: Socket %d has no port.\n", socket);
	errno = EHOSTUNREACH;
	return -1;
    }
    /*
     * Read all messages from the port. * If the buffer overflows, discard
     * the rest.
     */
    while (msg = (struct data_message *) GetMsg(sockets[socket].port)) {
	/* concatenate the received message to the buffer */
	for (i = 0; i < msg->length && bufpos + i < length; i++)
	    buffer[bufpos + i] = msg->buffer[i];

	ReplyMsg((struct Message *) msg);	/* tell client we received it */
	bufpos = bufpos + i;
    }
    if (bufpos == 0) {
	fprintf(stderr, "read_socket : Could not read any message.\n");
	errno = EMSGSIZE;
	return -1;
    }
    return bufpos;
}

/*-----------------------------------------------------------------------
** int find_free_socket()
**
**   Return the number of the next free entry in sockets[], else -1.
*/

int find_free_socket()
{
    register short entry = MAXSOCKET - 1;

    while (entry >= 0) {
	if (!sockets[entry].in_use)
	    return entry;
	entry--;
    }
    return -1;
}

/*-----------------------------------------------------------------------
** int socket (int domain, int type, int protocol)
**
**   Open a new socket and set our ear (i.e. a messageport) on it.
**   Return it's index as fd_number.
*/

int socket(int domain, int type, int protocol)
{
    struct MsgPort *port;
    short new_sd;
    char *name, *tmpname, *tmp;
    static int count = 0;

    /* Open client parserport with a unique name */
    tmpname = "parserXXXXXXXX";
    tmp = tmpname + strlen(tmpname) - 8;
    sprintf(tmp, "%08x", count);

    Forbid();			/* Let no-one change the list while we are
				 * scanning it ! */
    while (FindPort(tmpname))
	sprintf(tmp, "%08x", ++count);
    name = (char *) malloc(strlen(tmpname) + 1);
    strcpy(name, tmpname);

    new_sd = find_free_socket();
    if (new_sd == -1) {
	Permit();
	errno = EMFILE;
	return -1;
    }
    if ((port = (struct MsgPort *) CreatePort(name, 0)) == NULL) {
	Permit();
	errno = ENOBUFS;
	return -1;
    }
    Permit();
    sockets[new_sd].from_client = name;	/* we will listen to this name */
    sockets[new_sd].port = port;
    sockets[new_sd].to_client = NULL;	/* will be filled in by accept() */
    sockets[new_sd].in_use = 1;

    return new_sd;
}

/*-----------------------------------------------------------------------
** int accept (int s, struct sockaddr *addr, int *addrlen)
**
**   Accept on socket <s> the connect_message of a new client and
**   create a new socket for it and fill in the <addr> record.
**   Return the socket_fd_number.
*/

int accept(int s, struct sockaddr * addr, int *addrlen)
{
    char hname[100];
    struct hostent *host;
    register struct connect_message *msg;
    register int new_socket;
    register char *to_client;

    if (!sockets[s].in_use) {
	fprintf(stderr, "accept: Socket %d is not in use.\n", s);
	errno = EINTR;
	return -1;
    }
    if (!sockets[s].port) {
	fprintf(stderr, "accept: Socket %d has no port.\n", s);
	return -1;
    }
    /* Allow 1 client, let the rest wait */
    msg = (struct connect_message *) GetMsg(sockets[s].port);
    if (!msg) {
	errno = EWOULDBLOCK;
	return -1;
    }
    if ((new_socket = socket(0, 0, 0)) == -1) {
	msg->port_name = NULL;
	ReplyMsg((struct Message *) msg);
	return -1;
    }
    /* store the port to which we must send data */
    to_client = (char *) malloc(strlen(msg->port_name) + 1);
    strcpy(to_client, msg->port_name);
    sockets[new_socket].to_client = to_client;

    /* Fill in the sockaddr record */
    if (!gethostname(hname, sizeof(hname))
	&& (host = gethostbyname(hname)) != NULL
	) {
	((struct sockaddr_in *) addr)->sin_addr.s_addr = inet_addr(host->h_addr_list[0]);
	free(host);
    } else
	((struct sockaddr_in *) addr)->sin_addr.s_addr = 0x80000001;

    /* return the port to which we will listen */
    msg->port_name = sockets[new_socket].from_client;

    /* if another socket has the same to_client, that other is invalid .. */
    for (s = MAXSOCKET - 1; s >= 0; s--) {
	if (!sockets[s].in_use || s == new_socket)
	    continue;
	if (!strcmp(sockets[s].to_client, sockets[new_socket].to_client)) {
	    free(sockets[s].to_client);
	    sockets[s].to_client = NULL;
	    close_socket(s);
	}
    }
    ReplyMsg((struct Message *) msg);
    return new_socket;
}

/*-----------------------------------------------------------------------
** int close_socket (int socket)
**
**   Close the given <socket> and deallocate all ressources.
*/

int close_socket(int socket)
{

    /* I should check that this was the last reference to the socket */
    if (socket < 0 || socket >= MAXSOCKET) {
	fprintf(stderr, "close_socket: Illegal socket number %d.\n", socket);
	return;
    }
    if (!sockets[socket].in_use) {
	fprintf(stderr, "close_socket: Socket %d not in use.\n", socket);
	return;
    }
    if (!sockets[socket].port) {
	fprintf(stderr, "close_socket: Socket %d has no port.\n");
	return;
    }
    DeletePort(sockets[socket].port);

    if (sockets[socket].from_client)
	free(sockets[socket].from_client);
    if (sockets[socket].to_client)
	free(sockets[socket].to_client);

    sockets[socket].from_client = NULL;
    sockets[socket].to_client = NULL;
    sockets[socket].port = NULL;
    sockets[socket].in_use = 0;
    return 0;
}

/*-----------------------------------------------------------------------
** int bind (int s, struct sockaddr *name, int namelen)
**
**   Bind an existing socket <s> to a new hostname/-port <name>.
*/

int bind(int s, struct sockaddr * name, int namelen)
{
    register struct MsgPort *new_port;
    register char *port_name;

    /*
     * according to the manual, socket s should have no name yet, but it
     * should get the name specified in name. So lets remove the old name,
     * and replace it with a new one. The portname is specified as an
     * unsigned int, but let's pretend it is a 32 bits number. We need 10 + 1
     * digits in the string, let's make that 20 and hope it's enough
     */
    port_name = (char *) malloc(20);
    sprintf(port_name, "%u", ((struct sockaddr_in *) name)->sin_port);

    if (!sockets[s].in_use) {
	fprintf(stderr, "bind: Socket %d is not in use.\n", s);
	return -1;
    }
    if (!sockets[s].port) {
	fprintf(stderr, "bind: Socked %d has no port.\n", s);
	return -1;
    }
    if (FindPort(port_name) != NULL) {
	fprintf(stderr, "bind: Non-unique name '%s' given.\n", port_name);
	errno = EADDRINUSE;
	return -1;
    }
    /*
     * Delete the old port, and create a new port with a name, no checking
     * for messages, cos I did not send any
     */
    new_port = (struct MsgPort *) CreatePort(port_name, 0);
    if (!new_port) {
	fprintf(stderr, "bind: Failed to create new socketport.\n");
	return -1;
    }
    /* delete the old port */
    close_socket(s);

    sockets[s].from_client = port_name;
    sockets[s].port = new_port;
    sockets[s].to_client = NULL;/* no replyport ! */
    sockets[s].in_use = 1;

    return 0;
}

/*-----------------------------------------------------------------------
** int socket_select (int width, fd_set *readfds, fd_set *writefds,
**		      fd_set *exceptfds, struct timeval *timeout)
**
**   Check if any signals from the sets are available.
**   Return the number of ready descriptors, 0 on timeout or -1 on failure.
**   This implementation ignores width, writefds and exceptfds.
*/

int socket_select(int width, fd_set * readfds, fd_set * writefds,
		      fd_set * exceptfds, struct timeval * timeout)
{

    struct timerequest *tr = NULL;
    ULONG signals, portsig;
    short sock, count;
    fd_set readyfds;
    static fd_set savefds;
    static short saved = 0;

    if (saved) {
	memcpy(readfds, &savefds, sizeof(fd_set));
	saved = 0;
	return 1;
    }
    if (!setup_timer(UNIT_VBLANK, &tr)) {
	/* failed to create a timerequest */
	errno = 0;
	return -1;
    }
    portsig = 1L << tr->tr_node.io_Message.mn_ReplyPort->mp_SigBit;
    signals = portsig | EXT_SIGINT;
    for (sock = MAXSOCKET - 1; sock >= 0; sock--)
	if (sockets[sock].in_use && sockets[sock].port)
	    signals |= 1L << sockets[sock].port->mp_SigBit;

    start_timer(timeout, tr);
    signals = Wait(signals);
    cleanup_timer(&tr);

    /*
     * Unset all fd-bits in readfds which sockets are not ready. * Count the
     * ready ones.
     */
    count = 0;
    memcpy(&readyfds, readfds, sizeof(fd_set));
    for (sock = MAXSOCKET - 1; sock >= 0; sock--) {
	if (!sockets[sock].in_use || !sockets[sock].port)
	    continue;
	if (signals & (1L << sockets[sock].port->mp_SigBit))
	    count++;
	else
	    FD_CLR(sock, &readyfds);
    }


    if (signals & EXT_SIGINT) {	/* we must save the ready sockets */
	SetSignal(0L, EXT_SIGINT);
	memcpy(&savefds, &readyfds, sizeof(fd_set));
	saved = 1;
	errno = EINTR;
	return -1;
    }
    /* if time-out and ready sockets, pretend as if no time-out occured */
    if (signals == portsig) {
	FD_ZERO(readfds);
	return 0;		/* a real time-out */
    }
    memcpy(readfds, &readyfds, sizeof(fd_set));
    return count;
}

/*-----------------------------------------------------------------------
** struct hostent *gethostbyname (char *host_name)
**
**   Search an host with the given <host_name> in the HOSTFILE.
**   If the hostname is hardcoded, just check against that.
**   Note that the host_name is not copied.
*/

struct hostent *gethostbyname(char *host_name)
{
    struct hostent *host;
    char **aliases, **addr_list;
    char tmp[100];		/* should be long enough */
    int a, b, c, d, count;
    FILE *f;
    char address[20];		/* 16+4 should be long enough */

#ifdef HOSTFILE
    if (!(f = fopen(HOSTFILE, "r"))) {
	if (sscanf(HOSTNAME, "%d.%d.%d.%d %s", &a, &b, &c, &d, tmp) != 5
	    || strcmp(host_name, tmp)
	    )
	    return NULL;
    } else {
	while ((count = fscanf(f, "%d.%d.%d.%d %s", &a, &b, &c, &d, tmp)) == 5
	       && strcmp(host_name, tmp)
	    );
	fclose(f);
	if (count != 5)
	    return NULL;
    }
#else
    if (sscanf(HOSTNAME, "%d.%d.%d.%d %s", &a, &b, &c, &d, tmp) != 5
	|| !strcmp(host_name, tmp)
	)
	return NULL;
#endif

    /* ok, data was read, now fill the hostent structure with it */
    host = (struct hostent *) malloc(sizeof(struct hostent));
    if (!host)
	return NULL;
    host->h_name = host_name;
    aliases = (char **) malloc(4);
    if (!aliases)
	return NULL;
    aliases[0] = '\0';
    host->h_aliases = aliases;
    addr_list = (char **) malloc(4);
    if (!addr_list)
	return NULL;
    sprintf(address, "%d.%d.%d.%d", a, b, c, d);
    addr_list[0] = (char *) malloc(strlen(address) + 1);
    strcpy(addr_list[0], address);
    host->h_addr_list = addr_list;
    host->h_addrtype = AF_INET;
    host->h_length = strlen(address);
    return host;
}

/*-----------------------------------------------------------------------
** int gethostname (char *name, int namelen)
**
**   Read the first entry from :etc/hosts and put the contained hostname
**   into <name>. Return 0 on success, -1 on error.
**   If the hostname is hardcoded, just use that one.
*/

int gethostname(char *name, int namelen)
{
    FILE *f;
    char tmp[100];		/* should be long enough */
    int a, b, c, d;

#ifdef HOSTFILE
    if (!(f = fopen(HOSTFILE, "r"))) {
	if (sscanf(HOSTNAME, "%d.%d.%d.%d %s", &a, &b, &c, &d, tmp) != 5) {
	    fprintf(stderr, "gethostname: Could not open %s\n", HOSTFILE);
	    fprintf(stderr, "gethostname: Illegal defined HOSTNAME\n");
	    return -1;
	}
    } else {
	if (fscanf(f, "%d.%d.%d.%d %s", &a, &b, &c, &d, tmp) != 5) {
	    fprintf(stderr, "gethostname: Illegal line found in %s\n", HOSTFILE);
	    fclose(f);
	    return -1;
	}
	fclose(f);
    }
#else
    if (sscanf(HOSTNAME, "%d.%d.%d.%d %s", &a, &b, &c, &d, tmp) != 5) {
	fprintf(stderr, "gethostname: Illegal defined HOSTNAME\n");
	return -1;
    }
#endif

    if (strlen(tmp) > namelen)
	return -1;
    strcpy(name, tmp);
    return 0;
}

/*-----------------------------------------------------------------------
** char *inet_ntoa (struct in_addr in)
**
**   Convert the numeric IP address into a nice (static) string.
**   ...plain and dirty...
*/

char *inet_ntoa(struct in_addr in)
{
    static char address[20];
    int a, b, c, d;
    unsigned long adr;

    adr = in.s_addr;

    a = adr >> 24 & 0xFF;
    b = (adr >> 16) & 0xFF;
    c = (adr >> 8) & 0xFF;
    d = adr & 0xFF;
    sprintf(address, "%d.%d.%d.%d", a, b, c, d);
    return address;
}

/*-----------------------------------------------------------------------
** unsigned long inet_addr (char *cp)
**
**   Convert a string with the IP-adr ('a.b.c.d') into an unsigned long.
**   ...plain and dirty...
*/

unsigned long inet_addr(char *cp)
{
    int a, b, c, d;

    if (sscanf(cp, "%d.%d.%d.%d", &a, &b, &c, &d) != 4)
	return -1L;
    if (a < 0 || a > 255)
	return -1L;
    if (b < 0 || b > 255)
	return -1L;
    if (c < 0 || c > 255)
	return -1L;
    if (d < 0 || d > 255)
	return -1L;
    return ((a & 0xFF) << 24) | ((b & 0xFF) << 16) | ((c & 0xFF) << 8) | (d & 0xFF);
}

/*-----------------------------------------------------------------------
** int getpeername (int s, struct sockaddr *name, int *namelen)
**
**   Put the name of the host to which the socket is connected to into
**   the sa_data field of the socket structure, and set namelen to its
**   length.
*/

int getpeername(int s, struct sockaddr * name, int *namelen)
{
    char *this_host;

    this_host = inet_ntoa(((struct sockaddr_in *) name)->sin_addr);
    *namelen = strlen(this_host);
    strcpy(name->sa_data, this_host);
    return 0;			/* success */
}

/*************************************************************************/
