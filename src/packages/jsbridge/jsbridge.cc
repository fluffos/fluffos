/*
 * jsbridge.cc -- LPC <-> JavaScript bridge efuns (WASM build only).
 *
 * Lets the mudlib reach the hosting page's JavaScript, and JavaScript
 * answer back asynchronously -- fetch(), canvas/WebGL, browser storage,
 * anything the page chooses to expose:
 *
 *   js_eval(string code)
 *     Synchronous eval in the page's context; returns the result as a
 *     string.
 *
 *   js_export(string name, void | string | function cb)
 *     Register an LPC function under `name` so the PAGE can call into
 *     LPC: Module.fluffos.callLPC(name, ...args) returns a Promise that
 *     resolves with the callback's return value. With no callback
 *     argument, unregisters `name`.
 *
 *   js_call(string handler, string *args, void | string | function cb)
 *     Asynchronous call to a HANDLER the page registered on
 *     Module.fluffos.handlers. The handler receives the args as
 *     individual string arguments and may return a value or a Promise;
 *     when it settles, cb(result, success, id) runs on the next driver
 *     tick (same delivery pattern as the resolve() efun). Returns a
 *     nonnegative call id. Structured data crosses the bridge as JSON
 *     strings -- encode/decode on either side.
 *
 * Page side (see src/www/wasm/index.html for working examples):
 *
 *   Module.fluffos.handlers = {
 *     fetch_text: async (url) => (await fetch(url)).text(),
 *   };
 */

#include "base/package_api.h"

#include <emscripten.h>

#include <cstdlib>
#include <map>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "backend.h"  // for add_walltime_event
#include "packages/core/sprintf.h"  // for string_print_formatted

namespace {

struct JsCall {
  LPC_INT key;
  svalue_t call_back;    // T_STRING (function name) or T_FUNCTION
  object_t* ob_to_call;  // owner of a string callback; ref held
  bool want_callback;
  bool delivering;  // a result is already scheduled for delivery
};

// Every in-flight js_call(): owns refs on the callback (and its object)
// until delivery, mirroring the resolve() efun's pending-query table.
std::map<LPC_INT, JsCall*> pending_js_calls;

// LPC functions exported to the page via js_export(): the JS side calls
// them through Module.fluffos.callLPC -> fluffos_lpc_call(). Each entry
// owns refs on its callback (and its object) until unregistered.
struct JsExport {
  svalue_t call_back;    // T_STRING (function name) or T_FUNCTION
  object_t* ob_to_call;  // owner of a string callback; ref held
};
std::map<std::string, JsExport*> g_js_exports;

void free_js_export(JsExport* exp) {
  free_svalue(&exp->call_back, "free_js_export");
  free_object(&exp->ob_to_call, "free_js_export");
  delete exp;
}

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_callback_refs(svalue_t* call_back, object_t* ob) {
  if (ob) {
    ob->extra_ref++;
  }
  if (call_back->type == T_STRING) {
    EXTRA_REF(BLOCK(call_back->u.string))++;
  } else if (call_back->type == T_FUNCTION) {
    call_back->u.fp->hdr.extra_ref++;
  }
}

void mark_js_calls_impl() {
  for (auto& entry : pending_js_calls) {
    auto* call = entry.second;
    if (!call->want_callback) continue;
    mark_callback_refs(&call->call_back, call->ob_to_call);
  }
  for (auto& entry : g_js_exports) {
    mark_callback_refs(&entry.second->call_back, entry.second->ob_to_call);
  }
}
#endif

void free_js_call(JsCall* call) {
  if (call->want_callback) {
    free_svalue(&call->call_back, "free_js_call");
    free_object(&call->ob_to_call, "free_js_call");
  }
  pending_js_calls.erase(call->key);
  delete call;
}

// Deliver a settled result to the LPC callback: cb(result, success, id).
void deliver_js_result(JsCall* call, bool success, const std::string& result) {
  if (call->want_callback) {
    copy_and_push_string(result.c_str());
    push_number(success ? 1 : 0);
    push_number(call->key);
    set_eval(max_eval_cost);
    if (call->call_back.type == T_STRING) {
      safe_apply(call->call_back.u.string, call->ob_to_call, 3, ORIGIN_INTERNAL);
    } else {
      safe_call_function_pointer(call->call_back.u.fp, 3);
    }
  }
  free_js_call(call);
}

}  // namespace

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_js_calls() { mark_js_calls_impl(); }
#endif

/*
 * Shutdown: drop every in-flight call and the refs it holds. Pending
 * delivery events are discarded unrun by clear_tick_events(), so nothing
 * will touch these entries afterwards.
 */
