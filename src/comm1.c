#define DO_FCNTL (defined(vax) || defined(SYSV) || \
	(defined(sun) && !defined(sparc)))
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#define	TELOPTS
#ifndef hpux
#include <arpa/telnet.h>
#else
#include "telnet.h"
#endif
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <memory.h>
#if DO_FCNTL
#include <fcntl.h>
#endif
#ifdef _AIX
#include <sys/select.h>
#endif
#include "config.h"
#include "lint.h"
#include "interpret.h"
#include "comm.h"
#include "object.h"
#include "sent.h"
#include "patchlevel.h"
#include "wiz_list.h"

#ifdef MUDWHO
#include "mudwho.h"
#endif

#ifdef MSDOS
#include "msdos/pcomm.h"
#endif

int socket PROT((int, int, int));
#ifndef sgi
#ifdef NeXT
int setsockopt PROT((int, int, int, void *, int));
#else
int setsockopt PROT((int, int, int, char *, int));
#endif
int bind PROT((int, struct sockaddr *, int));
int listen PROT((int, int));
int accept PROT((int, struct sockaddr *, int *));
#endif /* sgi */
#ifndef _AIX
int select PROT((int, fd_set *, fd_set *, fd_set *, struct timeval *));
#endif
void bzero PROT((char *, int));
void telnet_neg PROT((char *, char *));
void set_prompt PROT((char *));
char *query_ip_number PROT((struct object *));
static void add_ip_entry PROT((long, char *));

extern char *xalloc(), *string_copy(), *unshared_str_copy();
extern int d_flag;
extern int current_time;
char * first_cmd_in_buf PROT((struct interactive *));
void next_cmd_in_buf PROT((struct interactive *));
char * skip_eols PROT((struct interactive *, char *));
void remove_flush_entry PROT((struct interactive *ip));

void remove_interactive();

extern void debug_message(), fatal(), free_sentence();

struct interactive *all_players[MAX_PLAYERS];

extern int errno;

void new_player();

void flush_all_player_mess();

fd_set readfds;
int nfds = 0;
int num_player;

FILE *f_ip_demon = NULL, *f_ip_demon_wr;

#ifdef ACCESS_RESTRICTED
extern void *allow_host_access (); 
extern void release_host_access ();
#endif

static struct object *first_player_for_flush=(struct object *)NULL;

/*
 * Interprocess communication interface to the backend.
 */

static int s;
extern int port_number;

#ifdef COMM_STAT
int add_message_calls=0;
int inet_packets=0;
int inet_volume=0;
#endif

void prepare_ipc() {
#ifndef MSDOS
    struct sockaddr_in sin;
    struct hostent *hp;
    int tmp;
    char host_name[100];
#ifdef MUDWHO
    char buff[100];
#endif

    if (gethostname(host_name, sizeof host_name) == -1) {
        perror("gethostname");
	fatal("Error in gethostname()\n");
    }
    hp = gethostbyname(host_name);
    if (hp == 0) {
	(void)fprintf(stderr, "gethostbyname: unknown host.\n");
	exit(1);
    }
    memset((char *)&sin, '\0', sizeof sin);
    memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
    sin.sin_port = htons((u_short)port_number);
    sin.sin_family = hp->h_addrtype;
    sin.sin_addr.s_addr = INADDR_ANY;
    s = socket(hp->h_addrtype, SOCK_STREAM, 0);
    if (s == -1) {
	perror("socket");
	abort();
    }
    tmp = 1;
    if (setsockopt (s, SOL_SOCKET, SO_REUSEADDR,
		    (char *) &tmp, sizeof (tmp)) < 0) {
	perror ("setsockopt");
	exit (1);
    }
    if (bind(s, (struct sockaddr *)&sin, sizeof sin) == -1) {
	if (errno == EADDRINUSE) {
	    fprintf(stderr, "Socket already bound!\n");
	    debug_message("Socket already bound!\n");
	    exit(errno);
	} else {
	    perror("bind");
	    abort();
	}
    }
    if (listen(s, 5) == -1) {
	perror("listen");
	abort();
    }
    tmp = 1;
/*
   DO_FCNTL is a macro that determines which of the following two methods
   for setting the socket non-blocking are used.  It should work as-is for
   most machines.   But some machines are weird and do different things
   for different releases of the OS.  If the driver fails here, then use
   "#if !DO_FCNTL" in the following line instead of "#if DO_FCNTL".
*/
#if DO_FCNTL
    if (fcntl(s, F_SETFL, FNDELAY) == -1) {
	perror("comm1.c:fnctl socket F_SETFL FNDELAY");
	exit(-1);
    }
#else /* this is the usual case */
    if (ioctl(s, FIONBIO, &tmp) == -1) {
	perror("comm1.c:ioctl socket FIONBIO");
	exit(-2);
    }
#endif
    signal(SIGPIPE, SIG_IGN);
#ifdef MUDWHO
    sprintf(buff, "MudOS driver %s%d", 
	    VERSION, PATCH_LEVEL);
    rwhocli_setup(MUDWHO_SERVER,MUDWHO_PASSWORD,MUDWHO_NAME,buff);
#endif
#else
    c_listen();
#endif
}

/*
 * This one is called when shutting down the MUD.
 */
void ipc_remove() {
    (void)printf("Shutting down ipc...\n");
#ifndef MSDOS
    close(s);
#ifdef MUDWHO
    rwhocli_shutdown();
#endif
#else
    c_shutdown();
#endif
}

/*
 * Send a message to a player. If that player is shadowed, special
 * care has to be taken.
 */
