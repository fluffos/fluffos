// Debugger session core: lifecycle, the eval-loop instruction hook, the
// stop-the-world loop, and DAP request dispatch.  See DESIGN.md.

#include "base/std.h"

#include <event2/event.h>

#include <cstring>
#include <ctime>

#include "vm/internal/base/machine.h"
#include "vm/internal/eval_limit.h"
#include "vm/internal/simulate.h"
#include "backend.h"

#include "debugger/debug_hook.h"
#include "debugger/debugger.h"

uint32_t g_lpc_debug_flags = 0;

namespace dbg {

Session g_session;

namespace {

bool is_loopback(const char* addr) {
  return !strcmp(addr, "127.0.0.1") || !strcmp(addr, "::1") || !strcmp(addr, "localhost");
}

void send_response(const djson& request, bool success, djson body, const char* message) {
  auto& s = g_session;
  djson resp = {{"seq", s.out_seq++},
                {"type", "response"},
                {"request_seq", request.value("seq", 0)},
                {"command", request.value("command", "")},
                {"success", success}};
  if (!body.is_null()) {
    resp["body"] = body;
  }
  if (message) {
    resp["message"] = message;
  }
  transport_send(resp);
}

djson capabilities() {
  return djson{
      {"supportsConfigurationDoneRequest", true},
      {"supportsExceptionInfoRequest", true},
      {"supportsLoadedSourcesRequest", true},
      {"supportsDelayedStackTraceLoading", true},
      {"supportsVariablePaging", true},
      {"supportsSetVariable", true},
      {"supportsHitConditionalBreakpoints", true},
      {"exceptionBreakpointFilters",
       djson::array({djson{{"filter", "uncaught"},
                           {"label", "Uncaught LPC Errors"},
                           {"description", "Break when error() would reach the (mudlib) error "
                                           "handler with no catch() protecting it."},
                           {"default", false}},
                     djson{{"filter", "all"},
                           {"label", "All LPC Errors"},
                           {"description", "Break on every LPC error, including ones a catch() "
                                           "would absorb."},
                           {"default", false}}})},
  };
}

// Arm a one-shot stop at the next executed instruction.
void arm_pause(const char* reason, const char* desc) {
  auto& s = g_session;
  s.pause_pending = true;
  s.pause_reason = reason;
  s.pause_desc = desc;
  update_flags();
}

void arm_step(StepMode mode) {
  auto& s = g_session;
  s.step_mode = mode;
  s.step_depth = csp - &control_stack[0];
  s.step_file = nullptr;
  s.step_line = 0;
  if (current_prog) {
    const char* file;
    int line;
    get_explicit_line_number_info(pc + 1, current_prog, &file, &line);
    s.step_file = file;
    s.step_line = line;
  }
  update_flags();
}

void handle_request(const djson& msg) {
  auto& s = g_session;
  std::string cmd = msg.value("command", "");
  djson args = msg.contains("arguments") ? msg["arguments"] : djson::object();

  if (cmd == "initialize") {
    send_response(msg, true, capabilities(), nullptr);
    return;
  }

  if (cmd == "attach") {
    const char* pw = CONFIG_STR(__RC_DEBUGGER_PASSWORD__);
    if (pw && *pw) {
      std::string token = args.value("token", "");
      if (token != pw) {
        debug_message("Debugger: attach rejected (bad token).\n");
        send_response(msg, false, nullptr, "authentication failed");
        transport_kill_client(true);
        return;
      }
    }
    s.authed = true;
    s.attached = true;
    s.poll_countdown = kPollEvery;
    update_flags();
    debug_message("Debugger: client attached.\n");
    send_response(msg, true, djson::object(), nullptr);
    send_event("initialized", djson::object());
    return;
  }

  if (cmd == "disconnect") {
    send_response(msg, true, djson::object(), nullptr);
    debug_message("Debugger: client disconnected.\n");
    detach_client(true);
    return;
  }

  if (!s.attached) {
    send_response(msg, false, nullptr, "not attached");
    return;
  }

  // ---- attached-state requests ----
  if (cmd == "configurationDone") {
    send_response(msg, true, djson::object(), nullptr);
  } else if (cmd == "setBreakpoints") {
    send_response(msg, true, set_breakpoints_request(args), nullptr);
  } else if (cmd == "setExceptionBreakpoints") {
    s.break_uncaught = false;
    s.break_all = false;
    if (args.contains("filters")) {
      for (const auto& f : args["filters"]) {
        std::string name = f.get<std::string>();
        if (name == "uncaught") {
          s.break_uncaught = true;
        } else if (name == "all") {
          s.break_all = true;
        }
      }
    }
    send_response(msg, true, djson::object(), nullptr);
  } else if (cmd == "threads") {
    send_response(msg, true, djson{{"threads", djson::array({djson{{"id", 1}, {"name", "LPC"}}})}},
                  nullptr);
  } else if (cmd == "pause") {
    arm_pause("pause", nullptr);
    send_response(msg, true, djson::object(), nullptr);
  } else if (cmd == "continue") {
    if (!s.stopped) {
      send_response(msg, false, nullptr, "not stopped");
    } else {
      send_response(msg, true, djson{{"allThreadsContinued", true}}, nullptr);
      s.resume = true;
    }
  } else if (cmd == "next" || cmd == "stepIn" || cmd == "stepOut") {
    if (!s.stopped) {
      send_response(msg, false, nullptr, "not stopped");
    } else {
      arm_step(cmd == "next" ? StepMode::kOver
                             : (cmd == "stepIn" ? StepMode::kIn : StepMode::kOut));
      send_response(msg, true, djson::object(), nullptr);
      s.resume = true;
    }
  } else if (cmd == "stackTrace") {
    if (!s.stopped) {
      send_response(msg, false, nullptr, "not stopped");
    } else {
      send_response(msg, true,
                    build_stack_trace(args.value("startFrame", 0), args.value("levels", 0)),
                    nullptr);
    }
  } else if (cmd == "scopes") {
    if (!s.stopped) {
      send_response(msg, false, nullptr, "not stopped");
    } else {
      send_response(msg, true, build_scopes(args.value("frameId", 0)), nullptr);
    }
  } else if (cmd == "variables") {
    if (!s.stopped) {
      send_response(msg, false, nullptr, "not stopped");
    } else {
      send_response(msg, true,
                    build_variables(args.value("variablesReference", 0), args.value("start", 0),
                                    args.value("count", 0)),
                    nullptr);
    }
  } else if (cmd == "setVariable") {
    if (!s.stopped) {
      send_response(msg, false, nullptr, "not stopped");
    } else {
      std::string err;
      djson body = set_variable_request(args, err);
      if (body.is_null()) {
        send_response(msg, false, nullptr, err.c_str());
      } else {
        send_response(msg, true, body, nullptr);
      }
    }
  } else if (cmd == "exceptionInfo") {
    send_response(msg, true,
                  djson{{"exceptionId", "LPC Error"},
                        {"description", s.last_error},
                        {"breakMode", s.last_error_caught ? "always" : "unhandled"}},
                  nullptr);
  } else if (cmd == "source") {
    std::string path = args.value("path", "");
    if (path.empty() && args.contains("source")) {
      path = args["source"].value("path", "");
    }
    std::string content, err;
    if (read_mudlib_file(path, content, err)) {
      send_response(msg, true, djson{{"content", content}}, nullptr);
    } else {
      send_response(msg, false, nullptr, err.c_str());
    }
  } else if (cmd == "loadedSources") {
    send_response(msg, true, build_loaded_sources(), nullptr);
  } else if (cmd == "fluffos_objects") {
    send_response(msg, true,
                  build_objects_list(args.value("filter", ""), args.value("start", 0),
                                     args.value("count", 0)),
                  nullptr);
  } else if (cmd == "fluffos_object") {
    send_response(msg, true, build_object_info(args.value("name", "")), nullptr);
  } else if (cmd == "fluffos_files") {
    send_response(msg, true, build_file_list(args.value("path", "/")), nullptr);
  } else {
    send_response(msg, false, nullptr, "unsupported request");
  }
}

}  // namespace

void send_event(const char* event, djson body) {
  auto& s = g_session;
  djson e = {{"seq", s.out_seq++}, {"type", "event"}, {"event", event}, {"body", std::move(body)}};
  transport_send(e);
}

void update_flags() {
  auto& s = g_session;
  uint32_t f = 0;
  if (s.attached) {
    f |= LPC_DEBUG_ATTACHED;
    if (!s.bp_addrs.empty()) {
      f |= LPC_DEBUG_BREAKPOINTS;
    }
    if (s.step_mode != StepMode::kNone) {
      f |= LPC_DEBUG_STEPPING;
    }
    if (s.pause_pending) {
      f |= LPC_DEBUG_PAUSE_ARMED;
    }
  }
  g_lpc_debug_flags = f;
}

void dispatch_message(const std::string& text) {
  djson msg = djson::parse(text, nullptr, false);
  if (msg.is_discarded() || !msg.is_object()) {
    debug_message("Debugger: ignoring unparseable message.\n");
    return;
  }
  if (msg.value("type", "") == "request") {
    handle_request(msg);
  }
}

void on_client_established(struct lws* wsi) {
  auto& s = g_session;
  s.client = wsi;
  s.authed = false;
  s.connect_time = time(nullptr);
  s.inbuf.clear();
  s.outq.clear();
  s.pending_close = false;
  debug_message("Debugger: client connected, waiting for attach.\n");
}

void on_client_closed(struct lws* wsi) {
  auto& s = g_session;
  if (wsi != s.client) {
    return;
  }
  s.client = nullptr;
  s.pending_close = false;
  debug_message("Debugger: client connection closed.\n");
  detach_client(false);
}

void detach_client(bool close_socket) {
  auto& s = g_session;
  breakpoints_clear_all();
  s.step_mode = StepMode::kNone;
  s.pause_pending = false;
  s.pause_reason = nullptr;
  s.pause_desc = nullptr;
  s.break_uncaught = false;
  s.break_all = false;
  s.attached = false;
  s.authed = false;
  s.handles.clear();
  if (s.stopped) {
    // Never leave the mud frozen because the client went away.
    s.resume = true;
  }
  update_flags();
  if (close_socket && s.client) {
    transport_kill_client(true);
  }
}

void debugger_tick() {
  auto& s = g_session;
  if (s.client && !s.authed && time(nullptr) - s.connect_time > kAuthTimeoutSecs) {
    debug_message("Debugger: closing client that failed to attach within %ds.\n",
                  kAuthTimeoutSecs);
    transport_kill_client(false);
  }
}

// Stop the world.  Runs inside the instruction hook (or the error hook), so
// the whole driver is parked in eval_instruction(): servicing ONLY the
// debugger's private event base here means no other driver event can fire.
void enter_stopped(const char* reason, int hit_bp_id, const char* desc, const char* text) {
  auto& s = g_session;
  if (s.stopped || !s.attached || !s.client) {
    return;
  }
  // One-shot arms are consumed by stopping.
  s.step_mode = StepMode::kNone;
  s.pause_pending = false;
  update_flags();

  // The eval limit is a wall-clock POSIX timer (SIGVTALRM -> outoftime); a
  // multi-minute pause would fire it and abort the evaluation on resume.
  // Disarm now (set_eval(0) zeroes it_value = disarmed, and clears
  // outoftime), re-arm a fresh budget on resume.
  s.saved_eval = get_eval();
  set_eval(0);

  djson body = {{"reason", reason}, {"threadId", 1}, {"allThreadsStopped", true}};
  if (hit_bp_id) {
    body["hitBreakpointIds"] = djson::array({hit_bp_id});
  }
  if (desc) {
    body["description"] = desc;
  }
  if (text) {
    body["text"] = text;
  }
  send_event("stopped", std::move(body));

  s.stopped = true;
  s.resume = false;
  while (!s.resume && s.client) {
    transport_wait();
  }
  s.stopped = false;
  s.resume = false;
  s.handles.clear();

  // Fresh full budget: debugging already distorts wall-clock timing, and
  // resuming with a nearly-expired budget would abort the stepped code.
  set_eval(max_eval_cost > 0 ? max_eval_cost : 1000000);
  update_flags();
}

}  // namespace dbg