void jsbridge_cleanup() {
  while (!pending_js_calls.empty()) {
    free_js_call(pending_js_calls.begin()->second);
  }
  while (!g_js_exports.empty()) {
    auto it = g_js_exports.begin();
    auto* exp = it->second;
    g_js_exports.erase(it);
    free_js_export(exp);
  }
}

// clang-format off
EM_JS(char*, js_bridge_eval, (const char* code_ptr), {
  var text;
  try {
    var value = eval(UTF8ToString(code_ptr));
    if (value === undefined || value === null) {
      text = "";
    } else if (typeof value === "object") {
      try { text = JSON.stringify(value); } catch (e) { text = String(value); }
    } else {
      text = String(value);
    }
  } catch (err) {
    // A JS exception must NOT unwind through the live wasm frames (it
    // would abandon the VM mid-instruction); report it as text instead.
    text = "JS error: " + String(err);
  }
  var len = lengthBytesUTF8(text) + 1;
  var ptr = _malloc(len);
  stringToUTF8(text, ptr, len);
  return ptr;
});

EM_JS(void, js_bridge_invoke, (int id, const char* name_ptr, const char* args_ptr), {
  var name = UTF8ToString(name_ptr);
  var args = JSON.parse(UTF8ToString(args_ptr));
  var finish = function(ok, value) {
    var text;
    if (value === undefined || value === null) {
      text = "";
    } else if (typeof value === "object") {
      try { text = JSON.stringify(value); } catch (e) { text = String(value); }
    } else {
      text = String(value);
    }
    var len = lengthBytesUTF8(text) + 1;
    var ptr = _malloc(len);
    stringToUTF8(text, ptr, len);
    _fluffos_js_result(id, ok ? 1 : 0, ptr);
    _free(ptr);
  };
  var handlers = Module["fluffos"] && Module["fluffos"]["handlers"];
  var handler = handlers && handlers[name];
  if (typeof handler !== "function") {
    finish(false, "no such JS handler: " + name);
    return;
  }
  try {
    Promise.resolve(handler.apply(null, args)).then(
        function(value) { finish(true, value); },
        function(err) { finish(false, String(err)); });
  } catch (err) {
    finish(false, String(err));
  }
});
EM_JS(void, js_bridge_lpc_result, (int id, int ok, const char* text_ptr), {
  var fluffos = Module["fluffos"];
  var pending = fluffos && fluffos["_lpcPending"];
  var entry = pending && pending[id];
  if (!entry) return;
  delete pending[id];
  var text = UTF8ToString(text_ptr);
  if (ok) entry["resolve"](text); else entry["reject"](new Error(text));
});
// clang-format on

namespace {

// Run an exported LPC callback for a page-initiated call and settle the
// page's Promise. Runs from the scheduler (driver context).
void deliver_lpc_call(const std::string& name, const std::vector<std::string>& args, int id) {
  auto it = g_js_exports.find(name);
  if (it == g_js_exports.end()) {
    js_bridge_lpc_result(id, 0, ("no such LPC export: " + name).c_str());
    return;
  }
  auto* exp = it->second;

  auto* arr = allocate_empty_array(args.size());
  for (size_t i = 0; i < args.size(); i++) {
    arr->item[i].type = T_STRING;
    arr->item[i].subtype = STRING_MALLOC;
    arr->item[i].u.string = string_copy(args[i].c_str(), "deliver_lpc_call");
  }
  push_refed_array(arr);
  push_number(id);

  set_eval(max_eval_cost);
  svalue_t* ret;
  if (exp->call_back.type == T_STRING) {
    ret = safe_apply(exp->call_back.u.string, exp->ob_to_call, 2, ORIGIN_INTERNAL);
  } else {
    ret = safe_call_function_pointer(exp->call_back.u.fp, 2);
  }

  if (ret == nullptr) {
    js_bridge_lpc_result(id, 0, "LPC callback error (object destructed or runtime error)");
    return;
  }
  if (ret->type == T_STRING) {
    js_bridge_lpc_result(id, 1, ret->u.string);
    return;
  }
  // Other values are %O-formatted (numbers print plainly; return a
  // JSON-encoded string from LPC for structured data).
  char* formatted = string_print_formatted("%O", 1, ret);
  js_bridge_lpc_result(id, 1, formatted ? formatted : "");
  if (formatted) {
    FREE_MSTR(formatted);
  }
}

}  // namespace

/*
 * Called from the page (Module.fluffos.callLPC helper) to invoke an LPC
 * function registered with js_export(). Only schedules -- the LPC call
 * runs on the next driver tick, in proper driver context. Returns a
 * nonnegative call id (the Promise key), or -1 if nothing is registered
 * under that name.
 */