/*VARARGS1*/
void add_message(fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9)
    char *fmt;
    int a1, a2, a3, a4, a5, a6, a7, a8, a9;
{
    char buff[10000];		/* Kludgy! Hope this is enough ! */
    char buff2[MAX_SOCKET_PACKET_SIZE+1];
    struct interactive *ip;
    int n, chunk, length;
    int from, to;
    int min_length;
    int old_message_length;

    if (command_giver == 0 || (command_giver->flags & O_DESTRUCTED) ||
	command_giver->interactive == 0 ||
	command_giver->interactive->do_close) {
	putchar(']');
	if ( fmt != MESSAGE_FLUSH )
	    printf(fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9);
	fflush(stdout);
	return;
    }
    ip = command_giver->interactive;
    old_message_length = ip->message_length;
    if ( fmt == MESSAGE_FLUSH ) {
	min_length = 1;
	strncpy ( buff, ip->message_buf, length=old_message_length );
    	buff[length] = '\0';
    } else {
	min_length = DESIRED_SOCKET_PACKET_SIZE;
	(void)sprintf(buff+old_message_length,fmt,a1,a2,a3,a4,a5,a6,a7,a8,a9);
	length = old_message_length + strlen(buff+old_message_length);
	/*
	 * Always check that your arrays are big enough ! :-)
	 */
	if (length > sizeof buff)
	    fatal("Too long message!\n");
#ifndef NO_SHADOWS /* LPCA */
	if (shadow_catch_message(command_giver, buff+old_message_length))
	    return;
#endif NO_SHADOWS
#ifdef COMM_STAT
        add_message_calls++;
#endif
	if (ip->snoop_by) {
	    struct object *save = command_giver;
	    command_giver = ip->snoop_by->ob;
	    add_message("%%%s", buff+old_message_length);
	    command_giver = save;
	}
	if ( length >= min_length ) {
	    strncpy ( buff, ip->message_buf, old_message_length );
	} else {
	    strcpy( ip->message_buf+old_message_length,
		buff+old_message_length );
	}
    }
    if (d_flag > 1)
	debug_message("[%s(%d)]: %s", command_giver->name, length, buff);
    /*
     * Insert CR after all NL.
     */
    to = 0;
#ifdef PORTALS
    if (ip->out_portal) {
	buff2[0] = ']';
	to = 1;
    }
#endif
    for (from = 0; length-from >= min_length; to = 0 ) {
	for ( ; to < (sizeof buff2)-1 && buff[from] != '\0';) {
	    if (buff[from] == '\n')
		buff2[to++] = '\r';
#if 0
	    if (isprint(buff[from]) || isspace(buff[from]))
		buff2[to++] = buff[from++];
	    else
		from++;
#else
	    buff2[to++] = buff[from++];
#endif
	}
	if ( to == sizeof(buff2) ) {
	    to -= 2;
	    from--;
	}
	chunk = to;
    /*
     * We split up the message into something smaller than the max size.
     */
	if ((n = write(ip->socket, buff2, chunk)) == -1) {
	    if (errno == EMSGSIZE) {
		fprintf(stderr, "comm1: write EMSGSIZE.\n");
		return;
	    }
	    if (errno == EINVAL) {
		fprintf(stderr, "comm1: write EINVAL.\n");
	        if (old_message_length) remove_flush_entry(ip);
		ip->do_close = 1;
		return;
	    }
	    if (errno == ENETUNREACH) {
		fprintf(stderr, "comm1: write ENETUNREACH.\n");
	        if (old_message_length) remove_flush_entry(ip);
		ip->do_close = 1;
		return;
	    }
	    if (errno == EHOSTUNREACH) {
		fprintf(stderr, "comm1: write EHOSTUNREACH.\n");
	        if (old_message_length) remove_flush_entry(ip);
		ip->do_close = 1;
		return;
	    }
	    if (errno == EPIPE) {
		fprintf(stderr, "comm1: write EPIPE detected\n");
	        if (old_message_length) remove_flush_entry(ip);
		ip->do_close = 1;
		return;
	    }
	    if (errno == EWOULDBLOCK) {
		fprintf(stderr, "comm1: write EWOULDBLOCK. Message discarded.\n");
	        if (old_message_length) remove_flush_entry(ip);
/*		ip->do_close = 1;  -- LA */
		return;
	    }
	    fprintf(stderr, "write: unknown errno %d\n", errno);
	    perror("write");
	    if (old_message_length) remove_flush_entry(ip);
		ip->do_close = 1;
	    return;
	}
#ifdef COMM_STAT
	inet_packets++;
	inet_volume += n;
#endif
	if (n != chunk)
	    fprintf(stderr, "write socket: wrote %d, should be %d.\n",
		    n, chunk);
	continue;
    }
    length -= from;
    ip->message_length=length;
    if (from)
        strncpy( ip->message_buf, buff+from, length );
    if ( length && !old_message_length ) { /* buffer became 'dirty' */
	if ( ip->next_player_for_flush = first_player_for_flush ) {
	    first_player_for_flush->interactive->previous_player_for_flush =
		command_giver;
	}
	ip->previous_player_for_flush = 0;
	first_player_for_flush = command_giver;
    }
    if ( !length && old_message_length ) { /* buffer has become empty */
	remove_flush_entry(ip);
    }
}

void remove_flush_entry(ip)
    struct interactive *ip;
{

    ip->message_length=0;
    if ( ip->previous_player_for_flush ) {
	ip->previous_player_for_flush->interactive->next_player_for_flush
	= ip->next_player_for_flush;
    } else {
	first_player_for_flush = ip->next_player_for_flush;
    }
	if ( ip->next_player_for_flush ) {
	ip->next_player_for_flush->interactive->previous_player_for_flush
	= ip->previous_player_for_flush;
    }
}

void flush_all_player_mess() {
    struct object *p,*np;
    struct object *save = command_giver;

    for ( p = first_player_for_flush; p; p=np) {
        np = p->interactive->next_player_for_flush;
	/* beware of side-effects when calling add_message the first time! */
	command_giver = p;
	add_message(MESSAGE_FLUSH);
    }
    command_giver=save;
}


/*
 * Copy a string, replacing newlines with '\0'. Also add an extra
 * space and back space for every newline. This trick will allow
 * otherwise empty lines, as multiple newlines would be replaced by
 * multiple zeroes only.
 */
static int copy_chars(from, to, n, ip)
    char *from, *to;
    int n;
    struct interactive *ip;
{
    int i;
    char *start = to;
    for (i=0; i<n; i++) {
	if (from[i] == '\r')
	    continue;
	if (from[i] == '\n') {
	    *to++ = ' ';
	    *to++ = '\b';
	    *to++ = '\0';
	    continue;
	}
	*to++ = from[i];
	/* single character mode */
	/* ack! special case so we don't pick up flow control chars */
	if (ip->single_char && from[0] != '\377')
	  *to++ = '\0';
    }
    return to - start;
}