// ---------------------------------------------------------------------------
// Hook API (called from core driver code; see debug_hook.h)
// ---------------------------------------------------------------------------

void lpc_debugger_init() {
  int port = CONFIG_INT(__RC_DEBUGGER_PORT__);
  if (port <= 0) {
    return;
  }
  const char* addr = CONFIG_STR(__RC_DEBUGGER_ADDRESS__);
  if (!addr || !*addr) {
    addr = "127.0.0.1";
  }
  const char* pw = CONFIG_STR(__RC_DEBUGGER_PASSWORD__);
  if (!dbg::is_loopback(addr) && (!pw || !*pw)) {
    debug_message(
        "Debugger: REFUSING to listen on non-loopback address %s without a "
        "'debugger password'; debugger disabled.\n",
        addr);
    return;
  }
  if (!dbg::transport_init(port, addr)) {
    debug_message("Debugger: failed to listen on %s:%d; debugger disabled.\n", addr, port);
    return;
  }
  auto& s = dbg::g_session;
  s.enabled = true;
  // Pump the private base from the main loop while the VM is idle.
  s.pump_ev = event_new(
      g_event_base, -1, EV_PERSIST,
      [](evutil_socket_t, short, void*) {
        dbg::transport_pump();
        dbg::debugger_tick();
      },
      nullptr);
  struct timeval tv = {0, 50 * 1000};
  event_add(s.pump_ev, &tv);
  debug_message("Debugger: listening on %s:%d (DAP over websocket, subprotocol \"dap\").\n", addr,
                port);
}