extern "C" EMSCRIPTEN_KEEPALIVE int fluffos_lpc_call(const char* name, const char* json_args) {
  static int next_lpc_call_id = 0;

  if (name == nullptr || g_js_exports.find(name) == g_js_exports.end()) {
    return -1;
  }

  // Non-throwing parse: the input crosses an extern "C" boundary from
  // page JS; a std::exception here would abort the wasm module.
  auto parsed = nlohmann::json::parse(json_args ? json_args : "", nullptr, false);
  if (parsed.is_discarded() || !parsed.is_array()) {
    return -1;
  }
  std::vector<std::string> args;
  args.reserve(parsed.size());
  for (auto& item : parsed) {
    args.push_back(item.is_string() ? item.get<std::string>() : item.dump());
  }

  int const id = next_lpc_call_id++;
  std::string const name_copy(name);
  add_walltime_event(std::chrono::milliseconds(0),
                     [name_copy, args, id] { deliver_lpc_call(name_copy, args, id); });
  return id;
}

/*
 * Called from JS (see js_bridge_invoke above) when a handler settles.
 * May run outside driver context (a microtask between ticks), so the LPC
 * callback is deferred to the scheduler rather than applied here.
 */
extern "C" EMSCRIPTEN_KEEPALIVE void fluffos_js_result(int id, int success, const char* result) {
  auto it = pending_js_calls.find(id);
  if (it == pending_js_calls.end()) {
    return;
  }
  auto* call = it->second;
  // fluffos_js_result is reachable from page JS; ignore a duplicate
  // report for an id whose delivery is already scheduled.
  if (call->delivering) {
    return;
  }
  call->delivering = true;
  // The string's memory is freed by the JS caller on return; copy now,
  // deliver on the next tick.
  std::string text(result ? result : "");
  bool const ok = success != 0;
  add_walltime_event(std::chrono::milliseconds(0),
                     [call, ok, text] { deliver_js_result(call, ok, text); });
}

#ifdef F_JS_EXPORT
void f_js_export() {
  int const num_arg = st_num_arg;
  svalue_t* arg = sp - num_arg + 1;

  // Validate before any allocation: error() unwinds past C++ destructors.
  if (num_arg > 1 && arg[1].type != T_STRING && arg[1].type != T_FUNCTION) {
    error("Bad argument 2 to js_export(): expected string or function.\n");
  }

  std::string const name(arg[0].u.string);

  // Replace or remove an existing registration.
  auto it = g_js_exports.find(name);
  if (it != g_js_exports.end()) {
    auto* old = it->second;
    g_js_exports.erase(it);
    free_js_export(old);
  }

  int registered = 0;
  if (num_arg > 1) {
    auto* exp = new JsExport{};
    assign_svalue_no_free(&exp->call_back, &arg[1]);
    exp->ob_to_call = current_object;
    add_ref(current_object, "js_export");
    g_js_exports[name] = exp;
    registered = 1;
  }

  pop_n_elems(num_arg);
  push_number(registered);
}
#endif

#ifdef F_JS_EVAL
void f_js_eval() {
  char* result = js_bridge_eval(sp->u.string);
  std::string const text(result ? result : "");
  if (result != nullptr) {
    free(result);
  }
  pop_stack();
  copy_and_push_string(text.c_str());
}
#endif

#ifdef F_JS_CALL
void f_js_call() {
  static LPC_INT next_key = 0;

  int const num_arg = st_num_arg;
  svalue_t* arg = sp - num_arg + 1;

  const char* handler = arg[0].u.string;
  array_t* args = arg[1].u.arr;

  // Validate everything BEFORE any allocation: error() unwinds past C++
  // destructors, so no non-trivial object may be live when it fires.
  for (int i = 0; i < args->size; i++) {
    if (args->item[i].type != T_STRING) {
      error("Bad argument 2 to js_call(): element %d is not a string.\n", i);
    }
  }
  if (num_arg > 2 && arg[2].type != T_STRING && arg[2].type != T_FUNCTION) {
    error("Bad argument 3 to js_call(): expected string or function.\n");
  }

  // Marshal the args as a JSON array of strings.
  nlohmann::json jargs = nlohmann::json::array();
  for (int i = 0; i < args->size; i++) {
    jargs.push_back(args->item[i].u.string);
  }

  auto* call = new JsCall{};
  call->key = next_key++;
  call->want_callback = (num_arg > 2);
  if (call->want_callback) {
    assign_svalue_no_free(&call->call_back, &arg[2]);
    call->ob_to_call = current_object;
    add_ref(current_object, "js_call");
  }
  pending_js_calls[call->key] = call;

  // LPC strings are arbitrary bytes; replace invalid UTF-8 rather than
  // letting dump() throw (a std::exception would abort the wasm module).
  std::string const jargs_text =
      jargs.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
  js_bridge_invoke(call->key, handler, jargs_text.c_str());

  pop_n_elems(num_arg);
  push_number(call->key);
}
#endif