/*
 * Get a message from any player.  For all players without a completed
 * cmd in their input buffer, read their socket.  Then, return the first
 * cmd of the next player in sequence that has a complete cmd in their buffer.
 * CmdsGiven is used to allow people in ED to send more cmds (if they have
 * them queued up) than normal players.  If we get a heartbeat, still read
 * all sockets; if the next cmd giver is -1, we have already cycled and
 * can go back to do the heart beat.
 */

#define	StartCmdGiver	(MAX_PLAYERS-1) /* the one after heartbeat */
#define IncCmdGiver	NextCmdGiver = (NextCmdGiver < 0? StartCmdGiver: \
					NextCmdGiver - 1)

int NextCmdGiver;
int CmdsGiven = 0;	 /* -1 is used to poll heart beat. */
int twait = 0;		 /* wait time for select() */
extern int time_to_call_heart_beat, comm_time_to_call_heart_beat;

int get_message(buff, size)
    char *buff;
    int size;
{
    int i, res;
    struct interactive * ip = 0;
    char *p;

    /*
     * Stay in this loop until we have a message from a player.
     */
    while(1) {
	int new_socket;
	struct sockaddr_in addr;
	int length;
	struct timeval timeout;

	/* First, try to get a new player... */
	length = sizeof addr;
	new_socket = accept(s, (struct sockaddr *)&addr, &length);
	if (new_socket != -1)
	    new_player(new_socket, &addr, length);
	else if (new_socket == -1 && errno != EWOULDBLOCK && errno != EINTR) {
	    perror("accept");
	    abort();
	}

	nfds = 0;
	FD_ZERO(&readfds);
	for (i=0; i<MAX_PLAYERS; i++) {
	    ip = all_players[i];
	    if (!ip)
		continue;
	    if (ip->do_close) {
		ip->do_close = 0;
		remove_interactive(ip->ob);
		continue;
	    }
	    if (!first_cmd_in_buf(ip)) {
		FD_SET(ip->socket, &readfds);
		if (ip->socket >= nfds)
		    nfds = ip->socket+1;
#ifdef PORTALS
		if (ip->out_portal) {
		    FD_SET(ip->portal_socket, &readfds);
		    if (ip->portal_socket >= nfds)
			nfds = ip->portal_socket + 1;
		}
#endif /* PORTALS */
	    }
	}
	if (f_ip_demon != NULL) {
	    FD_SET(fileno(f_ip_demon), &readfds);
	}
	timeout.tv_sec = twait; /* avoid busy waiting when no buffered cmds */
	timeout.tv_usec = 0;
	res = select(nfds, &readfds, 0, 0, &timeout);
#ifdef MSDOS
	if (timer_expired()) {
	    twait = 0;
	    goto return_next_command;
	}
#endif
	if (res == -1) {
	    twait = 0;
	    if (errno == EINTR) /* if we got an alarm, finish the round */
		goto return_next_command;
	    perror("select");
	    return 0;
	}
	if (res) { /* waiting packets */
	    if (f_ip_demon != NULL && FD_ISSET(fileno(f_ip_demon), &readfds)) {
		char buf[200], *pp, *q;
		long laddr;
		if (fgets(buf, sizeof buf, f_ip_demon)) {
/*printf("hname says: %s\n", buf);*/
		    laddr=inet_addr(buf);
		    if (laddr != -1) {
			pp = strchr(buf, ' ');
			if (pp) {
			    pp++;
			    q = strchr(buf, '\n');
			    if (q) {
				*q = 0;
				add_ip_entry(laddr, pp);
			    }
			}
		    }
		}
	    }
	    for (i=0; i<MAX_PLAYERS; i++) { /* read all pending sockets */
		ip = all_players[i];
		if (ip == 0)
		    continue;
		if (FD_ISSET(ip->socket, &readfds)) { /* read this player */
		    int l;

		    /*
		     * Don't overfill their buffer.
		     * Use a very conservative estimate on how much we can
		     * read.
		     */
		    l = (MAX_TEXT - ip->text_end - 1)/3;
		    if (l < size)
			size = l;

		    if ((l = read(ip->socket, buff, size)) == -1) {
			if (errno == ENETUNREACH) {
			    debug_message("Net unreachable detected.\n");
			    remove_interactive(ip->ob);
			    continue;
			}
			if (errno == EHOSTUNREACH) {
			    debug_message("Host unreachable detected.\n");
			    remove_interactive(ip->ob);
			    continue;
			}
			if (errno == ETIMEDOUT) {
			    debug_message("Connection timed out detected.\n");
			    remove_interactive(ip->ob);
			    continue;
			}
			if (errno == ECONNRESET) {
			    debug_message("Connection reset by peer detected.\n");
			    remove_interactive(ip->ob);
			    continue;
			}
			if (errno == EWOULDBLOCK) {
			    debug_message("read would block socket %d!\n",
					  ip->socket);
			    remove_interactive(ip->ob);
			    continue;
			}
			if (errno == EMSGSIZE) {
			    debug_message("read EMSGSIZE !\n");
			    continue;
			}
			perror("read");
			debug_message("Unknown errno %d\n", errno);
			remove_interactive(ip->ob);
			continue;
		    }
#ifdef PORTALS
		    /*
		     * IF the data goes through a portal, send it,
		     * but don't return any data.
		     */
		    if (ip->out_portal) {
			if (ip->text_end) { /* pending text first */
			    write(ip->portal_socket, ip->text, ip->text_end);
			    ip->text_end = 0;
			}
			if (l)
			    write(ip->portal_socket, buff, l);
			continue;
		    }
#endif /* PORTALS */
		    if (l == 0) {
			if (ip->closing)
			    fatal("Tried to read from closing socket.\n");
			remove_interactive(ip->ob);
			return 0;
		    }
		    buff[l] = '\0';
		    /* replace newlines by nulls and catenate to buffer */
		    ip->text_end +=
			copy_chars(buff, ip->text + ip->text_end, l, ip);
#if 0
		    for (x=0; x<l; x++) {
			if (buff[x] == '\n' || buff[x] == '\r')
			    buff[x] = '\0';
		    }
		    memcpy(ip->text+ip->text_end, buff, l+1);
		    ip->text_end += l;
#endif
		    /* now, text->end is just after the last char read.  If last */
		    /* char was a nl, char *before* text_end will be null. */
		    ip->text[ip->text_end] = '\0';
		}
	    }
	}
	/*
	 * we have read the sockets; now find and return a command
	 */
    return_next_command:
	twait = 0;
	ip = 0;
	for (i = 0; i < MAX_PLAYERS + 1; i++) {
	    if (NextCmdGiver == -1) { /* we have cycled around all players */
		CmdsGiven = 0;	  /* check heart beat */
		IncCmdGiver;
		if (comm_time_to_call_heart_beat) {
		    time_to_call_heart_beat = 1; /* twait stays 0! */
		    return 0;
		}
	    }
	    ip = all_players[NextCmdGiver];
	    if (ip && (p = first_cmd_in_buf(ip))) /* wont respond to partials */
		break;
	    CmdsGiven = 0; /* new player, no cmds issued */
	    IncCmdGiver;
	}
	
	if ((!ip)||!p) { /* no cmds found; loop and select (on timeout) again */
	    if (comm_time_to_call_heart_beat) { /* may as well do it now */
		time_to_call_heart_beat = 1; /* no cmds, do heart beat */
		NextCmdGiver = StartCmdGiver;/* do a complete poll next time */
		CmdsGiven = 0;
		return(0);
	    }
	    /* no heart beat to do and no cmds pending - avoid busy wait on select */
	    twait = 1;
	    continue; /* else await another cmd */
	}
	
	/*
	 * we have a player cmd - return it.  If he is in ed, count his
	 * cmds, else only allow 1 cmd.  If he has only one partially
	 * completed cmd left after * this, move it to the start of his
	 * buffer; new stuff will be appended.
	 */
	
	command_giver = ip->ob;
	telnet_neg(buff, p);

	next_cmd_in_buf(ip); /* move on buffer pointers */
	
	/* if he is not in ed, dont let him issue another till the poll comes again */
	
	if (ip->ed_buffer && CmdsGiven < ALLOWED_ED_CMDS)
	    CmdsGiven++;
	else {
	    IncCmdGiver;
	    CmdsGiven = 0;
	}
	
	/* manage snooping - should the snooper see type ahead?  Well, he doesn't here
	 */
	if (ip->snoop_by && !ip->noecho) {
	    command_giver = ip->snoop_by->ob;
	    add_message("%% %s\n", buff);
	}
	command_giver = ip->ob;
	if (ip->noecho) {
	    /* Must not enable echo before the user input is received. */
	    add_message("%c%c%c", IAC, WONT, TELOPT_ECHO);
	}
	ip->noecho = 0;
	ip->last_time = current_time;
	return 1;
    }
}

