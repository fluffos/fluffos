/*
 *  comm.c -- communications functions and more.
 *            Dwayne Fontenot (Jacques@TMI)
 */
#include <varargs.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#define TELOPTS
#include <arpa/telnet.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <memory.h>
#include <setjmp.h>
#include "config.h"
#include "lint.h"
#include "interpret.h"
#include "comm.h"
#include "socket_efuns.h"
#include "object.h"
#include "sent.h"
#include "patchlevel.h"
#include "debug.h"

/*
 * external function prototypes.
 */
extern char *xalloc(), *string_copy(), *unshared_str_copy();
extern int parse_command();
extern void call_heart_beat();
extern void debug_message(), fatal(), free_sentence();
#ifdef ED
extern void save_ed_buffer();
#endif
#ifdef ACCESS_RESTRICTED
extern void release_host_access();
void *allow_host_access();
#else
int allow_host_access();
#endif /* ACCESS_RESTRICTED */

int total_users = 0;

/*
 * local function prototypes.
 */
void init_user_conn();
void ipc_remove();
void init_addr_server();
void add_message();
int flush_message();
static int copy_chars();
void sigpipe_handler();
void sigalrm_handler();
int process_user_command();
void hname_handler();
void get_user_data();
char *get_user_command();
char *first_cmd_in_buf PROT((struct interactive *));
int cmd_in_buf PROT((struct interactive *));
void next_cmd_in_buf PROT((struct interactive *));
void remove_interactive();
int call_function_interactive();
int set_call();
void set_prompt PROT((char *));
void print_prompt();
int new_set_snoop();
void telnet_neg PROT((char *, char *));
void query_addr_name PROT((struct object *));
char *query_ip_name();
static void add_ip_entry PROT((long, char *));
char *query_ip_number PROT((struct object *));
char *query_host_name();
struct object *query_snoop();
int query_idle();
void notify_no_command();
void clear_notify();
void set_notify_fail_message();
int replace_interactive();

/*
 * external variables.
 */
extern int port_number;
extern int errno;
extern int d_flag;
extern int current_time;
extern struct object *command_giver, *current_interactive;
extern struct lpc_socket lpc_socks[];
extern int heart_beat_flag;
extern char *default_fail_message;

/*
 * public local variables.
 */
fd_set readmask, writemask;
int num_user;
int num_hidden; /* for the O_HIDDEN flag.  This counter must be kept
   up to date at all times!  If you modify the O_HIDDEN flag in an object,
   make sure that you update this counter if the object is interactive. */
#ifdef COMM_STAT
int add_message_calls=0;
int inet_packets=0;
int inet_volume=0;
#endif /* COMM_STAT */
struct interactive *all_users[MAX_USERS];

/*
 * private local variables.
 */
static int new_user_fd;
static int addr_server_fd = 0;

/*
 * Initialize new user connection socket.
 */
void init_user_conn()
{
  struct sockaddr_in sin;
  int sin_len;
  int optval;

  /*
   * create socket of proper type.
   */
  if((new_user_fd = socket(AF_INET,SOCK_STREAM,0)) == -1){
    perror("init_user_conn: socket");
    exit(1);
  }
  /*
   * enable local address reuse.
   */
  optval = 1;
  if(setsockopt(new_user_fd,SOL_SOCKET,SO_REUSEADDR,(char *)&optval,
		sizeof(optval)) == -1){
    perror("init_user_conn: setsockopt");
    exit(2);
  }
  /*
   * fill in socket address information.
   */
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons((u_short)port_number);
  /*
   * bind name to socket.
   */
  if(bind(new_user_fd,(struct sockaddr *)&sin,sizeof(sin)) == -1){
    perror("init_user_conn: bind");
    exit(3);
  }
  /*
   * get socket name.
   */
  sin_len = sizeof(sin);
  if(getsockname(new_user_fd,(struct sockaddr *)&sin,&sin_len) == -1){
    perror("init_user_conn: getsockname");
    exit(4);
  }
  /*
   * register signal handler for SIGPIPE.
   */
  if(signal(SIGPIPE,sigpipe_handler) == SIGNAL_ERROR){
    perror("init_user_conn: signal SIGPIPE");
    exit(5);
  }
  /*
   * set socket non-blocking,
   */
  if(set_socket_nonblocking(new_user_fd, 1) == -1){
    perror("init_user_conn: set_socket_nonblocking 1");
    exit(8);
  }
  /*
   * listen on socket for connections.
   */
  if(listen(new_user_fd,SOMAXCONN) == -1){
    perror("init_user_conn: listen");
    exit(10);
  }
}

/*
 * Shut down new user accept file descriptor.
 */
void ipc_remove() {
  fprintf(stderr,"Shutting down new user conn...\n");
  /*
   * disallow further sends or receives on socket.
   */
#if !defined(ultrix)
  if(shutdown(new_user_fd,2) == -1){
    perror("ipc_remove: shutdown");
  }
#else /* ultrix */
  shutdown(new_user_fd,2);
#endif /* !defined(ultrix) */
  if(close(new_user_fd) == -1){
    perror("ipc_remove: close");
  }
}

void init_addr_server(hostname,addr_server_port)
     char *hostname;
     int addr_server_port;
{
  struct sockaddr_in server;
  struct hostent *hp;
  int server_fd;
  int optval;

  /*
   * get network host data for hostname.
   */
  hp = gethostbyname(hostname);
  if(hp == NULL){
    perror("init_addr_server: gethostbyname");
    return;
  }
  /*
   * set up address information for server.
   */
  server.sin_family = AF_INET;
  server.sin_port = htons((u_short)addr_server_port);
  server.sin_addr.s_addr = inet_addr(hostname);
  memcpy((char *)&server.sin_addr,(char *)hp->h_addr,hp->h_length);
  /*
   * create socket of proper type.
   */
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(server_fd < 0){  /* problem opening socket */
    perror("init_addr_server: socket");
    return;
  }
  /*
   * enable local address reuse.
   */
  optval = 1;
  if(setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,(char *)&optval,
		sizeof(optval)) == -1){
    perror("init_addr_server: setsockopt");
    return;
  }
  /*
   * connect socket to server address.
   */
  if(connect(server_fd, (struct sockaddr *)&server, sizeof(server)) == -1){
    perror("init_addr_server: connect");
    close(server_fd);
    return;
  }
  addr_server_fd = server_fd;
  fprintf(stderr,"Connected to address server on %s port %d\n",hostname,
	  addr_server_port);
  /*
   * set socket non-blocking.
   */
  if(set_socket_nonblocking(server_fd, 1) == -1){
    perror("init_addr_server: set_socket_nonblocking 1");
    return;
  }
}

