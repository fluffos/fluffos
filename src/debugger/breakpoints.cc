// Source breakpoints: file:line -> bytecode address resolution over the
// per-program line tables, plus store maintenance across program load/free.
//
// Line-table encoding (produced by switch_to_line() in compiler/internal/
// icode.cc, decoded by find_line() in interpret.cc): prog->line_info is a
// sequence of [uint8 run_len][ADDRESS_TYPE abs_line] entries; a run covers
// the next run_len bytecode bytes and attributes them to absolute line
// abs_line, which translate_absolute_line() maps through prog->file_info to
// an (include-)file index + local line.  The first byte of a run is the first
// opcode generated for that source line, which is exactly where the
// instruction hook's `pc` will point when it is about to execute -- so run
// starts are our breakpoint addresses.

#include "base/std.h"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <set>
#include <unordered_set>

#include "vm/internal/base/machine.h"
#include "vm/internal/simulate.h"

#include "debugger/debugger.h"

namespace dbg {

namespace {

// "/std/room.c" | "std/room.lpc" | "/std/room" -> "std/room" (matches the
// driver's extension-blind object identity rules).
std::string strip_ext(std::string s) {
  if (s.size() > 4 && s.compare(s.size() - 4, 4, ".lpc") == 0) {
    s.erase(s.size() - 4);
  } else if (s.size() > 2 && s.compare(s.size() - 2, 2, ".c") == 0) {
    s.erase(s.size() - 2);
  }
  return s;
}

void collect_prog_rec(program_t* p, std::unordered_set<program_t*>& out) {
  if (!p || !out.insert(p).second) {
    return;
  }
  for (int i = 0; i < p->num_inherited; i++) {
    collect_prog_rec(p->inherit[i].prog, out);
  }
}

// Every live program: each object's program plus its transitive inherits.
void collect_programs(std::unordered_set<program_t*>& out) {
  for (object_t* ob = obj_list; ob; ob = ob->next_all) {
    collect_prog_rec(ob->prog, out);
  }
}

// File ids (1-based indexes into prog->strings) under which `canon` appears
// in this program's file_info table.  Empty set = program doesn't involve
// that source file.
std::set<int> file_ids_for(program_t* p, const std::string& canon) {
  std::set<int> ids;
  if (!p->file_info || !p->line_info) {
    return ids;
  }
  unsigned short* fi = p->file_info;
  auto* li_start = reinterpret_cast<unsigned char*>(fi + fi[1]);
  unsigned short* e = fi + 2;
  while (reinterpret_cast<unsigned char*>(e) < li_start) {
    int fid = e[1];
    if (fid >= 1 && fid <= p->num_strings && !ids.count(fid)) {
      if (strip_ext(p->strings[fid - 1]) == canon) {
        ids.insert(fid);
      }
    }
    e += 2;
  }
  return ids;
}

struct RunStart {
  int addr;  // bytecode offset of the run's first opcode
  int line;  // local line within the requested file
};

// All run starts in `p` that begin code for a line of one of `fids`' files.
// Consecutive runs for the same (file, line) collapse to their first start
// (>255-byte lines are split into multiple entries by the encoder).
std::vector<RunStart> line_run_starts(program_t* p, const std::set<int>& fids) {
  std::vector<RunStart> out;
  unsigned short* fi = p->file_info;
  unsigned char* li = p->line_info;
  auto* li_end = reinterpret_cast<unsigned char*>(reinterpret_cast<char*>(fi) + fi[0]);
  int addr = 0;
  int prev_file = -1, prev_line = -1;
  while (li < li_end) {
    int len = *li++;
    ADDRESS_TYPE abs_line;
    memcpy(&abs_line, li, sizeof(ADDRESS_TYPE));
    li += sizeof(ADDRESS_TYPE);
    int fidx = 0, lline = 0;
    translate_absolute_line(abs_line, &fi[2], &fidx, &lline);
    if (fids.count(fidx) && !(fidx == prev_file && lline == prev_line)) {
      out.push_back({addr, lline});
    }
    prev_file = fidx;
    prev_line = lline;
    addr += len;
  }
  return out;
}

// Parses a DAP hitCondition string: an optional comparator (> >= < <= == =
// != %) followed by an integer; a bare integer means >=N (VS Code's typed
// hit-count UI sends the raw text as-is, so this is a debug-adapter-defined
// grammar, not something DAP itself mandates). Empty is valid (no
// condition). Returns false with `err` set on anything else, including
// trailing garbage after the number.
bool parse_hit_condition(const std::string& cond, std::string& op, long& n, std::string& err) {
  if (cond.empty()) {
    op.clear();
    return true;
  }
  size_t i = 0;
  auto skip_ws = [&] {
    while (i < cond.size() && isspace(static_cast<unsigned char>(cond[i]))) {
      i++;
    }
  };
  skip_ws();
  if (cond.compare(i, 2, ">=") == 0) {
    op = ">=";
    i += 2;
  } else if (cond.compare(i, 2, "<=") == 0) {
    op = "<=";
    i += 2;
  } else if (cond.compare(i, 2, "==") == 0) {
    op = "==";
    i += 2;
  } else if (cond.compare(i, 2, "!=") == 0) {
    op = "!=";
    i += 2;
  } else if (i < cond.size() && cond[i] == '>') {
    op = ">";
    i += 1;
  } else if (i < cond.size() && cond[i] == '<') {
    op = "<";
    i += 1;
  } else if (i < cond.size() && cond[i] == '=') {
    op = "==";
    i += 1;
  } else if (i < cond.size() && cond[i] == '%') {
    op = "%";
    i += 1;
  } else {
    op = ">=";  // bare number
  }
  skip_ws();
  size_t digits_start = i;
  if (i < cond.size() && (cond[i] == '-' || cond[i] == '+')) {
    i++;
  }
  size_t num_start = i;
  while (i < cond.size() && isdigit(static_cast<unsigned char>(cond[i]))) {
    i++;
  }
  if (i == num_start) {
    err =
        "hitCondition must be an optional comparator (> >= < <= == = != %) followed by an "
        "integer, e.g. '>= 3' or '5'";
    return false;
  }
  n = strtol(cond.c_str() + digits_start, nullptr, 10);
  skip_ws();
  if (i != cond.size()) {
    err = "unexpected trailing text in hitCondition: '" + cond.substr(i) + "'";
    return false;
  }
  return true;
}

void register_addr(program_t* p, int offset, int bp_id) {
  auto& s = g_session;
  char* addr = p->program + offset;
  if (s.bp_addrs.emplace(addr, bp_id).second) {
    s.bp_by_prog[p].push_back(addr);
  }
}

// Re-resolve every stored breakpoint against the live program set.  Returns
// per-(path, index) verification results via the store itself.
void rebind_all() {
  auto& s = g_session;
  s.bp_addrs.clear();
  s.bp_by_prog.clear();
  // Every SrcBp, valid or not -- breakpoint_hit_should_stop() needs to find
  // it by id. Rebuilt unconditionally since set_breakpoints_request()
  // always replaces a whole canonical path's vector wholesale, which can
  // move every SrcBp in it to a new address.
  s.bp_by_id.clear();
  for (auto& [canon, list] : s.bps) {
    for (auto& bp : list) {
      s.bp_by_id[bp.id] = &bp;
    }
  }
  if (s.bps.empty()) {
    update_flags();
    return;
  }

  std::unordered_set<program_t*> progs;
  collect_programs(progs);

  for (auto& [canon, list] : s.bps) {
    // Gather candidate run starts across every program touching this file.
    std::vector<std::pair<program_t*, RunStart>> runs;
    for (program_t* p : progs) {
      auto fids = file_ids_for(p, canon);
      if (fids.empty()) {
        continue;
      }
      for (const auto& r : line_run_starts(p, fids)) {
        runs.push_back({p, r});
      }
    }
    for (auto& bp : list) {
      bp.verified = false;
      bp.actual_line = bp.req_line;
      if (!bp.hit_condition_valid) {
        continue;  // bad hitCondition syntax: permanently unverified
      }
      if (runs.empty()) {
        continue;
      }
      // Exact line match first; otherwise snap to the nearest following line
      // that has code.
      int best = INT_MAX;
      for (const auto& [p, r] : runs) {
        if (r.line >= bp.req_line && r.line < best) {
          best = r.line;
        }
      }
      if (best == INT_MAX) {
        continue;
      }
      bp.actual_line = best;
      bp.verified = true;
      for (const auto& [p, r] : runs) {
        if (r.line == best) {
          register_addr(p, r.addr, bp.id);
        }
      }
    }
  }
  update_flags();
}

}  // namespace

std::string canonical_lpc_path(const char* path) {
  if (!path) {
    return "";
  }
  const char* p = path;
  while (*p == '/') {
    p++;
  }
  return strip_ext(p);
}

// DAP setBreakpoints: replace the full breakpoint set for one source file and
// answer with the per-breakpoint verification results, in request order.
djson set_breakpoints_request(const djson& args) {
  auto& s = g_session;

  std::string client_path;
  if (args.contains("source") && args["source"].contains("path")) {
    client_path = args["source"]["path"].get<std::string>();
  }
  std::string canon = canonical_lpc_path(client_path.c_str());

  struct ReqBp {
    int line = 0;
    std::string hit_condition;
  };
  std::vector<ReqBp> reqs;
  if (args.contains("breakpoints")) {
    for (const auto& b : args["breakpoints"]) {
      reqs.push_back({b.value("line", 0), b.value("hitCondition", "")});
    }
  } else if (args.contains("lines")) {
    for (const auto& l : args["lines"]) {
      reqs.push_back({l.get<int>(), ""});
    }
  }

  s.bps.erase(canon);
  // Parallel to the new SrcBp list: a per-entry hitCondition parse error, or
  // empty for none. Kept separate from SrcBp itself -- it's only needed
  // once, to build this response.
  std::vector<std::string> hit_errs;
  if (!canon.empty() && !reqs.empty()) {
    auto& list = s.bps[canon];
    for (const auto& req : reqs) {
      SrcBp bp;
      bp.id = s.next_bp_id++;
      bp.req_line = req.line;
      bp.hit_condition = req.hit_condition;
      std::string op, err;
      long n = 0;
      bp.hit_condition_valid = parse_hit_condition(req.hit_condition, op, n, err);
      hit_errs.push_back(bp.hit_condition_valid ? "" : err);
      list.push_back(bp);
    }
  }
  rebind_all();

  djson result = djson::array();
  if (!canon.empty()) {
    auto it = s.bps.find(canon);
    if (it != s.bps.end()) {
      for (size_t i = 0; i < it->second.size(); i++) {
        const auto& bp = it->second[i];
        djson entry = {{"id", bp.id},
                       {"verified", bp.verified},
                       {"line", bp.verified ? bp.actual_line : bp.req_line}};
        if (i < hit_errs.size() && !hit_errs[i].empty()) {
          entry["message"] = hit_errs[i];
        }
        result.push_back(entry);
      }
    }
  }
  return djson{{"breakpoints", result}};
}

bool breakpoint_hit_should_stop(int bp_id) {
  auto& s = g_session;
  auto it = s.bp_by_id.find(bp_id);
  if (it == s.bp_by_id.end() || !it->second) {
    return true;  // shouldn't happen; fail open rather than silently never stopping
  }
  SrcBp& bp = *it->second;
  bp.hit_count++;
  if (bp.hit_condition.empty()) {
    return true;
  }
  std::string op, err;
  long n = 0;
  // Re-parses rather than caching the parsed op/n on SrcBp: this only runs
  // when `pc` already matched a registered breakpoint address, i.e. it's
  // already off the per-instruction hot path, so the cost is negligible
  // and one grammar (parse_hit_condition) stays the single source of truth
  // for both validation (setBreakpoints) and evaluation (here).
  if (!parse_hit_condition(bp.hit_condition, op, n, err)) {
    return true;  // shouldn't happen: a bad condition never gets an address registered
  }
  int count = bp.hit_count;
  if (op == ">=") return count >= n;
  if (op == ">") return count > n;
  if (op == "<=") return count <= n;
  if (op == "<") return count < n;
  if (op == "==") return count == n;
  if (op == "!=") return count != n;
  if (op == "%") return n != 0 && (count % n) == 0;
  return true;
}

void breakpoints_on_program_loaded(program_t* prog) {
  auto& s = g_session;
  if (s.bps.empty() || !prog) {
    return;
  }
  // Only rebind when the new program actually involves a breakpointed file.
  bool relevant = false;
  for (const auto& [canon, list] : s.bps) {
    if (!file_ids_for(prog, canon).empty()) {
      relevant = true;
      break;
    }
  }
  if (!relevant) {
    return;
  }
  // Snapshot verification state so newly-bound breakpoints get a DAP
  // `breakpoint` event (pending -> verified).
  std::map<int, bool> before;
  for (const auto& [canon, list] : s.bps) {
    for (const auto& bp : list) {
      before[bp.id] = bp.verified;
    }
  }
  rebind_all();
  if (s.attached) {
    for (const auto& [canon, list] : s.bps) {
      for (const auto& bp : list) {
        if (bp.verified && !before[bp.id]) {
          send_event("breakpoint",
                     {{"reason", "changed"},
                      {"breakpoint",
                       {{"id", bp.id}, {"verified", true}, {"line", bp.actual_line}}}});
        }
      }
    }
  }
}

void breakpoints_on_program_freed(program_t* prog) {
  auto& s = g_session;
  auto it = s.bp_by_prog.find(prog);
  if (it == s.bp_by_prog.end()) {
    return;
  }
  // The program's bytecode is about to be freed: drop its addresses NOW.
  // Verification state is refreshed lazily on the next load/setBreakpoints.
  for (char* addr : it->second) {
    s.bp_addrs.erase(addr);
  }
  s.bp_by_prog.erase(it);
  update_flags();
}

void breakpoints_clear_all() {
  auto& s = g_session;
  s.bps.clear();
  s.bp_addrs.clear();
  s.bp_by_prog.clear();
  s.bp_by_id.clear();
  update_flags();
}

}  // namespace dbg