/*
 * find the first character of the next complete cmd in a buffer, 0 if no
 * completed cmd.  There is a completed cmd if there is a null between
 * text_start and text_end.  Zero length commands are discarded (as occur
 * between <cr> and <lf>).  Update text_start if we have to skip leading
 * nulls.
 */

char * first_cmd_in_buf(ip) 
struct interactive *ip;
{
    char * p, *q;

    p = ip->text + ip->text_start;

    while ((p < (ip->text + ip->text_end)) && !*p) /* skip null input */
	p++;

    ip->text_start = p - ip->text;

    if (ip->text_start >= ip->text_end) {
	ip->text_start = ip->text_end = 0;
	ip->text[0] = '\0';
	return(0);
    }

    while ((p < (ip->text + ip->text_end)) && *p) /* find end of cmd */
	p++;

    if (p < ip->text + ip->text_end) /* null terminated, was command */
	return(ip->text + ip->text_start);

/* have a partial command at end of buffer; move it to start, return null */
/* if it can't move down, truncate it and return it as cmd. */
    
    p = ip->text + ip->text_start;
    q = ip->text;
    while (p < (ip->text + ip->text_end))
	*(q++) = *(p++);

    ip->text_end -= ip->text_start;
    ip->text_start = 0;
    if (ip->text_end > MAX_TEXT - 2) {
	ip->text[ip->text_end-2] = '\0'; /* nulls to truncate */
	ip->text[ip->text_end-1] = '\0'; /* nulls to truncate */
	ip->text_end--;
	return(ip->text);
	}
/* buffer not full and no newline - no cmd. */
    return(0);
}

/*
 * move pointers to next cmd, or clear buf.
 */

void next_cmd_in_buf(ip)
struct interactive *ip;
{
    char * p = ip->text + ip->text_start;
    while (*p && p < ip->text + ip->text_end)
	p++;
    /* skip past any nulls at the end */
    while (!*p && p < ip->text + ip->text_end)
	p++;
    if (p < ip->text + ip->text_end)
	ip->text_start = p - ip->text;
    else {
	ip->text_start = ip->text_end = 0;
	ip->text[0] = '\0';
    }
}
    

/*
 * Remove an interactive player immediately.
 */
void remove_interactive(ob)
    struct object *ob;
{
    struct object *save = command_giver;
    int i;

    for (i=0; i<MAX_PLAYERS; i++) {
	if (all_players[i] != ob->interactive)
	    continue;
	if (ob->interactive->closing)
	    fatal("Double call to remove_interactive()\n");
	ob->interactive->closing = 1;

/* wayfarer - net-death notification */
 
        if (!(ob->flags & O_DESTRUCTED))
            apply("net_dead",ob,0);

	if (ob->interactive->snoop_by) {
	    ob->interactive->snoop_by->snoop_on = 0;
	    ob->interactive->snoop_by = 0;
	}
	if (ob->interactive->snoop_on) {
	    ob->interactive->snoop_on->snoop_by = 0;
	    ob->interactive->snoop_on = 0;
	}
	command_giver = ob;
	add_message("Closing down.\n");
	if (ob->interactive->ed_buffer) {
	    extern void save_ed_buffer();

	    save_ed_buffer();
	}
	add_message(MESSAGE_FLUSH);
	(void)shutdown(ob->interactive->socket, 2);
	close(ob->interactive->socket);
#ifdef ACCESS_RESTRICTED
        release_host_access (ob->interactive->access_class);
#endif
	num_player--;
	if (ob->interactive->input_to) {
	    free_object(ob->interactive->input_to->ob, "remove_interactive");
	    free_sentence(ob->interactive->input_to);
	    ob->interactive->input_to = 0;
	}
	FREE((char *)ob->interactive);
	ob->interactive = 0;
	all_players[i] = 0;
	free_object(ob, "remove_interactive");
	command_giver = save;
	return;
    }
    (void)fprintf(stderr, "Could not find and remove player %s\n", ob->name);
    abort();
}

