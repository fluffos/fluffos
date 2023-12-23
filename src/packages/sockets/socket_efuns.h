/*
 *  socket_efuns.h -- definitions and prototypes for socket_efuns.c
 *                    5-92 : Dwayne Fontenot : original coding.
 *                   10-92 : Dave Richards : less original coding.
 */

#ifndef _SOCKET_EFUNS_H_
#define _SOCKET_EFUNS_H_

#include <event2/util.h>

// The number here is same as the one in include/socket.h
enum socket_mode {
  MUD = 0,
  STREAM = 1,
  DATAGRAM = 2,
  STREAM_BINARY = 3,
  DATAGRAM_BINARY = 4,
  STREAM_TLS = 5,
  STREAM_TLS_BINARY = 6
};

enum socket_option {
  SO_INVALID = 0,
  SO_TLS_VERIFY_PEER = 1,
  SO_TLS_SNI_HOSTNAME = 2,
};

constexpr int NUM_SOCKET_OPTIONS = 3;

enum socket_state {
  STATE_CLOSED,
  STATE_FLUSHING,
  STATE_UNBOUND,
  STATE_BOUND,
  STATE_LISTEN,
  STATE_HANDSHAKE,
  STATE_DATA_XFER
};

#define BUF_SIZE 65535 /* max reliable packet size        */
#ifdef IPV6
#define ADDR_BUF_SIZE INET6_ADDRSTRLEN
#else
#define ADDR_BUF_SIZE 64 /* max length of address string    */
#endif
struct lpc_socket_t {
  evutil_socket_t fd;
  unsigned short flags;
  enum socket_mode mode;
  enum socket_state state;
  struct sockaddr_storage l_addr;
  struct sockaddr_storage r_addr;
  ev_socklen_t l_addrlen;
  ev_socklen_t r_addrlen;
  object_t *owner_ob;
  object_t *release_ob;
  union string_or_func read_callback;
  union string_or_func write_callback;
  union string_or_func close_callback;
  char *r_buf;
  int r_off;
  int r_len;
  char *w_buf;
  int w_off;
  int w_len;
  struct event *ev_read;
  struct event *ev_write;
  struct lpc_socket_event_data *ev_data;
  SSL_CTX* ssl_ctx;
  SSL* ssl;
  svalue_t options[NUM_SOCKET_OPTIONS];
};

enum socket_flags {
  S_RELEASE = 0x001,
  S_BLOCKED = 0x002,
  S_HEADER = 0x004,
  S_WACCEPT = 0x008,
  S_BINARY = 0x010,
  S_READ_FP = 0x020,
  S_WRITE_FP = 0x040,
  S_CLOSE_FP = 0x080,
  S_EXTERNAL = 0x100,
  S_LINKDEAD = 0x200,
  S_TLS_SUPPORT = 0x400,
};

array_t *socket_status(int);
array_t *socket_status_by_fd(int);
int check_valid_socket(const char *const, int, object_t *, const char *const, int);
void socket_read_select_handler(int);
void socket_write_select_handler(int);
void assign_socket_owner(svalue_t *, object_t *);
object_t *get_socket_owner(int);
void dump_socket_status(outbuffer_t *);
void close_referencing_sockets(object_t *);
int get_socket_address(int, char *, int *, int);
int socket_bind(int, int, const char *);
int socket_create(enum socket_mode, svalue_t *, svalue_t *);
int socket_listen(int, svalue_t *);
int socket_accept(int, svalue_t *, svalue_t *);
int socket_connect(int, const char *, svalue_t *, svalue_t *);
int socket_write(int, svalue_t *, const char *);
int socket_close(int, int);
int socket_release(int, object_t *, svalue_t *);
int socket_acquire(int, svalue_t *, svalue_t *, svalue_t *);
const char *socket_error(int);
int find_new_socket(void);
void set_read_callback(int, svalue_t *);
void set_write_callback(int, svalue_t *);
void set_close_callback(int, svalue_t *);

int lpc_socks_num();
lpc_socket_t *lpc_socks_get(int i);
void mark_sockets();

void lpc_socks_closeall();

void new_lpc_socket_event_listener(int idx, lpc_socket_t *sock, evutil_socket_t real_fd);

#endif /* _SOCKET_EFUNS_H_ */
