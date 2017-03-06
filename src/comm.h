/*
 * comm.h -- definitions and prototypes for comm.c
 *
 */

#ifndef COMM_H
#define COMM_H

#include <sys/socket.h>  // for sockaddr, socklen_t

/*
 * This macro is for testing whether ip is still valid, since many
 * functions call LPC code, which could otherwise use
 * enable_commands(), set_this_player(), or destruct() to cause
 * all hell to break loose by changing or dangling command_giver
 * or command_giver->interactive.  It also saves us a few dereferences
 * since we know we can trust ip, and also increases code readability.
 *
 * Basically, this should be used as follows:
 *
 * (1) when using command_giver:
 *     set a variable named ip to command_giver->interactive at a point
 *     when you know it is valid.  Then, after a call that might have
 *     called LPC code, check IP_VALID(command_giver), or use
 *     VALIDATE_IP.
 * (2) some other object:
 *     set a variable named ip to ob->interactive, and save ob somewhere;
 *     or if you are just dealing with an ip as input, save ip->ob somewhere.
 *     After calling LPC code, check IP_VALID(ob), or use VALIDATE_IP.
 *
 * Yes, I know VALIDATE_IP uses a goto.  It's due to C's lack of proper
 * exception handling.  Only use it in subroutines that are set up
 * for it (i.e. define a failure label, and are set up to deal with
 * branching to it from arbitrary points).
 */
#define IP_VALID(ip, ob) (ob && ip && ob->interactive == ip)
#define VALIDATE_IP(ip, ob) \
  if (!IP_VALID(ip, ob)) goto failure

/*
 * comm.c
 */

void add_vmessage(struct object_t *, const char *, ...);
void add_message(struct object_t *, const char *, int);
void shutdown_external_ports(void);
void set_prompt(const char *);
int process_user_command(struct interactive_t *, char *);
int replace_interactive(struct object_t *, struct object_t *);
int set_call(struct object_t *, struct sentence_t *, int);
void remove_interactive(struct object_t *, int);

int flush_message(struct interactive_t *);
void flush_message_all();

int query_idle(struct object_t *);
#ifndef NO_SNOOP
int new_set_snoop(struct object_t *, struct object_t *);
struct object_t *query_snoop(struct object_t *);
struct object_t *query_snooping(struct object_t *);

struct interactive_t* new_user_handler(int , int, char*);
void on_user_command(struct interactive_t *, char*); // in comm.cc
void on_binary_data(struct interactive_t*, const char*, int); // in comm.cc
void on_ascii_data(struct interactive_t*, const char*); // in comm.cc
void on_mud_data(struct interactive_t*, char*); // in comm.cc
#endif

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_iptable(void);
#endif

#endif /* COMM_H */