#ifndef MSDOS
#ifndef ACCESS_RESTRICTED

int
allow_host_access(new_socket)
    int new_socket;
{
    struct sockaddr_in apa;
    int len = sizeof apa;
    char * ipname, *CALLOC(), *xalloc(), *index();
    static int read_access_list = 0;
    static struct access_list {
	int addr_len;
	char * addr, *name, *comment;
	struct access_list * next;
    } * access_list;
    register struct access_list * ap;

    if(!read_access_list) {
	FILE * f = fopen("ACCESS.DENY", "r");
	char buf[1024], ipn[50], hname[100], comment[1024], *p1, *p2;
	struct access_list * na;
	struct hostent * hent;

	read_access_list = 1;
	if(f) {
	    while(fgets(buf, sizeof buf - 1, f)) {
		if(*buf != '#') {
		    ipn[0] = hname[0] = comment[0] = 0;
		    if(p1 = index(buf, ':')) *p1 = 0;
		    if(buf[0] && buf[0] != '\n')
			strncpy(ipn, buf, sizeof ipn - 1);
		    if((p2 = p1) && *++p2) {
			if(p1 = index(p2, ':')) *p1 = 0;
			if(p2[0] && p2[0] != '\n')
			    strcpy(hname, p2);
			if(p1 && p1[1] && p1[1] != '\n')
			    strcpy(comment, p1+1);
		    }
			
		    if(!(na = (struct access_list *)xalloc(sizeof na[0]))) {
			fatal("Out of mem.\n");
		    }
		    na->addr = na->name = na->comment = 0;
		    na->next = 0;
		    if(*ipn && (!(na->addr = xalloc(strlen(ipn) + 1)) ||
				!strcpy(na->addr, ipn)))
			fatal("Out of mem.\n");
		    if(*hname && (!(na->name = xalloc(strlen(hname) + 1)) ||
				  !strcpy(na->name, hname)))
			fatal("Out of mem.\n");
		    if(*comment && (!(na->comment=xalloc(strlen(comment)+1))||
				    !strcpy(na->comment, comment)))
			fatal("Out of mem.\n");

		    if((!(int)*ipn)
			&&
			((!*hname)
			  || (!(hent = gethostbyname(hname))) ||
				 (!(na->addr =
				   xalloc(hent->h_length+1)))||
				 !strcpy(na->addr,
					inet_ntoa(*(struct in_addr *)hent->h_addr)))) {
			if(na->name) FREE(na->name);
			if(na->comment) FREE(na->comment);
			FREE((char *)na);
			continue;
		    }
		    if(!(na->addr_len = strlen(na->addr)))
			continue;

		    /* printf("disabling: %s:%s:%s\n", na->addr,
			   na->name?na->name:"no name",
			   na->comment?na->comment:"no comment");  */

		    na->next = access_list;
		    access_list = na;
		}
	    }
	    fclose(f);
	}
    }
    
    if (!access_list)
	return 0;
	
    if(getpeername(new_socket, (struct sockaddr *)&apa, &len) == -1) {
	close(new_socket);
	perror("getpeername");
	return -1;
    }

    ipname = inet_ntoa(apa.sin_addr);
    
    for(ap = access_list; ap; ap = ap->next)
	if(!strncmp(ipname, ap->addr, ap->addr_len)){
	    if(ap->comment) (void) write(new_socket, ap->comment,
					 strlen(ap->comment));
	    printf("Stopping: %s:%s\n", ap->addr, ap->name?ap->name:"no name");
	    close(new_socket);
	    return -1;
	}
    return 0;
}
#endif /* not ACCESS_RESTRICTED */
#endif

/*
 * get the I'th player object from the interactive list, i starts at 0
 * and can go to num_player - 1.  For users(), etc.
 */
struct object * get_interactive_object(i)
int i;
{
    int n;

    if (i >= num_player) /* love them ASSERTS() :-) */
	fatal("Get interactive (%d) with only %d players!", i, num_player);

    for (n = 0; n < MAX_PLAYERS; n++)
	if (all_players[n])
	    if (!(i--))
		return(all_players[n]->ob);

    fatal("Get interactive: player %d not found! (num_players = %d)",
		i, num_player);
    return 0;	/* Just to satisfy some compiler warnings */
}

