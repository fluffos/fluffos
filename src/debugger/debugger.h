// Internal shared declarations for the LPC debugger implementation.
// Only src/debugger/*.cc may include this header; core driver code uses
// debug_hook.h.  See DESIGN.md for the architecture.

#ifndef DEBUGGER_DEBUGGER_H
#define DEBUGGER_DEBUGGER_H

#include <cstdint>
#include <deque>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

struct program_t;
struct object_t;
struct lws;
struct lws_context;
struct event_base;
struct event;

namespace dbg {

using djson = nlohmann::json;

enum class StepMode { kNone, kIn, kOver, kOut };

// One requested source breakpoint (per setBreakpoints entry).
struct SrcBp {
  int id = 0;
  int req_line = 0;     // line the client asked for
  int actual_line = 0;  // line we bound to (snap-to-next-code-line)
  bool verified = false;
};

// A variablesReference handle.  Valid only while stopped; pointers are
// borrowed from the frozen world and the table is cleared on resume, so the
// debugger holds no LPC references across resume (keeps the Debug-build
// ref-count checker oblivious to us by construction).
struct VarHandle {
  enum Kind {
    kScopeArgs,
    kScopeLocals,
    kScopeGlobals,
    kArray,   // ptr = array_t*
    kClass,   // ptr = array_t*
    kMapping, // ptr = mapping_t*
    kBuffer,  // ptr = buffer_t*
    kObject,  // ptr = object_t*
  } kind;
  int frame = 0;
  void* ptr = nullptr;
};

constexpr int kHandleBase = 1000;
constexpr int kPollEvery = 65536;     // instructions between transport polls
constexpr int kAuthTimeoutSecs = 10;  // ws connect -> successful attach

struct Session {
  // ---- transport ----
  struct event_base* base = nullptr;  // private event base (never the main one)
  struct lws_context* ctx = nullptr;
  struct event* pump_ev = nullptr;  // recurring pump on the MAIN base
  struct event* tick_ev = nullptr;  // recurring wakeup on the private base
  struct lws* client = nullptr;     // the single attached client, if any
  bool authed = false;
  int64_t connect_time = 0;
  std::string inbuf;                // ws fragment reassembly
  std::deque<std::string> outq;     // complete outbound DAP messages
  bool pending_close = false;       // close once outq drains (see transport_kill_client)

  // ---- session ----
  bool enabled = false;  // "debugger port" configured and listener up
  bool attached = false;
  int64_t out_seq = 1;

  // ---- run control ----
  bool pause_pending = false;
  const char* pause_reason = nullptr;  // DAP `stopped` reason for the armed pause
  const char* pause_desc = nullptr;    // optional description (e.g. "debug_break()")
  bool stopped = false;                // sitting in the stop loop
  bool resume = false;                 // stop loop exit signal
  bool in_hook = false;
  bool in_error_hook = false;
  int poll_countdown = kPollEvery;

  StepMode step_mode = StepMode::kNone;
  intptr_t step_depth = 0;
  const char* step_file = nullptr;  // shared-string ptr from prog->strings
  int step_line = 0;

  bool break_uncaught = false;
  bool break_all = false;
  std::string last_error;   // for exceptionInfo
  bool last_error_caught = false;

  // ---- breakpoints ----
  int next_bp_id = 1;
  // canonical path ("std/room", extension-blind, no leading '/') -> bps
  std::map<std::string, std::vector<SrcBp>> bps;
  std::unordered_map<char*, int> bp_addrs;  // live code address -> bp id
  std::unordered_map<program_t*, std::vector<char*>> bp_by_prog;

  // ---- stopped-state handles ----
  std::vector<VarHandle> handles;

  uint64_t saved_eval = 0;
};

extern Session g_session;

// ---- transport_lws.cc ----
bool transport_init(int port, const char* address);
void transport_shutdown();
void transport_pump();  // non-blocking service pass over the private base
void transport_wait();  // one blocking pass (stop loop; bounded by tick_ev)
void transport_send(const djson& msg);
void transport_kill_client(bool flush);

// ---- debug_server.cc ----
void dispatch_message(const std::string& text);
void on_client_established(struct lws* wsi);
void on_client_closed(struct lws* wsi);
void debugger_tick();  // housekeeping: auth timeout
void update_flags();
void send_event(const char* event, djson body);
void enter_stopped(const char* reason, int hit_bp_id, const char* desc, const char* text);
void detach_client(bool close_socket);

// ---- breakpoints.cc ----
std::string canonical_lpc_path(const char* path);
djson set_breakpoints_request(const djson& args);
void breakpoints_on_program_loaded(program_t* prog);
void breakpoints_on_program_freed(program_t* prog);
void breakpoints_clear_all();

// ---- inspect.cc ----
int frame_count();
djson build_stack_trace(int start, int levels);
djson build_scopes(int frame_id);
djson build_variables(int ref, int start, int count);
// Returns a null djson (and sets `err`) on failure; caller (debug_server.cc)
// turns that into a DAP error response instead of a success body.
djson set_variable_request(const djson& args, std::string& err);
djson build_loaded_sources();
djson build_objects_list(const std::string& filter, int start, int count);
djson build_object_info(const std::string& name);
djson build_file_list(const std::string& path);
bool read_mudlib_file(const std::string& path, std::string& out, std::string& err);

}  // namespace dbg

#endif  // DEBUGGER_DEBUGGER_H
