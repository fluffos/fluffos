/*
 * main_wasm.cc -- exported entry points of the WASM driver.
 *
 * There is no main(): the page owns the event loop. The expected call
 * sequence from JS (see src/www/wasm/index.html and wasm/README.md):
 *
 *   const Module = await createFluffOS({...});
 *   Module.FS.chdir('/testsuite');                    // mudlib mount
 *   Module.fluffos.onOutput = (id, bytes) => {...};   // wire bytes out
 *   ccall('fluffos_boot', ..., ['etc/config.test']);
 *   setInterval(() => ccall('fluffos_tick', ...), gametick);
 *   const id = ccall('fluffos_connect', ...);         // telnet "connect"
 *   ccall('fluffos_input', ..., [id, bytes]);         // wire bytes in
 */

#include "base/std.h"

#include <emscripten.h>

#include <ctime>

#include "backend.h"
#include "comm.h"
#include "mainlib.h"
#include "vm/vm.h"
#include "vm/internal/simulate.h"
#include "wasm/comm_wasm.h"

extern "C" {

/*
 * Boot the driver: read config, chdir into the mudlib, init the VM, load
 * master/simul_efun, register the maintenance events. Returns 0 on
 * success, nonzero on failure.
 */
EMSCRIPTEN_KEEPALIVE int fluffos_boot(const char* config_file) {
  tzset();

  try {
    auto* base = init_main(config_file);

    debug_message("==== Runtime Config Table ====\n");
    print_rc_table();
    debug_message("==============================\n");

    vm_start();

    // Registers heartbeats/resets/reclaims on the tick queues and returns;
    // the page drives them through fluffos_tick().
    backend(base);

    if (!init_user_conn()) {
      return 2;
    }
  } catch (const char* e) {
    debug_message("fluffos_boot: fatal: %s\n", e ? e : "unknown error");
    return 1;
  } catch (const std::exception& e) {
    debug_message("fluffos_boot: fatal: %s\n", e.what());
    return 1;
  }

  debug_message("Initializations complete.\n\n");
  return 0;
}

/*
 * Equivalent of the native driver's -f command line flag: calls
 * master::flag(str). Used to run the LPC testsuite ("test") in the
 * browser. Returns 0 on success, nonzero if the master shut the mud down.
 */
EMSCRIPTEN_KEEPALIVE int fluffos_flag(const char* flag) {
  debug_message("Calling master::flag(\"%s\")...\n", flag);
  push_constant_string(flag);
  auto ret = safe_apply_master_ob(APPLY_FLAG, 1);
  if (ret == (svalue_t*)-1 || ret == nullptr || MudOS_is_being_shut_down) {
    debug_message("Shutdown by master object.\n");
    return 1;
  }
  return 0;
}

/*
 * Advance driver time. now_ms is the host's monotonic clock in
 * milliseconds (performance.now()). Runs due call_outs/heartbeats and all
 * buffered user commands (scheduled by the transports as zero-delay
 * wall-time events). Returns the suggested delay until the next call.
 */
EMSCRIPTEN_KEEPALIVE double fluffos_tick(double now_ms) { return wasm_backend_advance(now_ms); }

// New virtual telnet connection; returns connection id or -1.
EMSCRIPTEN_KEEPALIVE int fluffos_connect() { return wasm_console_connect(); }

// Client->server wire bytes for a connection.
EMSCRIPTEN_KEEPALIVE void fluffos_input(int id, const char* data, int len) {
  if (len > 0) {
    wasm_console_receive(id, data, static_cast<size_t>(len));
  }
}

// Close a connection (as if the peer dropped).
EMSCRIPTEN_KEEPALIVE void fluffos_disconnect(int id) { wasm_console_disconnect(id); }

// Orderly driver shutdown; does not return (exits the runtime).
EMSCRIPTEN_KEEPALIVE void fluffos_shutdown(int code) { shutdownMudOS(code); }

}  // extern "C"