/*
 * Send a message to an interactive object. If that object is shadowed,
 * special handling is done.
 */
void
#ifdef VARARGS
add_message(va_alist)
     va_dcl
#else
add_message(format, t1, t2, t3, t4, t5, t6, t7, t8, t9)
    char *format;
    char *t1, t2, t3, t4, t5, t6, t7, t8, t9;
#endif
{
#ifdef VARARGS
	va_list args;
	char *format;
#endif
	struct interactive *ip;
	char *cp, new_string_data[LARGEST_PRINTABLE_STRING];
	struct object *save_command_giver;

#ifdef VARARGS
	va_start(args);
	format = va_arg(args, char *);
#endif
  /*
   * if command_giver->interactive is not valid, write message on stderr.
   */
	if ((command_giver == 0) || (command_giver->flags & O_DESTRUCTED)
		|| (command_giver->interactive == 0)
		|| command_giver->interactive->net_dead
		|| command_giver->interactive->closing)
	{
		putc(']',stderr);
#ifdef VARARGS
		vfprintf(stderr,format,args);
		va_end(args);
#else
		fprintf(stderr,format,t1,t2,t3,t4,t5,t6,t7,t8,t9);
#endif
		return;
	}
	ip = command_giver->interactive;
	new_string_data[0] = '\0';
	/*
	  this is dangerous since the data may not all fit into new_string_data
	  but how to tell if it will without trying it first?  I suppose one
	  could rewrite vsprintf to accept a maximum length (like strncpy) --
	  have fun!
	*/
#ifdef VARARGS
	vsprintf(new_string_data, format, args);
	va_end(args);
#else
	sprintf(new_string_data, format, t1,t2,t3,t4,t5,t6,t7,t8,t9);
#endif
#ifndef NO_SHADOWS
  /*
   * shadow handling.
   */
	if (shadow_catch_message(command_giver,new_string_data)) {
	    /*
	    * snoop handling.
	    */
#ifdef SNOOP_SHADOWED
		if (ip->snoop_by) {
			save_command_giver = command_giver;
			command_giver = ip->snoop_by->ob;
			add_message("$$ %s",new_string_data);
			command_giver = save_command_giver;
		}
#endif
		return;
	}
#endif /* NO_SHADOWS */

  /*
   * write message into command_giver->interactive->message_buf.
   */
	for (cp = new_string_data; *cp != '\0'; cp++) {
		if (ip->message_length == MESSAGE_BUF_SIZE) {
			if (!flush_message()) {
				fprintf (stderr,"Broken connection during add_message.\n");
				return;
			}
			if (ip->message_length == MESSAGE_BUF_SIZE)
				break;
		}
		if (*cp == '\n') {
			if (ip->message_length == (MESSAGE_BUF_SIZE-1)) {
				if (!flush_message()) {
					fprintf (stderr,"Broken connection during add_message.\n");
					return;
				}
				if (ip->message_length == (MESSAGE_BUF_SIZE-1))
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
            fprintf (stderr,"Broken connection during add_message.\n");
            return;
        }
    }
  /*
   * snoop handling.
   */
	if (ip->snoop_by) {
		save_command_giver = command_giver;
		command_giver = ip->snoop_by->ob;
		add_message("%% %s",new_string_data);
		command_giver = save_command_giver;
	}
#ifdef COMM_STAT
	add_message_calls++;
#endif /* COMM_STAT */
}

/*
 * Flush outgoing message buffer of current interactive object.
 */
int flush_message()
{
	struct interactive *ip;
	int length, num_bytes;

  /*
   * if command_giver->interactive is not valid, do nothing.
   */
	if(command_giver == 0 ||
		(command_giver->flags & O_DESTRUCTED) ||
		command_giver->interactive == 0 ||
		command_giver->interactive->closing){
		fprintf(stderr,"flush_message: invalid command_giver!\n");
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
		num_bytes = write(ip->fd,ip->message_buf + ip->message_consumer,length);
		if (num_bytes == -1) {
			if (errno == EWOULDBLOCK) {
				debug(512,("flush_message: write: Operation would block\n"));
				return 1;
			} else {
				fprintf(stderr,"flush_message: write on fd %d\n",ip->fd);
				perror("flush_message: write");
				ip->net_dead = 1;
				return 0;
			}
		}
		ip->message_consumer = (ip->message_consumer + num_bytes) %
			MESSAGE_BUF_SIZE;
		ip->message_length -= num_bytes;
#ifdef COMM_STAT
		inet_packets++;
		inet_volume += num_bytes;
#endif /* COMM_STAT */
	}
	return 1;
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

  for(i=0; i<n; i++){
    if(from[i] == '\r')
      continue;
    if(from[i] == '\n'){
      *to++ = ' ';
      *to++ = '\b';
      *to++ = '\0';
      continue;
    }
    *to++ = from[i];
    /* single character mode */
    /* ack! special case so we don't pick up flow control chars */
    if(ip->single_char && from[0] != '\377')
      *to++ = '\0';
  }
  return(to - start);
}

/*
 * SIGPIPE handler -- does very little for now.
 */
void sigpipe_handler()
{
  fprintf(stderr,"SIGPIPE received.\n");
}

/*
 * SIGALRM handler.
 */
void sigalrm_handler()
{
  heart_beat_flag = 1;
  debug(512,("sigalrm_handler: SIGALRM\n"));
}

INLINE void make_selectmasks()
{
  int i;

  /*
   * generate readmask and writemask for select() call.
   */
  FD_ZERO(&readmask);
  FD_ZERO(&writemask);
  /*
   * set new user accept fd in readmask.
   */
  FD_SET(new_user_fd,&readmask);
  /*
   * set user fds in readmask.
   */
  for(i=0;i<MAX_USERS;i++){
    if(!all_users[i] || all_users[i]->closing || all_users[i]->cmd_in_buf)
      continue;
    /*
     * if this user needs more input to make a complete command,
     * set his fd so we can get it.
     */
    FD_SET(all_users[i]->fd,&readmask);
    if(all_users[i]->message_length != 0)
      FD_SET(all_users[i]->fd,&writemask);
  }
  /*
   * if f_ip_demon is set, set its fd in readmask. f_ip_demon is hname.
   */
  if(addr_server_fd != 0) {
    FD_SET(addr_server_fd,&readmask);
  }
  /*
   * set fd's for efun sockets.
   */
  for(i=0;i<MAX_EFUN_SOCKS;i++){
    if(lpc_socks[i].state != CLOSED){
      if ((lpc_socks[i].flags & S_WACCEPT) == 0)
	FD_SET(lpc_socks[i].fd,&readmask);
      if (lpc_socks[i].flags & S_BLOCKED)
	FD_SET(lpc_socks[i].fd,&writemask);
    }
  }
}

/*
 * Process I/O.
 */
INLINE void process_io()
{
  int i;
  struct object *save_command_giver;

  debug(256,("@"));
  /*
   * check for new user connection.
   */
  if(FD_ISSET(new_user_fd,&readmask)){
    debug(512,("process_io: NEW_USER\n"));
    new_user_handler();
  }
  /*
   * check for data pending on user connections.
   */
  for(i=0;i<MAX_USERS;i++){
    if(!all_users[i] || all_users[i]->closing || all_users[i]->cmd_in_buf)
      continue;
    if (all_users[i]->net_dead) {
       remove_interactive(all_users[i]->ob);
       continue;
    }
    if(FD_ISSET(all_users[i]->fd,&readmask)){
      debug(512,("process_io: USER %d\n",i));
      get_user_data(all_users[i]);
      if(!all_users[i]) continue;
    }
    if(FD_ISSET(all_users[i]->fd,&writemask)){
      save_command_giver = command_giver;
      command_giver = all_users[i]->ob;
      flush_message();
      command_giver = save_command_giver;
    }
  }
  /*
   * check for data pending on efun socket connections.
   */
  for(i=0;i<MAX_EFUN_SOCKS;i++){
    if(lpc_socks[i].state != CLOSED)
      if(FD_ISSET(lpc_socks[i].fd,&readmask))
	socket_read_select_handler(i);
    if(lpc_socks[i].state != CLOSED)
      if(FD_ISSET(lpc_socks[i].fd,&writemask))
	socket_write_select_handler(i);
  }
  /*
   * check for data pending from address server.
   */
  if(addr_server_fd != 0){
    if(FD_ISSET(addr_server_fd,&readmask)){
      debug(512,("process_io: IP_DAEMON\n"));
      hname_handler();
    }
  }
}

/*
 * This is the new user connection handler. This function is called by the
 * event handler when data is pending on the listening socket (new_user_fd).
 * If space is available, an interactive data structure is initialized and
 * the user is connected.
 */
void new_user_handler()
{
  int new_socket_fd;
  struct sockaddr_in addr;
  int length;
  int i;
  char *full_message;
#ifdef ACCESS_RESTRICTED
  void *class;
#endif /* ACCESS_RESTRICTED */
  struct object *ob;
  struct svalue *ret;
  extern struct object *master_ob;

  length = sizeof(addr);
  debug(512,("new_user_handler: accept on fd %d\n",new_user_fd));
  new_socket_fd = accept(new_user_fd,(struct sockaddr *)&addr,(int *)&length);
  if(new_socket_fd < 0){
    if(errno == EWOULDBLOCK){
      debug(512,("new_user_handler: accept: Operation would block\n"));
    }
    else {
      perror("new_user_handler: accept");
    }
    return;
  }
#ifdef ACCESS_RESTRICTED
  if(!(class = allow_host_access(new_socket_fd, new_socket_fd))){
    fprintf(stderr,"new_user_handler: allow_host_access denied.\n");
    return;
  }
#else /* !ACCESS_RESTRICTED */
  if(allow_host_access(new_socket_fd)){
    fprintf(stderr,"new_user_handler: allow_host_access denied.\n");
    return;
  }
#endif /* ACCESS_RESTRICTED */
  for(i=0;i<MAX_USERS;i++){
    if(all_users[i] != 0)
      continue;
    assert_master_ob_loaded();
    command_giver = master_ob;
    master_ob->interactive =
      (struct interactive *)
		DXALLOC(sizeof(struct interactive), 20, "new_user_handler");
    total_users++;
    master_ob->interactive->default_err_message = 0;
    master_ob->flags |= O_ONCE_INTERACTIVE;
    /*
     * initialize new user interactive data structure.
     */
    master_ob->interactive->ob = master_ob;
    master_ob->interactive->input_to = 0;
    master_ob->interactive->closing = 0;
    master_ob->interactive->text[0] = '\0';
    master_ob->interactive->text_end = 0;
    master_ob->interactive->text_start = 0;
    master_ob->interactive->cmd_in_buf = 0;
    master_ob->interactive->carryover = NULL;
    master_ob->interactive->snoop_on = 0;
    master_ob->interactive->snoop_by = 0;
    master_ob->interactive->noecho = 0;
    master_ob->interactive->noesc = 0;
    master_ob->interactive->last_time = current_time;
    master_ob->interactive->trace_level = 0;
    master_ob->interactive->trace_prefix = 0;
    master_ob->interactive->ed_buffer = 0;
    master_ob->interactive->message_producer = 0;
    master_ob->interactive->message_consumer = 0;
    master_ob->interactive->message_length = 0;
    master_ob->interactive->num_carry = 0;
    master_ob->interactive->net_dead = 0;
    master_ob->interactive->single_char = 0;
    all_users[i] = master_ob->interactive;
    all_users[i]->fd = new_socket_fd;
    set_prompt("> ");
    memcpy((char *)&all_users[i]->addr, (char *)&addr, length);
#ifdef ACCESS_RESTRICTED
    all_users[i]->access_class = class;
#endif /* ACCESS_RESTRICTED */
    num_user++;
    debug(512,("New connection from %s.\n",inet_ntoa(addr.sin_addr)));
    /*
     * The user object has one extra reference.
     * It is asserted that the master_ob is loaded.
     */
    add_ref(master_ob, "new_user");
    ret = apply_master_ob("connect", 0);
    if(ret == 0 || ret->type != T_OBJECT){
      remove_interactive(master_ob);
      fprintf(stderr,"Connection from %s aborted.\n",inet_ntoa(addr.sin_addr));
      return;
    }
    /*
     * There was an object returned from connect(). Use this as the
     * user object.
     */
    ob = ret->u.ob;
    if(ob->flags & O_HIDDEN)
      num_hidden++;
    ob->interactive = master_ob->interactive;
    ob->interactive->ob = ob;
    ob->flags |= O_ONCE_INTERACTIVE;
    /*
     * assume the existance of write_prompt and process_input in user.c
     * until proven wrong (after trying to call them).
     */
    ob->interactive->has_write_prompt = 1;
    ob->interactive->has_process_input = 1;

    master_ob->flags &= ~O_ONCE_INTERACTIVE;
    master_ob->interactive = 0;
    free_object(master_ob, "reconnect");
    add_ref(ob, "new_user");
    command_giver = ob;
    if(addr_server_fd != 0){
      query_addr_name(ob);
    }
    logon(ob);
    debug(512,("new_user_handler: end\n"));
    return;
  }
  full_message = "No user slots available: closing connection...\r\n";
  write(new_socket_fd, full_message, strlen(full_message));
  if(close(new_socket_fd) == -1){
    perror("new_user_handler: close");
  }
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
  static char buf[MAX_TEXT], *tbuf;
  struct object *save_current_object = current_object;
  struct object *save_command_giver = command_giver;
  struct svalue *ret;

  if((user_command = get_user_command())){
    if(command_giver->flags & O_DESTRUCTED)
      return(1);
    update_load_av();
    current_object = 0;
    current_interactive = command_giver;
    debug(512,("process_user_command: command_giver = %s\n",
	       command_giver->name));
    tbuf = user_command;
    if(user_command[0] == '!' && (command_giver->interactive->ed_buffer ||
				  command_giver->interactive->input_to)) {
      if(command_giver->interactive->has_process_input){
	push_constant_string(user_command + 1); /* not malloc'ed */
	ret = apply("process_input",command_giver,1);
	if(ret && (ret->type == T_STRING) && ret->u.string){
	  strncpy(buf+1,ret->u.string,MAX_TEXT - 2);
	  tbuf = buf;
	} else if (command_giver->interactive) {
	  command_giver->interactive->has_process_input = 0;
	}
      }
      if (command_giver->flags & O_DESTRUCTED) {
          return 1;
      }
      parse_command(tbuf+1,command_giver);
    } else if(command_giver->interactive->ed_buffer){
#ifdef ED
      ed_cmd(user_command);
#endif /* ED */
    } else if (call_function_interactive(command_giver->interactive,
					 user_command)) {
      ; /* do nothing */
    } else {
      /*
       * send a copy of user input back to user object to
       * provide support for things like command history and
       * mud shell programming languages.
       */
      if(command_giver->interactive->has_process_input){
	push_constant_string(user_command); /* not malloc'ed */
	ret = apply("process_input",command_giver,1);
	if (ret && (ret->type == T_STRING) && ret->u.string) {
	  strncpy(buf,ret->u.string,MAX_TEXT - 1);
	  tbuf = buf;
	} else if (command_giver->interactive) {
	  command_giver->interactive->has_process_input = 0;
	}
      }
      if (command_giver->flags & O_DESTRUCTED) {
          return 1;
      }
      parse_command(tbuf,command_giver);
    }
    /*
     * Print a prompt if user is still here.
     */
    if(command_giver->interactive)
      print_prompt();
    current_object = save_current_object;
    command_giver = save_command_giver;
    return(1);
  }
  current_object = save_current_object;
  command_giver = save_command_giver;
  return(0);
}

#define HNAME_BUF_SIZE 200
/*
 * This is the hname input data handler. This function is called by the
 * master handler when data is pending on the hname socket (f_ip_demon).
 */

void hname_handler()
{
  static char hname_buf[HNAME_BUF_SIZE];
  int num_bytes;
  int tmp;
  char *pp, *q;
  long laddr;

  if(addr_server_fd == 0) return;
  num_bytes = read(addr_server_fd,hname_buf,HNAME_BUF_SIZE);
  switch(num_bytes){
  case -1:
    switch(errno){
    case EWOULDBLOCK:
      debug(512,("hname_handler: read on fd %d: Operation would block.\n",
       addr_server_fd));
      break;
    default:
      fprintf(stderr,"hname_handler: read on fd %d\n",addr_server_fd);
      perror("hname_handler: read");
      tmp = addr_server_fd;
      addr_server_fd = 0;
      close(tmp);
      return;
      break;
    }
    break;
  case 0:
    fprintf(stderr,"hname_handler: closing address server connection.\n");
    tmp = addr_server_fd;
    addr_server_fd = 0;
    close(tmp);
    return;
    break;
  default:
    hname_buf[num_bytes] = '\0';
    debug(512,("hname_handler: address server replies: %s",hname_buf));
    laddr = inet_addr(hname_buf);
    if(laddr != -1){
      pp = strchr(hname_buf,' ');
      if(pp){
	pp++;
	q = strchr(hname_buf,'\n');
	if(q){
	  *q = 0;
	  add_ip_entry(laddr,pp);
	}
      }
    }
    break;
  }
}
/*
 * Read pending data for a user into user->interactive->text.
 */
void get_user_data(ip)
     struct interactive *ip;
{
  struct object *save_command_giver;
  static char buf[MAX_TEXT];
  int text_space;
  int num_bytes;

  /*
   * this /3 is here because of the trick copy_chars() uses to allow
   * empty commands. it needs to be fixed right. later.
   */
  text_space = (MAX_TEXT - ip->text_end - 1) / 3;
  /*
   * read user data.
   */
  debug(512,("get_user_data: read on fd %d\n",ip->fd));
  num_bytes = read(ip->fd,buf,text_space);
  switch(num_bytes){
  case 0:
    if(ip->closing)
      fprintf(stderr,"get_user_data: tried to read from closing fd.\n");
    remove_interactive(ip->ob);
    return;
    break;
  case -1:
    if(errno == EWOULDBLOCK){
      debug(512,("get_user_data: read on fd %d: Operation would block.\n",
		 ip->fd));
    }
    else {
      fprintf(stderr,"get_user_data: read on fd %d\n",ip->fd);
      perror("get_user_data: read");
      remove_interactive(ip->ob);
      return;
    }
    break;
  default:
    buf[num_bytes] = '\0';
    /*
     * replace newlines with nulls and catenate to buffer.
     */
    ip->text_end += copy_chars(buf,ip->text + ip->text_end,num_bytes,ip);
    /*
     * now, text->end is just after the last char read. If last
     * char was a nl, char *before* text_end will be null.
     */
    ip->text[ip->text_end] = '\0';
    /*
     * handle snooping - snooper does not see type-ahead. seems like that
     * would be very inefficient, for little functional gain.
     */
    if(ip->snoop_by && !ip->noecho){
      save_command_giver = command_giver;
      command_giver = ip->snoop_by->ob;
      add_message("%%%s",buf);
      command_giver = save_command_giver;
    }
    /*
     * set flag if new data completes command.
     */
    if(cmd_in_buf(ip)) ip->cmd_in_buf = 1;
    break;
  }
}

/*
 * Return the first cmd of the next user in sequence that has a complete cmd
 * in their buffer.
 * CmdsGiven is used to allow users in ED to send more cmds (if they have
 * them queued up) than users not in ED.
 */
#define StartCmdGiver   (MAX_USERS-1)
#define IncCmdGiver     NextCmdGiver = (NextCmdGiver == 0? StartCmdGiver: \
                                        NextCmdGiver - 1)