void lpc_debugger_shutdown() {
  auto& s = dbg::g_session;
  if (!s.enabled) {
    return;
  }
  dbg::detach_client(true);
  if (s.pump_ev) {
    event_free(s.pump_ev);
    s.pump_ev = nullptr;
  }
  dbg::transport_shutdown();
  s.enabled = false;
}

void lpc_debugger_instruction_hook() {
  auto& s = dbg::g_session;
  if (s.in_hook || s.stopped) {
    return;
  }
  s.in_hook = true;

  const char* reason = nullptr;
  const char* desc = nullptr;
  int hit_bp = 0;
  uint32_t flags = g_lpc_debug_flags;

  if (flags & LPC_DEBUG_PAUSE_ARMED) {
    reason = s.pause_reason ? s.pause_reason : "pause";
    desc = s.pause_desc;
  }
  if (!reason && (flags & LPC_DEBUG_BREAKPOINTS)) {
    auto it = s.bp_addrs.find(pc);
    if (it != s.bp_addrs.end() && dbg::breakpoint_hit_should_stop(it->second)) {
      reason = "breakpoint";
      hit_bp = it->second;
    }
  }
  if (!reason && (flags & LPC_DEBUG_STEPPING) && s.step_mode != dbg::StepMode::kNone) {
    intptr_t d = csp - &control_stack[0];
    bool stop = false;
    if (d < s.step_depth) {
      // Returned (or unwound) past the stepped frame: stop for every mode.
      stop = true;
    } else if (s.step_mode == dbg::StepMode::kIn && d > s.step_depth) {
      stop = true;  // stepped into a callee
    } else if (d == s.step_depth && s.step_mode != dbg::StepMode::kOut) {
      const char* file;
      int line;
      get_explicit_line_number_info(pc + 1, current_prog, &file, &line);
      if (line != s.step_line || file != s.step_file) {
        stop = true;
      }
    }
    if (stop) {
      reason = "step";
    }
  }

  if (reason) {
    dbg::enter_stopped(reason, hit_bp, desc, nullptr);
  } else if (flags & LPC_DEBUG_ATTACHED) {
    // Service the transport from inside long evaluations so `pause` can
    // interrupt a runaway loop that never returns to the main event loop.
    if (--s.poll_countdown <= 0) {
      s.poll_countdown = dbg::kPollEvery;
      dbg::transport_pump();
      dbg::debugger_tick();
    }
  }
  s.in_hook = false;
}