void new_player(new_socket, addr, len)
    int new_socket;
    struct sockaddr_in *addr;
    int len;
{
    int i;
    char *p;
   
#ifndef MSDOS
#ifdef ACCESS_RESTRICTED
    void *class;

    if (!(class = allow_host_access (new_socket, new_socket)))
      return;
#else
    if(allow_host_access(new_socket))
	return;
#endif
#endif
    if (d_flag > 1)
	debug_message("New player at socket %d.\n", new_socket);
    for (i=0; i<MAX_PLAYERS; i++) {
	struct object *ob;
	struct svalue *ret;
	extern struct object *master_ob;
	
	if (all_players[i] != 0)
	    continue;
	assert_master_ob_loaded();
	command_giver = master_ob;
	master_ob->interactive =
	    (struct interactive *)xalloc(sizeof (struct interactive));
	master_ob->interactive->default_err_message = 0;
	master_ob->flags |= O_ONCE_INTERACTIVE;
	/* This initialization is not pretty. */
	master_ob->interactive->ob = master_ob;
	master_ob->interactive->text[0] = '\0';
	master_ob->interactive->input_to = 0;
	master_ob->interactive->closing = 0;
	master_ob->interactive->snoop_on = 0;
	master_ob->interactive->snoop_by = 0;
#ifdef PORTALS
	master_ob->interactive->out_portal = 0;
	master_ob->interactive->portal_socket = 0;
	master_ob->interactive->from_portal = 0;
#endif /* PORTALS */
	master_ob->interactive->text_end = 0;
	master_ob->interactive->text_start = 0;
	master_ob->interactive->do_close = 0;
	master_ob->interactive->noecho = 0;
        master_ob->interactive->noesc = 0;
	master_ob->interactive->trace_level = 0;
	master_ob->interactive->trace_prefix = 0;
	master_ob->interactive->last_time = current_time;
	master_ob->interactive->ed_buffer = 0;
	master_ob->interactive->message_length=0;
#ifdef MUDWHO
        master_ob->interactive->login_time = current_time;
#endif
	all_players[i] = master_ob->interactive;
	all_players[i]->socket = new_socket;
	set_prompt("> ");
#if 1
	memcpy((char *)&all_players[i]->addr, (char *)addr, len);
#else
	getpeername(new_socket, (struct sockaddr *)&all_players[i]->addr,
		    &len);
#endif

#ifdef ACCESS_RESTRICTED
        all_players[i]->access_class = class;
#endif
	num_player++;
	/*
	 * The player object has one extra reference.
	 * It is asserted that the master_ob is loaded.
	 */
	add_ref(master_ob, "new_player");
	ret = apply_master_ob("connect", 0);
	if (ret == 0 || ret->type != T_OBJECT) {
	    remove_interactive(master_ob);
	    return;
	}
	/*
	 * There was an object returned from connect(). Use this as the
	 * player object.
	 */
	ob = ret->u.ob;
	ob->interactive = master_ob->interactive;
	ob->interactive->ob = ob;
	ob->flags |= O_ONCE_INTERACTIVE;

	/*
	  assume the existance of write_prompt and process_input in player.c
	  until proven wrong (after trying to call them)
	*/
	ob->interactive->has_write_prompt = 1;
	ob->interactive->has_process_input = 1;

	master_ob->flags &= ~O_ONCE_INTERACTIVE;
	add_message(MESSAGE_FLUSH);
	master_ob->interactive = 0;
	free_object(master_ob, "reconnect");
	add_ref(ob, "new_player");
	command_giver = ob;
        if (f_ip_demon_wr != NULL) {
/*printf("sent hname %s\n:", query_ip_number(ob));*/
            fprintf(f_ip_demon_wr, "%s\n", query_ip_number(ob));
            fflush(f_ip_demon_wr);
        }
	logon(ob);
	flush_all_player_mess();
	return;
    }
    p = "Lpmud is full. Come back later.\r\n";
    write(new_socket, p, strlen(p));
    close(new_socket);
}

int call_function_interactive(i, str)
    struct interactive *i;
    char *str;
{
    char *function;
    struct object *ob;

    i->noesc = 0;
    if (!i->input_to)
	return 0;
    /*
     * Special feature: input_to() has been called to setup
     * a call to a function.
     */
    if (i->input_to->ob->flags & O_DESTRUCTED) {
	/* Sorry, the object has selfdestructed ! */
	free_object(i->input_to->ob, "call_function_interactive");
	free_sentence(i->input_to);
	i->input_to = 0;
	return 0;
    }
    free_object(i->input_to->ob, "call_function_interactive");
    function = string_copy(command_giver->interactive->input_to->function);
    ob = i->input_to->ob;
    free_sentence(i->input_to);
    /*
     * We must clear this reference before the call to apply(), because
     * someone might want to set up a new input_to().
     */
    i->input_to = 0;
    /*
     * clear single character mode
     */
    i->single_char = 0;
    add_message("%c%c%c", IAC, WONT, TELOPT_SGA);
    /*
     * Now we set current_object to this object, so that input_to will
     * work for static functions.
     */
    push_constant_string(str);
    current_object = ob;
    (void)apply(function, ob, 1);
    FREE(function);
    flush_all_player_mess();
    return 1;
}

int set_call(ob, sent, flags, single_char)
    struct object *ob;
    struct sentence *sent;
    int flags;
    int single_char;
{
    struct object *save = command_giver;
    if (ob == 0 || sent == 0)
	return 0;
    if (ob->interactive == 0 || ob->interactive->input_to)
	return 0;
    ob->interactive->input_to = sent;
    ob->interactive->noecho = (flags & I_NOECHO != 0);
    ob->interactive->noesc = (flags & I_NOESC != 0);
    ob->interactive->single_char = single_char;
    command_giver = ob;
    if (flags & I_NOECHO)
	add_message("%c%c%c", IAC, WILL, TELOPT_ECHO);
    if (single_char)
      add_message("%c%c%c", IAC, WILL, TELOPT_SGA);
    command_giver = save;
    return 1;
}

void show_info_about(str, room, i)
    char *str, *room;
    struct interactive *i;
{
    struct hostent *hp = 0;

#if 0
    hp = gethostbyaddr(&i->addr.sin_addr.s_addr, 4, AF_INET);
#endif
    add_message("%-15s %-15s %s\n",
		hp ? hp->h_name : inet_ntoa(i->addr.sin_addr), str, room);
}

void remove_all_players()
{
    int i;

    for (i=0; i<MAX_PLAYERS; i++) {
	if (all_players[i] == 0)
	    continue;
	command_giver = all_players[i]->ob;
	(void)apply("quit", all_players[i]->ob, 0);
    }
}

void set_prompt(str)
    char *str;
{
    command_giver->interactive->prompt = str;
}

/*
 * Print the prompt, but only if input_to not is disabled.
 */
void print_prompt()
{
    if (command_giver == 0)
		fatal("command_giver == 0.\n");
    if (command_giver->interactive->input_to == 0) {
      /* give player obj a chance to write its own prompt */
		if (!command_giver->interactive->has_write_prompt)
			add_message(command_giver->interactive->prompt);
	   else if (command_giver->interactive &&
				command_giver->interactive->ed_buffer)
			add_message(command_giver->interactive->prompt);
		else if (!(command_giver->flags & O_DESTRUCTED) &&
			!apply("write_prompt",command_giver,0)) {
			command_giver->interactive->has_write_prompt = 0;
			add_message(command_giver->interactive->prompt);
		}
		if (1) { /* add test for heart_beat later */
			   flush_all_player_mess();
		}
    }
}

/*
 * Let object 'me' snoop object 'you'. If 'you' is 0, then turn off
 * snooping.
 */
void set_snoop(me, you)
    struct object *me, *you;
{
    struct interactive *on = 0, *by = 0, *tmp;
    int i;