static int NextCmdGiver = StartCmdGiver;

char *get_user_command()
{
  int i;
  struct interactive *ip;
  char *user_command = NULL;
  static char buf[MAX_TEXT];

  /*
   * find and return a user command.
   */
  for(i=0;i<MAX_USERS;i++){
    ip = all_users[NextCmdGiver];
    if(ip && ip->cmd_in_buf){
      user_command = first_cmd_in_buf(ip);
      break;
    }
    IncCmdGiver;
  }
  /*
   * no cmds found; return(NULL).
   */
  if(!ip || !user_command) return((char *)NULL);
  /*
   * we have a user cmd -- return it.
   * If user has only one partially completed cmd left after this,
   * move it to the start of his buffer; new stuff will be appended.
   */
  debug(512,("get_user_command: user_command = (%s)\n",user_command));
  command_giver = ip->ob;
  /* 
   * telnet option parsing and negotiation.
   */
  telnet_neg(buf,user_command);
  /*
   * move input buffer pointers to next command.
   */
  next_cmd_in_buf(ip);
  if(!cmd_in_buf(ip)) ip->cmd_in_buf = 0;
  
  IncCmdGiver;

  if(ip->noecho){
    /*
     * Must not enable echo before the user input is received.
     */
    add_message("%c%c%c",IAC,WONT,TELOPT_ECHO);
    ip->noecho = 0;
  }
  ip->last_time = current_time;
  return(buf);
}

