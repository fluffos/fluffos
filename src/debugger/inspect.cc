// Read-only inspection: stack frames, scopes, variables, loaded sources,
// object/file browsing.  Frame walking mirrors dump_trace()/get_svalue_trace()
// (src/vm/internal/trace.cc): the TOP frame's live registers are the globals
// (current_prog/current_object/pc/fp); every lower frame at csp entry `p`
// reads its registers from p[1], because push_control_stack() saves the
// CALLER's state.
//
// While the VM is stopped nothing can mutate or free LPC values, so variable
// handles borrow raw pointers; the table is cleared on resume (debug_server),
// which is what DAP specifies for variablesReference lifetimes anyway.

#include "base/std.h"

#include <dirent.h>
#include <sys/stat.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <unordered_set>

#include "vm/internal/base/machine.h"
#include "vm/internal/base/mapping.h"
#include "vm/internal/simulate.h"
#include "vm/internal/otable.h"

#include "debugger/debugger.h"

namespace dbg {

namespace {

constexpr size_t kStringPreviewCap = 200;
constexpr int kDefaultPage = 1000;
constexpr size_t kMaxSourceBytes = 2u << 20;

struct FrameRegs {
  control_stack_t* entry = nullptr;  // identity: framekind + fr live here
  program_t* prog = nullptr;         // live registers of this frame:
  object_t* ob = nullptr;
  char* pc_ = nullptr;
  svalue_t* fp_ = nullptr;
  bool top = false;
};

bool get_frame(int idx, FrameRegs& out) {
  if (idx < 0 || idx >= frame_count()) {
    return false;
  }
  control_stack_t* p = csp - idx;
  out.entry = p;
  out.top = (p == csp);
  if (out.top) {
    out.prog = current_prog;
    out.ob = current_object;
    out.pc_ = pc;
    out.fp_ = fp;
  } else {
    out.prog = p[1].prog;
    out.ob = p[1].ob;
    out.pc_ = p[1].pc;
    out.fp_ = p[1].fp;
  }
  return out.prog != nullptr;
}

// Function name and arg/local counts for a frame (num_arg == -1 for frames
// with no LPC variables: catch / fake).  Mirrors get_trace_details().
void frame_func(const FrameRegs& f, std::string& name, int& num_arg, int& num_local) {
  num_arg = -1;
  num_local = -1;
  switch (f.entry->framekind & FRAME_MASK) {
    case FRAME_FUNCTION: {
      function_t* fn = &f.prog->function_table[f.entry->fr.table_index];
      name = fn->funcname ? fn->funcname : "<unknown>";
      num_arg = fn->num_arg;
      num_local = fn->num_local;
      break;
    }
    case FRAME_FUNP:
      name = "<function>";
      num_arg = f.entry->fr.funp->f.functional.num_arg;
      num_local = f.entry->fr.funp->f.functional.num_local;
      break;
    case FRAME_CATCH:
      name = "catch";
      break;
    case FRAME_FAKE:
      name = "<fake>";
      break;
    default:
      name = "<unknown frame>";
      break;
  }
}

// For the top frame `pc` points AT the opcode about to execute, but
// find_line() expects a mid-instruction pointer (return addresses, error
// pcs); query pc+1 so the first instruction of a line reports that line.
void frame_line(const FrameRegs& f, const char** file, int* line) {
  get_explicit_line_number_info(f.top ? f.pc_ + 1 : f.pc_, f.prog, file, line);
}

std::string preview_string(const char* s) {
  std::string out = "\"";
  size_t len = strlen(s);
  if (len > kStringPreviewCap) {
    out.append(s, kStringPreviewCap);
    char tail[64];
    snprintf(tail, sizeof(tail), "\"… (%zu chars)", len);
    out += tail;
  } else {
    out.append(s, len);
    out += "\"";
  }
  return out;
}

const char* type_name(const svalue_t* sv) {
  switch (sv->type) {
    case T_NUMBER:
      return "int";
    case T_REAL:
      return "float";
    case T_STRING:
      return "string";
    case T_ARRAY:
      return "array";
    case T_MAPPING:
      return "mapping";
    case T_OBJECT:
      return "object";
    case T_FUNCTION:
      return "function";
    case T_BUFFER:
      return "buffer";
    case T_CLASS:
      return "class";
    default:
      return "mixed";
  }
}

std::string preview(const svalue_t* sv) {
  char buf[128];
  switch (sv->type) {
    case T_NUMBER:
      return std::to_string(sv->u.number);
    case T_REAL:
      snprintf(buf, sizeof(buf), "%.10g", sv->u.real);
      return buf;
    case T_STRING:
      return preview_string(sv->u.string);
    case T_OBJECT:
      if (!sv->u.ob || (sv->u.ob->flags & O_DESTRUCTED)) {
        return "<destructed object>";
      }
      snprintf(buf, sizeof(buf), "OBJ(/%s)", sv->u.ob->obname);
      return buf;
    case T_ARRAY:
      snprintf(buf, sizeof(buf), "({ %d elements })", sv->u.arr->size);
      return buf;
    case T_CLASS:
      snprintf(buf, sizeof(buf), "class (%d members)", sv->u.arr->size);
      return buf;
    case T_MAPPING:
      snprintf(buf, sizeof(buf), "([ %u entries ])", MAP_COUNT(sv->u.map));
      return buf;
    case T_FUNCTION:
      return "(: function :)";
    case T_BUFFER:
      snprintf(buf, sizeof(buf), "<buffer %u bytes>", sv->u.buf->size);
      return buf;
    default:
      snprintf(buf, sizeof(buf), "<type 0x%x>", sv->type);
      return buf;
  }
}

// Real name for frame slot `slot` (0..num_arg-1 = arguments, num_arg.. =
// locals), if the compiler captured one (DESIGN.md §9, gated on "debugger
// port" being set at COMPILE time); nullptr means fall back to the
// argN/localN index name -- either "debugger port" was 0 when this program
// compiled, this is a FRAME_FUNP frame (lambdas: rule_func() is the only
// capture site), or the slot belonged to a for/switch-scoped local whose
// block had already closed by the time the function finished compiling
// (see the "Scoping honesty" note in the design doc).
const char* local_name_for(const FrameRegs& f, int slot) {
  if ((f.entry->framekind & FRAME_MASK) != FRAME_FUNCTION) {
    return nullptr;
  }
  function_t* fn = &f.prog->function_table[f.entry->fr.table_index];
  if (!fn->local_names || slot < 0 || slot >= fn->num_arg + fn->num_local) {
    return nullptr;
  }
  short idx = fn->local_names[slot];
  if (idx < 0 || idx >= f.prog->num_strings) {
    return nullptr;
  }
  return f.prog->strings[idx];
}

int make_handle(VarHandle h) {
  auto& s = g_session;
  s.handles.push_back(h);
  return kHandleBase + static_cast<int>(s.handles.size()) - 1;
}

// A DAP variable for one svalue; compound values get a child handle so the
// client can expand them lazily.
djson render_var(const std::string& name, svalue_t* sv) {
  int ref = 0;
  switch (sv->type) {
    case T_ARRAY:
      if (sv->u.arr->size > 0) {
        ref = make_handle({VarHandle::kArray, 0, sv->u.arr});
      }
      break;
    case T_CLASS:
      if (sv->u.arr->size > 0) {
        ref = make_handle({VarHandle::kClass, 0, sv->u.arr});
      }
      break;
    case T_MAPPING:
      if (MAP_COUNT(sv->u.map) > 0) {
        ref = make_handle({VarHandle::kMapping, 0, sv->u.map});
      }
      break;
    case T_BUFFER:
      if (sv->u.buf->size > 0) {
        ref = make_handle({VarHandle::kBuffer, 0, sv->u.buf});
      }
      break;
    case T_OBJECT:
      if (sv->u.ob && !(sv->u.ob->flags & O_DESTRUCTED) && sv->u.ob->prog &&
          sv->u.ob->prog->num_variables_total > 0) {
        ref = make_handle({VarHandle::kObject, 0, sv->u.ob});
      }
      break;
    default:
      break;
  }
  djson var = {{"name", name},
               {"value", preview(sv)},
               {"type", type_name(sv)},
               {"variablesReference", ref}};
  if (sv->type == T_ARRAY) {
    var["indexedVariables"] = sv->u.arr->size;
  } else if (sv->type == T_MAPPING) {
    var["namedVariables"] = MAP_COUNT(sv->u.map);
  }
  return var;
}

// Object globals: names resolved through the recursive inherited-variable
// walk (variable_name), values straight out of ob->variables -- the same
// pairing debug_info(2, ob) uses.
void render_globals(object_t* ob, int start, int count, djson& out) {
  if (!ob || (ob->flags & O_DESTRUCTED) || !ob->prog || !ob->variables) {
    return;
  }
  int total = ob->prog->num_variables_total;
  int end = std::min(total, start + count);
  for (int i = start; i < end; i++) {
    char* vname = variable_name(ob->prog, i);
    std::string name = vname ? vname : ("global" + std::to_string(i));
    out.push_back(render_var(name, &ob->variables[i]));
  }
}

// Resolve an object global by its display name (variable_name() over the
// inherited layout, same pairing render_globals() uses).
svalue_t* resolve_global(object_t* ob, const std::string& name, std::string& err) {
  if (!ob || (ob->flags & O_DESTRUCTED) || !ob->prog || !ob->variables) {
    err = "object is gone";
    return nullptr;
  }
  int total = ob->prog->num_variables_total;
  for (int i = 0; i < total; i++) {
    char* vname = variable_name(ob->prog, i);
    if (vname && name == vname) {
      return &ob->variables[i];
    }
  }
  err = "no such variable: " + name;
  return nullptr;
}

// Inverse of build_variables()'s array/class naming ("[3]" / "member [3]").
bool parse_index_name(const std::string& name, bool is_class, int& out) {
  std::string s = name;
  const std::string prefix = is_class ? "member [" : "[";
  if (s.compare(0, prefix.size(), prefix) != 0 || s.empty() || s.back() != ']') {
    return false;
  }
  s = s.substr(prefix.size(), s.size() - prefix.size() - 1);
  if (s.empty()) {
    return false;
  }
  for (char c : s) {
    if (!isdigit(static_cast<unsigned char>(c))) {
      return false;
    }
  }
  out = std::stoi(s);
  return true;
}

// Re-walks the same container cases build_variables() renders, returning a
// pointer to the ONE child matching `name` instead of the whole page --
// setVariable's DAP contract identifies the target by the exact `name` a
// prior `variables` response rendered for this same variablesReference.
// Only reachable while stopped (debug_server.cc gates the request), so
// nothing can mutate the frozen world out from under this lookup.
svalue_t* resolve_lvalue(int ref, const std::string& name, std::string& err) {
  auto& s = g_session;
  int idx = ref - kHandleBase;
  if (idx < 0 || idx >= static_cast<int>(s.handles.size())) {
    err = "unknown variablesReference";
    return nullptr;
  }
  VarHandle h = s.handles[idx];
  switch (h.kind) {
    case VarHandle::kScopeArgs:
    case VarHandle::kScopeLocals: {
      FrameRegs f;
      if (!get_frame(h.frame, f) || !f.fp_) {
        err = "stale frame";
        return nullptr;
      }
      std::string fname;
      int na, nl;
      frame_func(f, fname, na, nl);
      if (na < 0) {
        err = "frame has no variables";
        return nullptr;
      }
      bool args = (h.kind == VarHandle::kScopeArgs);
      int n = args ? na : (nl < 0 ? 0 : nl);
      const char* prefix = args ? "arg" : "local";
      svalue_t* base = args ? f.fp_ : f.fp_ + na;
      for (int i = 0; i < n; i++) {
        int slot = args ? i : (na + i);
        const char* real = local_name_for(f, slot);
        std::string candidate = real ? real : (prefix + std::to_string(i));
        if (candidate == name) {
          return &base[i];
        }
      }
      err = "no such variable: " + name;
      return nullptr;
    }
    case VarHandle::kScopeGlobals: {
      FrameRegs f;
      if (!get_frame(h.frame, f)) {
        err = "stale frame";
        return nullptr;
      }
      return resolve_global(f.ob, name, err);
    }
    case VarHandle::kObject:
      return resolve_global(static_cast<object_t*>(h.ptr), name, err);
    case VarHandle::kArray:
    case VarHandle::kClass: {
      auto* arr = static_cast<array_t*>(h.ptr);
      int wanted;
      if (!parse_index_name(name, h.kind == VarHandle::kClass, wanted) || wanted < 0 ||
          wanted >= arr->size) {
        err = "no such element: " + name;
        return nullptr;
      }
      return &arr->item[wanted];
    }
    case VarHandle::kMapping: {
      // Keys are matched against the same (truncated) preview text
      // build_variables() renders as the DAP `name` -- exact for typical
      // int/string keys, but not a true identity: a preview collision
      // (two keys rendering identically, or a >60-char key indistinguishable
      // from its truncation) is rejected rather than risking a write to the
      // wrong entry.
      auto* m = static_cast<mapping_t*>(h.ptr);
      svalue_t* found = nullptr;
      int matches = 0;
      for (unsigned int b = 0; b < m->table_size; b++) {
        for (mapping_node_t* node = m->table[b]; node; node = node->next) {
          std::string key = preview(&node->values[0]);
          if (key.size() > 60) {
            key.resize(60);
            key += "…";
          }
          if (key == name) {
            matches++;
            found = &node->values[1];
          }
        }
      }
      if (matches == 0) {
        err = "no such key: " + name;
        return nullptr;
      }
      if (matches > 1) {
        err = "key preview '" + name +
              "' is ambiguous (multiple keys render identically); not editable via setVariable";
        return nullptr;
      }
      return found;
    }
    default:
      err = "this variable is not editable";
      return nullptr;
  }
}

// Deliberately minimal: integer, float, or a double-quoted string literal
// (with the common \\ \" \n \t \r escapes) -- never arrays/mappings/objects/
// function literals, which would need the compiler (phase 3's `evaluate`).
bool parse_literal(const std::string& text, svalue_t& out, std::string& err) {
  size_t b = text.find_first_not_of(" \t\r\n");
  size_t e = text.find_last_not_of(" \t\r\n");
  if (b == std::string::npos) {
    err = "empty value";
    return false;
  }
  std::string t = text.substr(b, e - b + 1);

  if (t.size() >= 2 && t.front() == '"' && t.back() == '"') {
    std::string decoded;
    for (size_t i = 1; i + 1 < t.size(); i++) {
      char c = t[i];
      if (c == '\\' && i + 2 < t.size()) {
        char n = t[i + 1];
        switch (n) {
          case 'n':
            decoded += '\n';
            i++;
            break;
          case 't':
            decoded += '\t';
            i++;
            break;
          case 'r':
            decoded += '\r';
            i++;
            break;
          case '"':
            decoded += '"';
            i++;
            break;
          case '\\':
            decoded += '\\';
            i++;
            break;
          default:
            decoded += c;
            break;
        }
      } else {
        decoded += c;
      }
    }
    out.type = T_STRING;
    out.subtype = STRING_MALLOC;
    out.u.string = string_copy(decoded.c_str(), "dbg setVariable");
    return true;
  }

  char* endp = nullptr;
  long long ival = strtoll(t.c_str(), &endp, 10);
  if (endp == t.c_str() + t.size() && endp != t.c_str()) {
    out.type = T_NUMBER;
    out.subtype = 0;
    out.u.number = static_cast<LPC_INT>(ival);
    return true;
  }

  endp = nullptr;
  double fval = strtod(t.c_str(), &endp);
  if (endp == t.c_str() + t.size() && endp != t.c_str()) {
    out.type = T_REAL;
    out.subtype = 0;
    out.u.real = fval;
    return true;
  }

  err =
      "unsupported value syntax; only an integer, float, or double-quoted string literal is "
      "supported here (arrays/mappings/objects/functions need the phase-3 expression evaluator)";
  return false;
}

bool sanitize_mudlib_path(const std::string& in, std::string& rel) {
  if (in.empty() || in[0] != '/') {
    return false;
  }
  // Reject any ".." component; collapse duplicate slashes as we go.
  std::string cleaned;
  size_t i = 0;
  while (i < in.size()) {
    while (i < in.size() && in[i] == '/') {
      i++;
    }
    size_t j = i;
    while (j < in.size() && in[j] != '/') {
      j++;
    }
    std::string comp = in.substr(i, j - i);
    if (comp == "..") {
      return false;
    }
    if (!comp.empty() && comp != ".") {
      if (!cleaned.empty()) {
        cleaned += '/';
      }
      cleaned += comp;
    }
    i = j;
  }
  rel = cleaned;
  return true;
}

std::string mudlib_real_path(const std::string& rel) {
  const char* root = CONFIG_STR(__MUD_LIB_DIR__);
  std::string real = root ? root : ".";
  if (!rel.empty()) {
    real += "/";
    real += rel;
  }
  return real;
}

}  // namespace

int frame_count() {
  if (!current_prog || csp < &control_stack[0]) {
    return 0;
  }
  return static_cast<int>(csp - &control_stack[0]) + 1;
}

djson build_stack_trace(int start, int levels) {
  int total = frame_count();
  if (levels <= 0) {
    levels = total;
  }
  djson frames = djson::array();
  for (int i = start; i < total && static_cast<int>(frames.size()) < levels; i++) {
    FrameRegs f;
    if (!get_frame(i, f)) {
      continue;
    }
    std::string fname;
    int na, nl;
    frame_func(f, fname, na, nl);
    const char* file;
    int line;
    frame_line(f, &file, &line);

    std::string path = std::string("/") + file;
    std::string base = path.substr(path.rfind('/') + 1);
    djson frame = {{"id", i},
                   {"name", fname + " (/" + (f.ob ? f.ob->obname : "?") + ")"},
                   {"source", {{"name", base}, {"path", path}}},
                   {"line", line},
                   {"column", 1}};
    short kind = f.entry->framekind & FRAME_MASK;
    if (kind == FRAME_CATCH || kind == FRAME_FAKE) {
      frame["presentationHint"] = "subtle";
    }
    frames.push_back(frame);
  }
  return djson{{"stackFrames", frames}, {"totalFrames", total}};
}

djson build_scopes(int frame_id) {
  FrameRegs f;
  djson scopes = djson::array();
  if (!get_frame(frame_id, f)) {
    return djson{{"scopes", scopes}};
  }
  std::string fname;
  int na, nl;
  frame_func(f, fname, na, nl);
  if (na >= 0) {
    scopes.push_back({{"name", "Arguments"},
                      {"presentationHint", "arguments"},
                      {"variablesReference", make_handle({VarHandle::kScopeArgs, frame_id})},
                      {"namedVariables", na},
                      {"expensive", false}});
    scopes.push_back({{"name", "Locals"},
                      {"presentationHint", "locals"},
                      {"variablesReference", make_handle({VarHandle::kScopeLocals, frame_id})},
                      {"namedVariables", nl < 0 ? 0 : nl},
                      {"expensive", false}});
  }
  if (f.ob && !(f.ob->flags & O_DESTRUCTED) && f.ob->prog) {
    scopes.push_back({{"name", "Object variables"},
                      {"variablesReference", make_handle({VarHandle::kScopeGlobals, frame_id})},
                      {"namedVariables", f.ob->prog->num_variables_total},
                      {"expensive", false}});
  }
  return djson{{"scopes", scopes}};
}

djson build_variables(int ref, int start, int count) {
  auto& s = g_session;
  djson vars = djson::array();
  int idx = ref - kHandleBase;
  if (idx < 0 || idx >= static_cast<int>(s.handles.size())) {
    return djson{{"variables", vars}};
  }
  if (count <= 0) {
    count = kDefaultPage;
  }
  VarHandle h = s.handles[idx];

  switch (h.kind) {
    case VarHandle::kScopeArgs:
    case VarHandle::kScopeLocals: {
      FrameRegs f;
      if (!get_frame(h.frame, f) || !f.fp_) {
        break;
      }
      std::string fname;
      int na, nl;
      frame_func(f, fname, na, nl);
      if (na < 0) {
        break;
      }
      bool args = (h.kind == VarHandle::kScopeArgs);
      int n = args ? na : (nl < 0 ? 0 : nl);
      const char* prefix = args ? "arg" : "local";
      svalue_t* base = args ? f.fp_ : f.fp_ + na;
      int end = std::min(n, start + count);
      for (int i = start; i < end; i++) {
        int slot = args ? i : (na + i);
        const char* real = local_name_for(f, slot);
        std::string name = real ? real : (prefix + std::to_string(i));
        vars.push_back(render_var(name, &base[i]));
      }
      break;
    }
    case VarHandle::kScopeGlobals: {
      FrameRegs f;
      if (get_frame(h.frame, f)) {
        render_globals(f.ob, start, count, vars);
      }
      break;
    }
    case VarHandle::kObject:
      render_globals(static_cast<object_t*>(h.ptr), start, count, vars);
      break;
    case VarHandle::kArray:
    case VarHandle::kClass: {
      auto* arr = static_cast<array_t*>(h.ptr);
      int end = std::min(arr->size, start + count);
      for (int i = start; i < end; i++) {
        const char* prefix = (h.kind == VarHandle::kClass) ? "member " : "";
        vars.push_back(render_var(prefix + std::string("[") + std::to_string(i) + "]",
                                  &arr->item[i]));
      }
      break;
    }
    case VarHandle::kMapping: {
      auto* m = static_cast<mapping_t*>(h.ptr);
      int seen = 0;
      int end = start + count;
      for (unsigned int b = 0; b < m->table_size && seen < end; b++) {
        for (mapping_node_t* node = m->table[b]; node && seen < end; node = node->next) {
          if (seen >= start) {
            std::string key = preview(&node->values[0]);
            if (key.size() > 60) {
              key.resize(60);
              key += "…";
            }
            vars.push_back(render_var(key, &node->values[1]));
          }
          seen++;
        }
      }
      break;
    }
    case VarHandle::kBuffer: {
      auto* buf = static_cast<buffer_t*>(h.ptr);
      int end = std::min(static_cast<int>(buf->size), start + count);
      for (int i = start; i < end; i++) {
        djson var = {{"name", "[" + std::to_string(i) + "]"},
                     {"value", std::to_string(buf->item[i])},
                     {"type", "int"},
                     {"variablesReference", 0}};
        vars.push_back(var);
      }
      break;
    }
  }
  return djson{{"variables", vars}};
}

djson set_variable_request(const djson& args, std::string& err) {
  int ref = args.value("variablesReference", 0);
  std::string name = args.value("name", "");
  std::string value_text = args.value("value", "");

  svalue_t* target = resolve_lvalue(ref, name, err);
  if (!target) {
    return djson();
  }

  svalue_t new_val{};
  if (!parse_literal(value_text, new_val, err)) {
    return djson();
  }

  assign_svalue(target, &new_val);
  free_svalue(&new_val, "dbg setVariable");

  return djson{{"value", preview(target)}, {"type", type_name(target)}, {"variablesReference", 0}};
}

djson build_loaded_sources() {
  std::unordered_set<program_t*> seen;
  djson sources = djson::array();
  for (object_t* ob = obj_list; ob; ob = ob->next_all) {
    program_t* p = ob->prog;
    if (!p || !seen.insert(p).second) {
      continue;
    }
    std::string path = std::string("/") + p->filename;
    sources.push_back({{"name", path.substr(path.rfind('/') + 1)}, {"path", path}});
  }
  return djson{{"sources", sources}};
}

djson build_objects_list(const std::string& filter, int start, int count) {
  if (count <= 0) {
    count = kDefaultPage;
  }
  djson objects = djson::array();
  int matched = 0;
  for (object_t* ob = obj_list; ob; ob = ob->next_all) {
    if (!ob->obname) {
      continue;
    }
    if (!filter.empty() && !strstr(ob->obname, filter.c_str())) {
      continue;
    }
    if (matched >= start && matched < start + count) {
      objects.push_back({{"name", std::string("/") + ob->obname},
                         {"program", ob->prog ? std::string("/") + ob->prog->filename : ""},
                         {"clone", (ob->flags & O_CLONE) != 0}});
    }
    matched++;
  }
  return djson{{"objects", objects}, {"total", matched}};
}

djson build_object_info(const std::string& name) {
  const char* n = name.c_str();
  while (*n == '/') {
    n++;
  }
  object_t* ob = ObjectTable::instance().find(n);
  if (!ob || (ob->flags & O_DESTRUCTED)) {
    return djson{{"found", false}};
  }
  djson info = {{"found", true},
                {"name", std::string("/") + ob->obname},
                {"program", ob->prog ? std::string("/") + ob->prog->filename : ""},
                {"clone", (ob->flags & O_CLONE) != 0}};
  if (ob->prog) {
    djson inherits = djson::array();
    for (int i = 0; i < ob->prog->num_inherited; i++) {
      inherits.push_back(std::string("/") + ob->prog->inherit[i].prog->filename);
    }
    info["inherits"] = inherits;
    info["functions"] = ob->prog->num_functions_defined;
    // Global values as previews only: this request is legal while the VM is
    // RUNNING, so no handles (handles exist only while stopped).
    djson globals = djson::array();
    if (ob->variables) {
      int total = ob->prog->num_variables_total;
      for (int i = 0; i < total; i++) {
        char* vname = variable_name(ob->prog, i);
        svalue_t* sv = &ob->variables[i];
        globals.push_back({{"name", vname ? vname : ""},
                           {"value", preview(sv)},
                           {"type", type_name(sv)}});
      }
    }
    info["variables"] = globals;
  }
  return info;
}

djson build_file_list(const std::string& path) {
  std::string rel;
  djson entries = djson::array();
  if (!sanitize_mudlib_path(path, rel)) {
    return djson{{"error", "invalid path"}, {"files", entries}};
  }
  std::string real = mudlib_real_path(rel);
  DIR* d = opendir(real.c_str());
  if (!d) {
    return djson{{"error", "cannot open directory"}, {"files", entries}};
  }
  struct dirent* de;
  while ((de = readdir(d))) {
    if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) {
      continue;
    }
    struct stat st;
    std::string full = real + "/" + de->d_name;
    bool is_dir = false;
    int64_t size = 0;
    if (stat(full.c_str(), &st) == 0) {
      is_dir = S_ISDIR(st.st_mode);
      size = st.st_size;
    }
    entries.push_back({{"name", de->d_name}, {"directory", is_dir}, {"size", size}});
  }
  closedir(d);
  return djson{{"files", entries}};
}

bool read_mudlib_file(const std::string& path, std::string& out, std::string& err) {
  std::string rel;
  if (!sanitize_mudlib_path(path, rel)) {
    err = "invalid path";
    return false;
  }
  std::ifstream f(mudlib_real_path(rel), std::ios::binary);
  if (!f) {
    err = "cannot open file";
    return false;
  }
  std::ostringstream ss;
  ss << f.rdbuf();
  out = ss.str();
  if (out.size() > kMaxSourceBytes) {
    err = "file too large";
    return false;
  }
  return true;
}

}  // namespace dbg
