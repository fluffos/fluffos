#ifdef __PACKAGE_SOCKETS__

#include <globals.h>

#define EESUCCESS 1
#define STREAM_TLS 5

#define SO_TLS_VERIFY_PEER 1
#define SO_TLS_SNI_HOSTNAME 2
#define SO_TLS_CERT 3
#define SO_TLS_KEY 4

nosave int server_fd = -1;
nosave int client_fd = -1;
nosave int accepted_fd = -1;

nosave string server_received = "";
nosave string client_received = "";

nosave int client_handshake_done = 0;
nosave int server_handshake_done = 0;
nosave int client_sent = 0;
nosave int test_completed = 0;

void cleanup() {
  if (accepted_fd != -1) {
    socket_close(accepted_fd);
    accepted_fd = -1;
  }
  if (client_fd != -1) {
    socket_close(client_fd);
    client_fd = -1;
  }
  if (server_fd != -1) {
    socket_close(server_fd);
    server_fd = -1;
  }
}

void timeout_fail() {
  if (!test_completed) {
    cleanup();
    ASSERT2(0, "TLS server socket test timed out");
  }
}

void finish_test() {
  ASSERT(server_handshake_done);
  ASSERT(client_handshake_done);
  ASSERT_EQ("client hello", server_received);
  ASSERT_EQ("server hello", client_received);
  test_completed = 1;
  cleanup();
}

void client_close(int fd) {
  if (!test_completed) {
    ASSERT2(0, sprintf("client socket %d closed unexpectedly", fd));
  }
}

void client_read(int fd, mixed msg) {
  client_received = msg;
  ASSERT_EQ("server hello", msg);
  call_out("finish_test", 0);
}

void server_conn_read(int fd, mixed msg) {
  int ret;

  server_received = msg;
  ASSERT_EQ("client hello", msg);
  ret = socket_write(fd, "server hello");
  ASSERT_EQ(EESUCCESS, ret);
}

void server_conn_write(int fd) {
  server_handshake_done = 1;
}

void server_accept_ready(int fd) {
  int new_fd;

  new_fd = socket_accept(fd, "server_conn_read", "server_conn_write");
  ASSERT2(new_fd >= 0, "socket_accept failed");

  accepted_fd = new_fd;
}

void client_write(int fd) {
  int ret;

  if (!client_handshake_done) {
    client_handshake_done = 1;
  }
  if (client_sent) {
    return;
  }

  ret = socket_write(fd, "client hello");
  ASSERT_EQ(EESUCCESS, ret);
  client_sent = 1;
}

void start_client(int port) {
  int ret;

  client_fd = socket_create(STREAM_TLS, "client_read", "client_close");
  ASSERT2(client_fd >= 0, "failed to create client socket");

  socket_set_option(client_fd, SO_TLS_VERIFY_PEER, 0);
  socket_set_option(client_fd, SO_TLS_SNI_HOSTNAME, "localhost");

  ret = socket_connect(client_fd, sprintf("127.0.0.1 %d", port), "client_read", "client_write");
  ASSERT_EQ(EESUCCESS, ret);
}

void begin_test() {
  string local;
  string *parts;
  int port;

  local = socket_address(server_fd, 1);
  ASSERT(stringp(local));

  parts = explode(local, " ");
  ASSERT_EQ(2, sizeof(parts));

  port = to_int(parts[1]);
  start_client(port);
}

void do_tests() {
  int ret;
  string err;

  server_fd = socket_create(STREAM_TLS, "server_accept_ready");
  ASSERT2(server_fd >= 0, "failed to create server socket");

  socket_set_option(server_fd, SO_TLS_CERT, "etc/cert.pem");
  socket_set_option(server_fd, SO_TLS_KEY, "etc/key.pem");

  ret = socket_bind(server_fd, 0);
  ASSERT_EQ(EESUCCESS, ret);

  ret = socket_listen(server_fd, "server_accept_ready");
  ASSERT_EQ(EESUCCESS, ret);

  call_out("begin_test", 0);
  call_out("timeout_fail", 5);
}

#else
void do_tests() {
  write("PACKAGE_SOCKETS not available, skipping TLS server test.\n");
}
#endif