/*
 * find the first character of the next complete cmd in a buffer, 0 if no
 * completed cmd.  There is a completed cmd if there is a null between
 * text_start and text_end.  Zero length commands are discarded (as occur
 * between <cr> and <lf>).  Update text_start if we have to skip leading
 * nulls.
 */
char *first_cmd_in_buf(ip)
     struct interactive *ip;
{
  char *p, *q;

  p = ip->text + ip->text_start;

  /*
   * skip null input.
   */
  while((p < (ip->text + ip->text_end)) && !*p)
    p++;

  ip->text_start = p - ip->text;

  if(ip->text_start >= ip->text_end){
    ip->text_start = ip->text_end = 0;
    ip->text[0] = '\0';
    return((char *)NULL);
  }
  /*
   * find end of cmd.
   */
  while((p < (ip->text + ip->text_end)) && *p)
    p++;
  /*
   * null terminated; was command.
   */
  if(p < ip->text + ip->text_end)
    return(ip->text + ip->text_start);
  /*
   * have a partial command at end of buffer; move it to start, return null.
   * if it can't move down, truncate it and return it as cmd.
   */
  p = ip->text + ip->text_start;
  q = ip->text;
  while(p < (ip->text + ip->text_end))
    *(q++) = *(p++);

  ip->text_end -= ip->text_start;
  ip->text_start = 0;
  if(ip->text_end > MAX_TEXT - 2){
    ip->text[ip->text_end-2] = '\0'; /* nulls to truncate */
    ip->text[ip->text_end-1] = '\0'; /* nulls to truncate */
    ip->text_end--;
    return(ip->text);
  }
  /*
   * buffer not full and no newline - no cmd.
   */
  return((char *)NULL);
}