void lpc_debugger_on_error(const char* err, int caught) {
  auto& s = dbg::g_session;
  if (!s.attached || s.stopped || s.in_error_hook || s.in_hook) {
    return;
  }
  if (!(s.break_all || (s.break_uncaught && !caught))) {
    return;
  }
  s.in_error_hook = true;
  s.last_error = err ? err : "";
  s.last_error_caught = caught != 0;
  dbg::enter_stopped("exception", 0, caught ? "Caught LPC error" : "Uncaught LPC error", err);
  s.in_error_hook = false;
}

void lpc_debugger_on_program_loaded(program_t* prog) {
  dbg::breakpoints_on_program_loaded(prog);
}

void lpc_debugger_on_program_freed(program_t* prog) {
  dbg::breakpoints_on_program_freed(prog);
}

int lpc_debugger_attached() { return dbg::g_session.attached ? 1 : 0; }

int lpc_debugger_wants_local_names() {
  // Deliberately keyed on the raw config value, not g_session.enabled: the
  // documented contract (rc.cc, DESIGN.md §9) is "on whenever 'debugger
  // port' is set", independent of whether the listener actually managed to
  // bind. Keeping this a pure config predicate also makes it exercisable
  // from a unit test without standing up a real socket.
  return CONFIG_INT(__RC_DEBUGGER_PORT__) > 0 ? 1 : 0;
}

void lpc_debugger_break() {
  auto& s = dbg::g_session;
  if (!s.attached) {
    return;  // hard requirement: exact no-op when no debugger is attached
  }
  s.pause_pending = true;
  s.pause_reason = "breakpoint";
  s.pause_desc = "debug_break()";
  dbg::update_flags();
}

// ---------------------------------------------------------------------------
