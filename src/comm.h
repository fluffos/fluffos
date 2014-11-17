/*
 * comm.h -- definitions and prototypes for comm.c
 *
 */

#ifndef COMM_H
#define COMM_H

#include "lpc_incl.h"
#include "network_incl.h"

#include "fliconv.h"
#include "thirdparty/libtelnet/libtelnet.h"
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <vector>

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
extern int inet_packets;
extern int inet_volume;
#ifdef F_NETWORK_STATS
extern int inet_out_packets;
extern int inet_out_volume;
extern int inet_in_packets;
extern int inet_in_volume;
#ifdef PACKAGE_SOCKETS
extern int inet_socket_in_packets;
extern int inet_socket_in_volume;
extern int inet_socket_out_packets;
extern int inet_socket_out_volume;
#endif
#endif
extern int add_message_calls;

#ifdef HAS_CONSOLE
extern int has_console;
extern void restore_sigttin(void);
#endif

void add_vmessage(object_t *, const char *, ...);
void add_message(object_t *, const char *, int);
void add_binary_message_noflush(object_t *, const unsigned char *, int);
void add_binary_message(object_t *, const unsigned char *, int);

void update_ref_counts_for_users(void);
void make_selectmasks(void);
void init_user_conn(void);
void shutdown_external_ports(void);
void set_prompt(const char *);
void process_io(void);
void get_user_data(interactive_t *);
int process_user_command(interactive_t *);
int replace_interactive(object_t *, object_t *);
int set_call(object_t *, sentence_t *, int);
void remove_interactive(object_t *, int);

int flush_message(interactive_t *);
void flush_message_all();

int query_idle(object_t *);
#ifndef NO_SNOOP
int new_set_snoop(object_t *, object_t *);
object_t *query_snoop(object_t *);
object_t *query_snooping(object_t *);
#endif

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_iptable(void);
#endif

// New user API handler.
void new_user_handler(int, struct sockaddr *, size_t, port_def_t *);

const char *sockaddr_to_string(const sockaddr *addr, socklen_t len);

#endif /* COMM_H */