/*
 * return(1) if there is a complete command in ip->text, otherwise return(0).
 */
int cmd_in_buf(ip)
     struct interactive *ip;
{
  char *p;

  p = ip->text + ip->text_start;

  /*
   * skip null input.
   */
  while((p < (ip->text + ip->text_end)) && !*p)
    p++;

  if((p - ip->text) >= ip->text_end){
    return(0);
  }
  /*
   * find end of cmd.
   */
  while((p < (ip->text + ip->text_end)) && *p)
    p++;
  /*
   * null terminated; was command.
   */
  if(p < ip->text + ip->text_end)
    return(1);
  /*
   * no newline - no cmd.
   */
  return(0);
}

/*
 * move pointers to next cmd, or clear buf.
 */
void next_cmd_in_buf(ip)
     struct interactive *ip;
{
  char *p = ip->text + ip->text_start;

  while(*p && p < ip->text + ip->text_end)
    p++;
  /*
   * skip past any nulls at the end.
   */
  while(!*p && p < ip->text + ip->text_end)
    p++;
  if(p < ip->text + ip->text_end)
    ip->text_start = p - ip->text;
  else {
    ip->text_start = ip->text_end = 0;
    ip->text[0] = '\0';
  }
}

/*
 * Remove an interactive user immediately.
 */
