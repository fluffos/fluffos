/*
 * comm_wasm.cc -- the JS-bridged transport for the WASM build.
 *
 * Implements net/transport.h with the webpage as the peer: server->client
 * wire bytes are pushed synchronously to Module.fluffos.onOutput(id,
 * bytes); client->server bytes come in through wasm_console_receive() and
 * take the exact same path a socket read would (comm_telnet_received()).
 * Also provides the connection lifecycle the native build gets from
 * net/transport_libevent.cc: init_user_conn()/shutdown_external_ports()
 * become no-ops (there are no listening sockets -- the JS host creates
 * connections explicitly via wasm_console_connect()).
 *
 * The driver still speaks real telnet (negotiation, IAC escaping, even
 * MCCP) through libtelnet; the page runs the telnet client side.
 */

#include "base/std.h"

#include "wasm/comm_wasm.h"

#include <emscripten.h>

#include <map>
#include <netinet/in.h>

#include "backend.h"
#include "comm.h"
#include "interactive.h"
#include "net/telnet.h"
#include "net/transport.h"
#include "user.h"
#include "vm/vm.h"

// clang-format off
EM_JS(void, js_fluffos_on_output, (int id, const char* data, int len), {
  if (Module["fluffos"] && Module["fluffos"]["onOutput"]) {
    Module["fluffos"]["onOutput"](id, HEAPU8.slice(data, data + len));
  }
});

EM_JS(void, js_fluffos_on_disconnect, (int id), {
  if (Module["fluffos"] && Module["fluffos"]["onDisconnect"]) {
    Module["fluffos"]["onDisconnect"](id);
  }
});
// clang-format on

namespace {

// Live virtual connections, by id. Entries are erased when the transport
// closes, which remove_interactive() always reaches.
std::map<int, interactive_t*> g_connections;
int g_next_id = 1;

interactive_t* find_connection(int id) {
  auto it = g_connections.find(id);
  return it == g_connections.end() ? nullptr : it->second;
}

class WasmConsoleTransport final : public Transport {
 public:
  explicit WasmConsoleTransport(interactive_t* ip) : ip_(ip) {}

  void write(const char* data, size_t len) override {
    if (len > 0) {
      js_fluffos_on_output(ip_->fd, data, static_cast<int>(len));
    }
  }

  int flush() override {
    // write() delivers to the JS host synchronously; nothing is buffered.
    return 1;
  }

  void schedule_command() override {
    // The WASM equivalent of the native per-user command timer; it runs
    // inside the next fluffos_tick(). The 1ms delay (instead of 0) keeps
    // a re-scheduled event out of the CURRENT tick's until-quiet drain,
    // bounding execution to one command per user per tick -- a pasted
    // wall of commands can't monopolize the browser thread.
    if (pending_command_event_ != nullptr) {
      return;
    }
    pending_command_event_ =
        add_walltime_event(std::chrono::milliseconds(1), [this] {
          // One command per firing; process_user_command() re-schedules
          // through maybe_schedule_user_command() if more are buffered.
          pending_command_event_ = nullptr;
          comm_run_scheduled_command(ip_);
        });
  }

  void close() override {
    if (pending_command_event_ != nullptr) {
      // The event outlives us in the scheduler's queue; neuter it.
      pending_command_event_->valid = false;
      pending_command_event_ = nullptr;
    }
    g_connections.erase(ip_->fd);
    js_fluffos_on_disconnect(ip_->fd);
  }

 private:
  interactive_t* ip_;
  TickEvent* pending_command_event_ = nullptr;
};

}  // namespace

int wasm_console_connect() {
  if (!master_ob) {
    debug_message("wasm_console_connect: driver is not booted yet.\n");
    return -1;
  }

  auto* user = user_add();
  int const id = g_next_id++;

  user->connection_type = PORT_TYPE_TELNET;
  user->ob = master_ob;
  user->last_time = get_current_time();
  user->trans = nullptr;
  user->fd = id;
  // Present the connection as if it arrived on the first configured
  // external port, so master::connect(port) sees a familiar value.
  user->local_port = external_port[0].port;
  user->external_port = 0;

  // A loopback address: there is no real peer, the page is the client.
  auto* addr = reinterpret_cast<sockaddr_in*>(&user->addr);
  addr->sin_family = AF_INET;
  addr->sin_port = htons(40000 + (id % 10000));
  addr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  user->addrlen = sizeof(sockaddr_in);

  user->transport = new WasmConsoleTransport(user);
  g_connections[id] = user;

  // Real telnet negotiation, straight to the page.
  user->telnet = net_telnet_init(user);
  send_initial_telnet_negotiations(user);

  on_user_logon(user);

  // logon() may have rejected the connection.
  if (find_connection(id) == nullptr) {
    return -1;
  }
  return id;
}

void wasm_console_receive(int id, const char* data, size_t len) {
  auto* ip = find_connection(id);
  if (ip == nullptr || len == 0) {
    return;
  }

  // Same path as a native socket read: telnet negotiation + input
  // decoding into ip->text, snoop, command detection and scheduling.
  comm_telnet_received(ip, data, len);
}

void wasm_console_disconnect(int id) {
  auto* ip = find_connection(id);
  if (ip == nullptr) {
    return;
  }
  ip->iflags |= NET_DEAD;
  remove_interactive(ip->ob, 0);
}

/*
 * No listening sockets on WASM: connections are created by the JS host
 * through wasm_console_connect().
 */
bool init_user_conn() { return true; }

void shutdown_external_ports() { debug_message("closed external ports\n"); }