    if (me->flags & O_DESTRUCTED)
	return;
    if (you && (you->flags & O_DESTRUCTED))
	return;
    for(i=0; i<MAX_PLAYERS && (on == 0 || by == 0); i++) {
	if (all_players[i] == 0)
	    continue;
	if (all_players[i]->ob == me)
	    by = all_players[i];
	else if (all_players[i]->ob == you)
	    on = all_players[i];
    }
    if (you == 0) {
	if (by == 0)
	    error("Could not find myself to stop snoop.\n");
	add_message("Ok.\n");
	if (by->snoop_on == 0)
	    return;
	by->snoop_on->snoop_by = 0;
	by->snoop_on = 0;
	return;
    }
    if (on == 0 || by == 0) {
	add_message("Failed.\n");
	return;
    }
    if (by->snoop_on) {
	by->snoop_on->snoop_by = 0;
	by->snoop_on = 0;
    }
    if (on->snoop_by) {
	add_message("Busy.\n");
	return;
    }
    /*
     * Protect against snooping loops.
     */
    for (tmp = on; tmp; tmp = tmp->snoop_on) {
	if (tmp == by) {
	    add_message("Busy.\n");
	    return;
	}
    }
    on->snoop_by = by;
    by->snoop_on = on;
    add_message("Ok.\n");
    return;
}

/*
 * Let object 'me' snoop object 'you'. If 'you' is 0, then turn off
 * snooping.
 *
 * This routine is almost identical to the old set_snoop. The main
 * difference is that the routine writes nothing to player directly,
 * all such communication is taken care of by the mudlib. It communicates
 * with master.c in order to find out if the operation is permissble or
 * not. The old routine let everyone snoop anyone. This routine also returns
 * 0 or 1 depending on success.
 */
int new_set_snoop(me, you)
    struct object *me, *you;
{
    struct interactive *on = 0, *by = 0, *tmp;
    int i;
    struct svalue *ret;

    /* Stop if people managed to quit before we got this far */
    if (me->flags & O_DESTRUCTED)
	return 0;
    if (you && (you->flags & O_DESTRUCTED))
	return 0;

    /* Find the snooper & snopee */
    for(i = 0 ; i < MAX_PLAYERS && (on == 0 || by == 0); i++) 
    {
	if (all_players[i] == 0)
	    continue;
	if (all_players[i]->ob == me)
	    by = all_players[i];
	else if (all_players[i]->ob == you)
	    on = all_players[i];
    }

    /* Illegal snoop attempt by null object */
    if (!current_object->eff_user)
	return 0;

    /* Check for permissions with valid_snoop in master */
    push_object(me);
    if (you == 0)
	push_number(0);
    else
	push_object(you);
    ret = apply_master_ob("valid_snoop", 2);

    if (!ret || ret->type != T_NUMBER || ret->u.number == 0)
	return 0;

    /* Stop snoop */
    if (you == 0) 
    {
	if (by == 0)
	    error("Could not find snooper to stop snoop on.\n");
	if (by->snoop_on == 0)
	    return 1;
	by->snoop_on->snoop_by = 0;
	by->snoop_on = 0;
	return 1;
    }

    /* Strange event, but possible, so test for it */
    if (on == 0 || by == 0)
	return 0;

    /* Protect against snooping loops */
    for (tmp = on; tmp; tmp = tmp->snoop_on) 
    {
	if (tmp == by) 
	    return 0;
    }

    /* Terminate previous snoop, if any */
    if (by->snoop_on) 
    {
	by->snoop_on->snoop_by = 0;
	by->snoop_on = 0;
    }
    if (on->snoop_by)
    {
	on->snoop_by->snoop_on = 0;
	on->snoop_by = 0;
    }

    on->snoop_by = by;
    by->snoop_on = on;
    return 1;
    
}

#define	TS_DATA		0
#define	TS_IAC		1
#define	TS_WILL		2
#define	TS_WONT		3
#define	TS_DO		4
#define	TS_DONT		5

void telnet_neg(to, from)
    char *to, *from;
{
    int state = TS_DATA;
    int ch;
    char *first = to;

    while(1) {
	ch = (*from++ & 0xff);
	switch(state) {
	case TS_DATA:
	    switch(ch) {
	    case IAC:
		state = TS_IAC;
		continue;
	    case '\b':	/* Backspace */
	    case 0x7f:	/* Delete */
		if (to <= first)
		    continue;
		to -= 1;
		continue;
	    default:
		if (ch & 0x80) {
		    if (d_flag) debug_message("Tel_neg: 0x%x\n", ch);
		    continue;
		}
		*to++ = ch;
		if (ch == 0)
		    return;
		continue;
	    }
	case TS_IAC:
	    switch(ch) {
	    case WILL:
		state = TS_WILL;
		continue;
	    case WONT:
		state = TS_WONT;
		continue;
	    case DO:
		state = TS_DO;
		continue;
	    case DONT:
		state = TS_DONT;
		continue;
	    case DM:
		break;
	    case NOP:
	    case GA:
	    default:
		break;
	    }
	    state = TS_DATA;
	    continue;
	case TS_WILL:
	    if (d_flag) debug_message("Will %s\n", telopts[ch]);
	    state = TS_DATA;
	    continue;
	case TS_WONT:
	    if (d_flag) debug_message("Wont %s\n", telopts[ch]);
	    state = TS_DATA;
	    continue;
	case TS_DO:
	    if (d_flag) debug_message("Do %s\n", telopts[ch]);
	    state = TS_DATA;
	    continue;
	case TS_DONT:
	    if (d_flag) debug_message("Dont %s\n", telopts[ch]);
	    state = TS_DATA;
	    continue;
	default:
	    if (d_flag) debug_message("Bad state: 0x%x\n", state);
	    state = TS_DATA;
	    continue;
	}
    }
}

#define IPSIZE 200
static struct ipentry {
    long addr;
    char *name;
} iptable[IPSIZE];
static int ipcur;

char *query_ip_name(ob)
    struct object *ob;
{
    int i;