void remove_interactive(ob)
    struct object *ob;
{
  struct object *save_command_giver = command_giver;
  int i;

  if (!ob->interactive) {
      return;
  }
  for(i=0;i<MAX_USERS;i++){
    if(all_users[i] != ob->interactive)
      continue;
    if(ob->interactive->closing){
      fprintf(stderr,"Double call to remove_interactive()\n");
      return;
    }
    ob->interactive->closing = 1;

    /*
     * auto-notification of net death
     */
    safe_apply("net_dead", ob, 0);

    if(ob->interactive->snoop_by){
      ob->interactive->snoop_by->snoop_on = 0;
      ob->interactive->snoop_by = 0;
    }
    if(ob->interactive->snoop_on){
      ob->interactive->snoop_on->snoop_by = 0;
      ob->interactive->snoop_on = 0;
    }
    command_giver = ob;
    debug(512,("Closing connection from %s.\n",
	    inet_ntoa(ob->interactive->addr.sin_addr)));
    if(ob->interactive->ed_buffer){
#ifdef ED
      save_ed_buffer();
#endif
    }
#if !defined(ultrix)
    if(shutdown(ob->interactive->fd,0) == -1){
      perror("remove_interactive: shutdown");
    }
#else /* ultrix */
    shutdown(ob->interactive->fd,0);
#endif /* !defined(ultrix) */
    debug(512,("remove_interactive: closing fd %d\n",ob->interactive->fd));
    if(close(ob->interactive->fd) == -1){
      perror("remove_interactive: close");
    }
#ifdef ACCESS_RESTRICTED
    release_host_access(ob->interactive->access_class);
#endif /* ACCESS_RESTRICTED */
    if(ob->flags & O_HIDDEN)
      num_hidden--;
    num_user--;
    if(ob->interactive->input_to){
      free_object(ob->interactive->input_to->ob, "remove_interactive");
      free_sentence(ob->interactive->input_to);
      if(ob->interactive->num_carry > 0)
        free_some_svalues(ob->interactive->carryover,
			  ob->interactive->num_carry);
      ob->interactive->carryover = NULL;
      ob->interactive->num_carry = 0;
      ob->interactive->input_to = 0;
    }
    FREE((char *)ob->interactive);
    total_users--;
    ob->interactive = 0;
    all_users[i] = 0;
    free_object(ob, "remove_interactive");
    command_giver = save_command_giver;
    return;
  }
  fprintf(stderr,"remove_interactive: could not find and remove user %s\n",
	  ob->name);
  abort();
}

#ifndef ACCESS_RESTRICTED

int allow_host_access(new_socket)
     int new_socket;
{
  struct sockaddr_in apa;
  int len = sizeof apa;
  char *ipname, *xalloc();
  static int read_access_list = 0;
  static struct access_list {
    int addr_len;
    char * addr, *name, *comment;
    struct access_list * next;
  } *access_list;
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
	  if((p1 = strchr(buf, ':'))) *p1 = 0;
	  if(buf[0] && buf[0] != '\n')
	    strncpy(ipn, buf, sizeof ipn - 1);
	  if((p2 = p1) && *++p2) {
	    if((p1 = strchr(p2, ':'))) *p1 = 0;
	    if(p2[0] && p2[0] != '\n')
	      strcpy(hname, p2);
	    if(p1 && p1[1] && p1[1] != '\n')
	      strcpy(comment, p1+1);
	  }
	  if(!(na = (struct access_list *)
		DXALLOC(sizeof na[0], 21, "allow_host_access: na"))) {
	    fatal("Out of mem.\n");
	  }
	  na->addr = na->name = na->comment = 0;
	  na->next = 0;
	  if(*ipn && (!(na->addr = DXALLOC(strlen(ipn) + 1, 22,
		"allow_host_access: na->addr")) ||
		      !strcpy(na->addr, ipn)))
	    fatal("Out of mem.\n");
	  if(*hname && (!(na->name =
		DXALLOC(strlen(hname) + 1, 23, "allow_host_access: na->name"))
		|| !strcpy(na->name, hname)))
	    fatal("Out of mem.\n");
	  if(*comment
		&& (!(na->comment=
			DXALLOC(strlen(comment)+1, 24, "allow_host_access: na->comment"))
			|| !strcpy(na->comment, comment)))
	    fatal("Out of mem.\n");

	  if((!(int)*ipn) && ((!*hname) || (!(hent = gethostbyname(hname))) ||
			   (!(na->addr = DXALLOC(hent->h_length+1, 25,
				"allow_host_access: na->addr")))||
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
  if(!access_list)
    return(0);

  if(getpeername(new_socket, (struct sockaddr *)&apa, &len) == -1) {
    if(close(new_socket) == -1){
      perror("allow_host_access: close");
    }
    perror("allow_host_access: getpeername");
    return(-1);
  }
  ipname = inet_ntoa(apa.sin_addr);

  for(ap = access_list; ap; ap = ap->next)
    if(!strncmp(ipname, ap->addr, ap->addr_len)){
      if(ap->comment) (void) write(new_socket, ap->comment,
				   strlen(ap->comment));
      printf("Stopping: %s:%s\n", ap->addr, ap->name?ap->name:"no name");
      if(close(new_socket) == -1){
	perror("allow_host_access: close");
      }
      return(-1);
    }
  return(0);
}
#endif /* not ACCESS_RESTRICTED */

int call_function_interactive(i, str)
     struct interactive *i;
     char *str;
{
  char *function;
  struct object *ob;
  struct svalue *args;
  int num_arg, tindex;

  i->noesc = 0;
  if(!i->input_to)
    return(0);
  /*
   * Special feature: input_to() has been called to setup
   * a call to a function.
   */
  if(i->input_to->ob->flags & O_DESTRUCTED){
    /* Sorry, the object has selfdestructed ! */
    free_object(i->input_to->ob, "call_function_interactive");
    free_sentence(i->input_to);
    i->input_to = 0;
    if(i->num_carry)
      free_some_svalues(i->carryover, i->num_carry);
    i->carryover = NULL;
    i->num_carry = 0;
    return(0);
  }
  /*
   * We must all references to input_to fields before the call to
   * apply(), because someone might want to set up a new input_to().
   */
  free_object(i->input_to->ob, "call_function_interactive");
  function = string_copy(command_giver->interactive->input_to->function);
  ob = i->input_to->ob;
  free_sentence(i->input_to);
  /* If we have args, we have to copy them, so the svalues on the
     interactive struct can be FREEd */
  num_arg = i->num_carry;
  if(num_arg){
    if((args=(struct svalue *)
        DXALLOC(num_arg * sizeof(struct svalue),26,"comm.c: input_to")) == NULL)
      fatal("Not enough memory for input_to.");
    copy_some_svalues(args, i->carryover, i->num_carry);
    free_some_svalues(i->carryover, i->num_carry);
    i->num_carry = 0;
    i->carryover = NULL;
  }
  else
    args = NULL;
  i->input_to = 0;
  if (i->single_char) {
    /*
     * clear single character mode
     */
     i->single_char = 0;
     add_message("%c%c%c", IAC, WONT, TELOPT_SGA);
  }
  /*
   * If we have args, we have to push them onto the stack in the
   * order they were in when we got them.  They will be popped off
   * by the called function.
   */
  push_constant_string(str);
  for(tindex = 0; tindex < num_arg; tindex++)
    push_svalue(&args[tindex]);
  /*
   * Now we set current_object to this object, so that input_to will
   * work for static functions.
   */
  current_object = ob;
  (void)apply(function, ob, num_arg + 1);
  FREE(function);
  free_some_svalues(args, num_arg);
  return(1);
}

int set_call(ob, sent, flags, single_char)
     struct object *ob;
     struct sentence *sent;
     int flags;
     int single_char;
{
  struct object *save_command_giver = command_giver;

  if(ob == 0 || sent == 0)
    return(0);
  if(ob->interactive == 0 || ob->interactive->input_to)
    return(0);
  ob->interactive->input_to = sent;
  ob->interactive->noecho = ((flags & I_NOECHO) != 0);
  ob->interactive->noesc = ((flags & I_NOESC) != 0);
  ob->interactive->single_char = single_char;
  command_giver = ob;
  if (flags & I_NOECHO)
    add_message("%c%c%c", IAC, WILL, TELOPT_ECHO);
  if (single_char)
    add_message("%c%c%c", IAC, WILL, TELOPT_SGA);
  command_giver = save_command_giver;
  return(1);
}

void
set_prompt(str)
char *str;
{
	if (command_giver && command_giver->interactive)  {
		command_giver->interactive->prompt = str;
	}
}

/*
 * Print the prompt, but only if input_to not is disabled.
 */
void print_prompt()
{
  if(command_giver == 0)
    fatal("command_giver == 0.\n");
  if(command_giver->interactive->input_to == 0){
    /* give user object a chance to write its own prompt */
    if(!command_giver->interactive->has_write_prompt)
      add_message(command_giver->interactive->prompt);
    else if(command_giver->interactive&&command_giver->interactive->ed_buffer)
      add_message(command_giver->interactive->prompt);
    else if(!(command_giver->flags & O_DESTRUCTED) &&
	    !apply("write_prompt",command_giver,0)) {
      if (command_giver->interactive) {
		  command_giver->interactive->has_write_prompt = 0;
		  add_message(command_giver->interactive->prompt);
      }
    }
  }
}

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
int new_set_snoop(me, you)
     struct object *me, *you;
{
  struct interactive *on = 0, *by = 0, *tmp;
  int i;

  /*
   * Stop if people managed to quit before we got this far.
   */
  if(me->flags & O_DESTRUCTED)
    return(0);
  if(you && (you->flags & O_DESTRUCTED))
    return(0);
  /*
   * Find the snooper && snoopee.
   */
  for(i = 0 ; i < MAX_USERS && (on == 0 || by == 0); i++){
    if(all_users[i] == 0)
      continue;
    if(all_users[i]->ob == me)
      by = all_users[i];
    else if(all_users[i]->ob == you)
      on = all_users[i];
  }

  /*
   * Stop snoop.
   */
  if(you == 0){
    if(by == 0)
      error("Could not find snooper to stop snoop on.\n");
    if(by->snoop_on == 0)
      return(1);
    by->snoop_on->snoop_by = 0;
    by->snoop_on = 0;
    return(1);
  }
  /*
   * Strange event, but possible, so test for it.
   */
  if(on == 0 || by == 0)
    return(0);
  /*
   * Protect against snooping loops.
   */
  for(tmp = on; tmp; tmp = tmp->snoop_on){
    if(tmp == by)
      return(0);
  }
  /*
   * Terminate previous snoop, if any.
   */
  if(by->snoop_on){
    by->snoop_on->snoop_by = 0;
    by->snoop_on = 0;
  }
  if(on->snoop_by){
    on->snoop_by->snoop_on = 0;
    on->snoop_by = 0;
  }
  on->snoop_by = by;
  by->snoop_on = on;
  return(1);
}

#define TS_DATA         0
#define TS_IAC          1
#define TS_WILL         2
#define TS_WONT         3
#define TS_DO           4
#define TS_DONT         5

void telnet_neg(to, from)
     char *to, *from;
{
  int state = TS_DATA;
  int ch;
  char *first = to;

  while(1){
    ch = (*from++ & 0xff);
    switch(state){
    case TS_DATA:
      switch(ch){
      case IAC:
	state = TS_IAC;
	continue;
      case '\b':  /* Backspace */
      case 0x7f:  /* Delete */
	if(to <= first)
	  continue;
	to -= 1;
	continue;
      default:
	if(ch & 0x80){
	  continue;
	}
	*to++ = ch;
	if(ch == 0)
	  return;
	continue;
      }
    case TS_IAC:
      switch(ch){
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
      case NOP:
      case GA:
      default:
	break;
      }
      state = TS_DATA;
      continue;
    case TS_WILL:
      state = TS_DATA;
      continue;
    case TS_WONT:
      state = TS_DATA;
      continue;
    case TS_DO:
      state = TS_DATA;
      continue;
    case TS_DONT:
      state = TS_DATA;
      continue;
    default:
      state = TS_DATA;
      continue;
    }
  }
}

void query_addr_name(ob)
     struct object *ob;
{
  static char buf[80];
  int msgtype = NAMEBYIP;

  memcpy(buf,(char *)&msgtype,sizeof(msgtype));
  sprintf(&buf[sizeof(int)],"%s",query_ip_number(ob));
  debug(512,("query_addr_name: sent address server %s\n",&buf[sizeof(int)]));
  if(write(addr_server_fd,buf,sizeof(int) + strlen(&buf[sizeof(int)]) + 1)
     == -1){
    switch(errno){
    case EBADF:
      fprintf(stderr,"Address server has closed connection.\n");
      addr_server_fd = 0;
      break;
    default:
      perror("query_addr_name: write");
      break;
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

  if(ob == 0)
    ob = command_giver;
  if(!ob || ob->interactive == 0)
    return((char *)NULL);
  for(i = 0; i < IPSIZE; i++){
    if(iptable[i].addr == ob->interactive->addr.sin_addr.s_addr &&
	iptable[i].name)
      return(iptable[i].name);
  }
  return(inet_ntoa(ob->interactive->addr.sin_addr));
}

static void add_ip_entry(addr, name)
     long addr;
     char *name;
{
  int i;

  for(i = 0; i < IPSIZE; i++){
    if(iptable[i].addr == addr)
      return;
  }
  iptable[ipcur].addr = addr;
  if(iptable[ipcur].name)
    free_string(iptable[ipcur].name);
  iptable[ipcur].name = make_shared_string(name);
  ipcur = (ipcur+1) % IPSIZE;
}

char *query_ip_number(ob)
     struct object *ob;
{
  if(ob == 0)
    ob = command_giver;
  if(!ob || ob->interactive == 0)
    return((char *)NULL);
  return(inet_ntoa(ob->interactive->addr.sin_addr));
}

#ifndef INET_NTOA_OK
/*
 * Note: if the address string is "a.b.c.d" the address number is
 *       a * 256^3 + b * 256^2 + c * 256 + d
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
  return(addr);
}
#endif /* INET_NTOA_OK */

char *query_host_name()
{
  static char name[20];

  gethostname(name,sizeof(name));
  name[sizeof(name) - 1] = '\0'; /* Just to make sure */
  return(name);
}

struct object *query_snoop(ob)
     struct object *ob;
{
  if (!ob->interactive || (ob->interactive->snoop_by == 0))
    return(0);
  return(ob->interactive->snoop_by->ob);
}

int query_idle(ob)
     struct object *ob;
{
  if(!ob->interactive)
    error("query_idle() of non-interactive object.\n");
  return(current_time - ob->interactive->last_time);
}

void notify_no_command()
{
  char *p,*m;

  if(!command_giver || !command_giver->interactive)
    return;
  p = command_giver->interactive->default_err_message;
  if(p){
    m = process_string(p); /* We want 'value by function call' /JnA */
#ifndef NO_SHADOWS
    if(!shadow_catch_message(command_giver, m))
#endif /* NO_SHADOWS */
      add_message(m);
    if(m != p)
      FREE(m);
    free_string(p);
    command_giver->interactive->default_err_message = 0;
  }
  else {
    add_message("%s\n", default_fail_message);
  }
}

void clear_notify()
{
  if(!command_giver->interactive)
    return;
  if(command_giver->interactive->default_err_message){
    free_string(command_giver->interactive->default_err_message);
    command_giver->interactive->default_err_message = 0;
  }
}

void set_notify_fail_message(str)
     char *str;
{
  if(!command_giver || !command_giver->interactive)
    return;
  clear_notify();
  if(command_giver->interactive->default_err_message)
    free_string(command_giver->interactive->default_err_message);
  command_giver->interactive->default_err_message = make_shared_string(str);
}

int replace_interactive(ob, obfrom)
     struct object *ob;
     struct object *obfrom;
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
	 * assume the existance of write_prompt and process_input in user.c
	 * until proven wrong (after trying to call them).
	 */
	ob->interactive->has_write_prompt = 1;
	ob->interactive->has_process_input = 1;
	obfrom->interactive = 0;
	ob->interactive->ob = ob;
	ob->flags |= O_ONCE_INTERACTIVE;
	obfrom->flags &= ~O_ONCE_INTERACTIVE;
	add_ref(ob, "exec");
	free_object(obfrom, "exec");
	if (obfrom == command_giver) {
		command_giver = ob;
	}
	return(1);
}

#ifdef DEBUG
/*
 * This is used for debugging reference counts.
 */
void update_ref_counts_for_users()
{
  int i;

  for(i=0; i<MAX_USERS; i++){
    if(all_users[i] == 0)
      continue;
    all_users[i]->ob->extra_ref++;
    if(all_users[i]->input_to)
      all_users[i]->input_to->ob->extra_ref++;
  }
}
#endif /* DEBUG */