    if (ob == 0)
	ob = command_giver;
    if (!ob || ob->interactive == 0)
	return 0;
    for(i = 0; i < IPSIZE; i++) {
	if (iptable[i].addr == ob->interactive->addr.sin_addr.s_addr &&
	    iptable[i].name)
	    return iptable[i].name;
    }
    return inet_ntoa(ob->interactive->addr.sin_addr);
}

static void add_ip_entry(addr, name)
long addr;
char *name;
{
    int i;

    for(i = 0; i < IPSIZE; i++) {
	if (iptable[i].addr == addr)
	    return;
    }
    iptable[ipcur].addr = addr;
    if (iptable[ipcur].name)
	free_string(iptable[ipcur].name);
    iptable[ipcur].name = make_shared_string(name);
    ipcur = (ipcur+1) % IPSIZE;
}

char *query_ip_number(ob)
    struct object *ob;
{
    if (ob == 0)
	ob = command_giver;
    if (!ob || ob->interactive == 0)
	return 0;
    return inet_ntoa(ob->interactive->addr.sin_addr);
}

#ifndef INET_NTOA_OK
/*
Note: if the address string is "a.b.c.d" the address number is
      a * 256^3 + b * 256^2 + c * 256 + d

*/

char *inet_ntoa(ad)
    struct in_addr ad;
{
    u_long s_ad;
    int a, b, c, d;
    static char addr[20]; /* 16 + 1 should be enough */

    s_ad = ad.s_addr;
    d = s_ad % 256;
    s_ad /= 256;
    c = s_ad % 256;
    s_ad /= 256;
    b = s_ad % 256;
    a = s_ad / 256;
    sprintf(addr, "%d.%d.%d.%d", a, b, c, d);
    return addr;
}
#endif /* INET_NTOA_OK */

char *query_host_name() {
    static char name[20];
    
    gethostname(name, sizeof name);
    name[sizeof name - 1] = '\0';	/* Just to make sure */
    return name;
}

struct object *query_snoop(ob)
    struct object *ob;
{
    if (ob->interactive->snoop_by == 0)
	return 0;
    return ob->interactive->snoop_by->ob;
}

int query_idle(ob)
    struct object *ob;
{
    if (!ob->interactive)
	error("query_idle() of non-interactive object.\n");
    return current_time - ob->interactive->last_time;
}

void notify_no_command() {
    char *p,*m;

    if (!command_giver->interactive)
	return;
    p = command_giver->interactive->default_err_message;
    if (p) {
	m = process_string(p); /* We want 'value by function call' /JnA */
#ifndef NO_SHADOWS
	if (!shadow_catch_message(command_giver, m))
#endif
	    add_message(m);
	if (m != p)
	    FREE(m);
	free_string(p);
	command_giver->interactive->default_err_message = 0;
    }
    else {
	add_message("What ?\n");
    }
}

void clear_notify() {
    if (!command_giver->interactive)
	return;
    if (command_giver->interactive->default_err_message) {
	free_string(command_giver->interactive->default_err_message);
	command_giver->interactive->default_err_message = 0;
    }
}

void set_notify_fail_message(str)
    char *str;
{
    if (!command_giver || !command_giver->interactive)
	return;
    clear_notify();
    if (command_giver->interactive->default_err_message)
	free_string(command_giver->interactive->default_err_message);
    command_giver->interactive->default_err_message = make_shared_string(str);
}

int replace_interactive(ob, obfrom, /*IGN*/name)
    struct object *ob;
    struct object *obfrom;
    char *name;
{
    /* marion
     * i see no reason why to restrict this, besides - the length
     * (was) missing to strncmp()
     * JnA: There is every reason to restrict this.
     *      Otherwise I can write my own player object without any security
     *      at all!
     */
    struct svalue *v;

    push_string(name, STRING_CONSTANT);
    v = apply_master_ob("valid_exec", 1);
    if (!v || v->type != T_NUMBER || v->u.number == 0)
	return 0;
/*
    if (strcmp(name, "secure/login.c") != 0)
	return 0;
*/
    /* fprintf(stderr,"DEBUG: %s,%s\n",ob->name,obfrom->name); */
    if (ob->interactive)
	error("Bad argument1 to exec()\n");
    if (!obfrom->interactive)
	error("Bad argument2 to exec()\n");
    if (obfrom->interactive->message_length) {
        struct object *save;
        save=command_giver;
        command_giver=obfrom;
        add_message(MESSAGE_FLUSH);
	command_giver=save;
    }
    ob->interactive = obfrom->interactive;
    obfrom->interactive = 0;
    ob->interactive->ob = ob;
    ob->flags |= O_ONCE_INTERACTIVE;
    obfrom->flags &= ~O_ONCE_INTERACTIVE;
    add_ref(ob, "exec");
    free_object(obfrom, "exec");
    if (obfrom == command_giver) command_giver = ob;
    return 1;
}

#ifdef DEBUG
/*
 * This is used for debugging reference counts.
 */

void update_ref_counts_for_players() {
    int i;

    for (i=0; i<MAX_PLAYERS; i++) {
	if (all_players[i] == 0)
	    continue;
	all_players[i]->ob->extra_ref++;
	if (all_players[i]->input_to)
	    all_players[i]->input_to->ob->extra_ref++;
    }
}
#endif /* DEBUG */

#ifdef MUDWHO

char mudwhoid[200];

sendmudwhoinfo()
{
    struct object *ob;
    int i;
    static int last_called_time;

    if (current_time - last_called_time < MUDWHO_REFRESH_TIME)
	return;

    last_called_time = get_current_time();

    rwhocli_pingalive();

    for (i = 0; i < num_player; i++) {
	ob = get_interactive_object(i);
	if (ob->living_name)
	{
	    sscanf(ob->name,"%*[^#]#%s",mudwhoid);
	    strcat(mudwhoid,"@");
	    strcat(mudwhoid,MUDWHO_NAME);
	    rwhocli_userlogin(mudwhoid,ob->living_name,
		ob->interactive->login_time);
	}
    }
}
sendmudwhologout(ob)
struct object *ob;
{
	if (ob->interactive)
	{
	    sscanf(ob->name,"%*[^#]#%s",mudwhoid);
	    strcat(mudwhoid,"@");
	    strcat(mudwhoid,MUDWHO_NAME);
	    rwhocli_userlogout(mudwhoid);
	}
}
#endif
