#include <gtest/gtest.h>
#include "base/package_api.h"

#include <climits>
#include <cmath>
#include <cstdio>
#include <fstream>

#include "mainlib.h"

#include "debugger/debug_hook.h"
#include "debugger/debugger.h"

class DebuggerTest : public ::testing::Test {
 public:
  static void SetUpTestSuite() {
    chdir(TESTSUITE_DIR);
    init_main("etc/config.test");
    vm_start();
  }

 protected:
  void SetUp() override {
    clear_state();
    dbg::breakpoints_clear_all();
    dbg::g_session.bps.clear();
  }

  void TearDown() override {
    dbg::breakpoints_clear_all();
    clear_state();
  }

  // Any call that dispatches into LPC (compiling/loading an object can run
  // create()/__INIT and master applies) needs a recovery point, exactly like
  // DriverTest::TestCompileDumpProgWorks in test_lpc.cc -- otherwise an
  // error() thrown with no established error_context hits the driver's
  // fatal() fallback and aborts the whole test binary instead of failing
  // just this one check.
  // callcreate=false (the default) skips create()/__INIT -- fine for tests
  // that only need the compiled program shape. Tests that read an actual
  // global VALUE (setVariable's array/mapping-element cases below need a
  // real array_t*/mapping_t*, not the uninitialized T_NUMBER(0) every
  // global starts as) must pass callcreate=true to run initializers.
  object_t* LoadFixture(const std::string& source, const char* virtual_name,
                        bool callcreate = false) {
    object_t* ob = nullptr;
    error_context_t econ{};
    save_context(&econ);
    try {
      ob = load_object_from_source(source, virtual_name, callcreate ? 1 : 0);
    } catch (...) {
      restore_context(&econ);
      ADD_FAILURE() << "load_object_from_source(" << virtual_name << ") threw";
      return nullptr;
    }
    pop_context(&econ);
    return ob;
  }

  // Deliberately not torn down: a freshly-loaded object's ref count reflects
  // it being alive in obj_list/otable, not a reference the caller owns, and
  // this test binary is short-lived and process-exit reclaims everything.
  // (destruct_object() followed by free_object() is the correct sequence
  // when teardown is actually needed, but exercising it here isn't worth
  // the risk: it uncovered a pre-existing dangling-obj_list_destruct bug --
  // dealloc_object() never unlinks a fully-freed object from the global
  // obj_list_destruct chain destruct_object() pushed it onto, so a LATER
  // destruct_object() call elsewhere can dereference the stale head --
  // unrelated to this feature and out of scope to fix here.)
};

// Regression for a real bug found while implementing local-name capture:
// debugmalloc.h's own comment says "the digit after + must be unique,
// range is 0-255" -- a GLOBAL id shared across every TAG_* category, not
// just within TAG_PERMANENT, because packages/develop/checkmemory.cc's
// check_string_stats() reads blocks[TAG_SHARED_STRING & 0xff] and
// blocks[TAG_MALLOC_STRING & 0xff], which folds away the category (high
// byte). TAG_LOCAL_NAMES was first picked as TAG_PERMANENT + 41, silently
// colliding with TAG_MALLOC_STRING (TAG_DATA + 41): every local-names
// block then got counted as a live malloc-string, and check_string_stats()
// failed the moment ANY program with captured names (e.g. master.lpc's
// connect()) was still loaded when a Debug-build LPC testsuite run's
// post-file check_memory() fired -- a hard, whole-suite-failing bug that
// only manifests with "debugger port" set, so it's easy to miss in normal
// CI. This doesn't re-run the full check (that needs a live driver boot,
// covered by manual validation -- see the fix commit message); it pins
// the specific arithmetic mistake so it can't silently come back.
TEST_F(DebuggerTest, LocalNamesTagDoesNotCollideWithAnotherCategory) {
  EXPECT_NE(TAG_LOCAL_NAMES & 0xff, TAG_MALLOC_STRING & 0xff);
  EXPECT_NE(TAG_LOCAL_NAMES & 0xff, TAG_SHARED_STRING & 0xff);
  EXPECT_NE(TAG_LOCAL_NAMES & 0xff, TAG_STRING & 0xff);
  EXPECT_NE(TAG_LOCAL_NAMES & 0xff, TAG_OBJ_VARS & 0xff);
  EXPECT_NE(TAG_LOCAL_NAMES & 0xff, TAG_LINENUMBERS & 0xff);
  EXPECT_NE(TAG_LOCAL_NAMES & 0xff, TAG_PROGRAM & 0xff);
}

TEST_F(DebuggerTest, CanonicalPathIsExtensionAndSlashBlind) {
  EXPECT_EQ(dbg::canonical_lpc_path("/std/room.c"), "std/room");
  EXPECT_EQ(dbg::canonical_lpc_path("/std/room.lpc"), "std/room");
  EXPECT_EQ(dbg::canonical_lpc_path("/std/room"), "std/room");
  EXPECT_EQ(dbg::canonical_lpc_path("std/room.c"), "std/room");
  EXPECT_EQ(dbg::canonical_lpc_path(""), "");
  EXPECT_EQ(dbg::canonical_lpc_path(nullptr), "");
}

// Exact hook API no-op contract when no client is attached: this is what
// testsuite/single/tests/efuns/debug_break.lpc pins from the LPC side, and
// what makes the feature truly zero-cost/zero-behavior-change when unused.
TEST_F(DebuggerTest, HookApiIsNoOpWhenDetached) {
  g_lpc_debug_flags = 0;
  EXPECT_EQ(lpc_debugger_attached(), 0);

  lpc_debugger_break();
  EXPECT_EQ(g_lpc_debug_flags, 0u) << "debug_break() must not arm anything while detached";

  lpc_debugger_on_error("test error", 0);
  EXPECT_FALSE(dbg::g_session.stopped);

  lpc_debugger_on_program_loaded(nullptr);
  lpc_debugger_on_program_freed(nullptr);
  SUCCEED();
}

TEST_F(DebuggerTest, SetBreakpointsResolvesExactAndSnapsToNextCodeLine) {
  // A small fixture with a blank line (no code) between two statements, and
  // named so canonical_lpc_path("/dbgtest/fixture") matches its filename.
  const char* src =
      "int counter;\n"    // line 1: global decl, no bytecode of its own
      "void bump() {\n"   // line 2
      "\n"                // line 3: blank, no code -- breakpoint must snap
      "  counter++;\n"    // line 4: real code
      "}\n";              // line 5
  object_t* ob = LoadFixture(src, "dbgtest/fixture");
  ASSERT_NE(ob, nullptr);
  ASSERT_NE(ob->prog, nullptr);

  dbg::djson args = {
      {"source", {{"path", "/dbgtest/fixture.c"}}},
      {"breakpoints", dbg::djson::array({{{"line", 4}}, {{"line", 3}}, {{"line", 999}}})},
  };
  dbg::djson resp = dbg::set_breakpoints_request(args);
  ASSERT_TRUE(resp.contains("breakpoints"));
  auto& bps = resp["breakpoints"];
  ASSERT_EQ(bps.size(), 3u);

  EXPECT_TRUE(bps[0]["verified"].get<bool>());
  EXPECT_EQ(bps[0]["line"].get<int>(), 4) << "exact code line must bind unchanged";

  EXPECT_TRUE(bps[1]["verified"].get<bool>());
  EXPECT_EQ(bps[1]["line"].get<int>(), 4) << "blank line must snap to the next line with code";

  EXPECT_FALSE(bps[2]["verified"].get<bool>()) << "a line past EOF can never verify";
}

TEST_F(DebuggerTest, SetBreakpointsAcceptsValidHitConditionSyntax) {
  object_t* ob = LoadFixture("int c;\nvoid bump() {\n  c++;\n}\n", "dbgtest/hitcond_valid");
  ASSERT_NE(ob, nullptr);

  for (const char* cond : {">= 3", "> 3", "<= 2", "< 2", "== 3", "!= 3", "% 2", "3", ""}) {
    dbg::djson args = {
        {"source", {{"path", "/dbgtest/hitcond_valid.c"}}},
        {"breakpoints", dbg::djson::array({{{"line", 3}, {"hitCondition", cond}}})},
    };
    dbg::djson resp = dbg::set_breakpoints_request(args);
    auto& bps = resp["breakpoints"];
    ASSERT_EQ(bps.size(), 1u) << "cond='" << cond << "'";
    EXPECT_TRUE(bps[0]["verified"].get<bool>()) << "cond='" << cond << "' " << resp.dump();
    EXPECT_FALSE(bps[0].contains("message")) << "cond='" << cond << "' " << resp.dump();
  }
}

TEST_F(DebuggerTest, SetBreakpointsRejectsInvalidHitConditionSyntax) {
  object_t* ob = LoadFixture("int c;\nvoid bump() {\n  c++;\n}\n", "dbgtest/hitcond_invalid");
  ASSERT_NE(ob, nullptr);

  for (const char* cond : {"abc", ">=", "3x", ">= 3 extra", "=="}) {
    dbg::djson args = {
        {"source", {{"path", "/dbgtest/hitcond_invalid.c"}}},
        {"breakpoints", dbg::djson::array({{{"line", 3}, {"hitCondition", cond}}})},
    };
    dbg::djson resp = dbg::set_breakpoints_request(args);
    auto& bps = resp["breakpoints"];
    ASSERT_EQ(bps.size(), 1u) << "cond='" << cond << "'";
    EXPECT_FALSE(bps[0]["verified"].get<bool>())
        << "cond='" << cond << "' must never verify, even though line 3 is real code";
    ASSERT_TRUE(bps[0].contains("message")) << "cond='" << cond << "'";
    EXPECT_FALSE(bps[0]["message"].get<std::string>().empty());
  }
}

// Exercises breakpoint_hit_should_stop() directly against hand-built SrcBp
// entries -- doesn't need a real stopped VM or even a real breakpoint
// address, since the function only touches g_session.bp_by_id/hit_count.
// The instruction hook's own use of it (only calling it when `pc` already
// matched bp_addrs) is covered by dap-smoke.js's repeat-connect hitCondition
// check, which is the only place a real, repeated address hit exists.
TEST_F(DebuggerTest, BreakpointHitShouldStopEvaluatesHitConditions) {
  auto& s = dbg::g_session;
  s.bps.clear();
  s.bp_by_id.clear();

  // Each condition gets its OWN single-element vector (a distinct
  // canonical-path key) so later inserts elsewhere in s.bps can never
  // reallocate -- and thus never invalidate -- an earlier bp's storage.
  auto make_bp = [&](const std::string& cond) -> int {
    dbg::SrcBp bp;
    bp.id = s.next_bp_id++;
    bp.hit_condition = cond;
    bp.hit_condition_valid = true;
    auto& list = s.bps["dbgtest/hitcount_probe_" + std::to_string(bp.id)];
    list.push_back(bp);
    s.bp_by_id[bp.id] = &list.back();
    return bp.id;
  };

  auto hits = [&](int id, int n) {
    std::vector<bool> out;
    for (int i = 0; i < n; i++) {
      out.push_back(dbg::breakpoint_hit_should_stop(id));
    }
    return out;
  };

  EXPECT_EQ(hits(make_bp(""), 3), (std::vector<bool>{true, true, true})) << "unconditional";
  EXPECT_EQ(hits(make_bp(">= 3"), 5), (std::vector<bool>{false, false, true, true, true}));
  EXPECT_EQ(hits(make_bp("3"), 5), (std::vector<bool>{false, false, true, true, true}))
      << "bare number defaults to >=";
  EXPECT_EQ(hits(make_bp("> 3"), 5), (std::vector<bool>{false, false, false, true, true}));
  EXPECT_EQ(hits(make_bp("<= 2"), 4), (std::vector<bool>{true, true, false, false}));
  EXPECT_EQ(hits(make_bp("< 2"), 4), (std::vector<bool>{true, false, false, false}));
  EXPECT_EQ(hits(make_bp("== 3"), 5), (std::vector<bool>{false, false, true, false, false}));
  EXPECT_EQ(hits(make_bp("!= 3"), 5), (std::vector<bool>{true, true, false, true, true}));
  EXPECT_EQ(hits(make_bp("% 2"), 4), (std::vector<bool>{false, true, false, true}));

  s.bps.clear();
  s.bp_by_id.clear();
}

TEST_F(DebuggerTest, BreakpointsOnUnloadedFileStayPending) {
  dbg::djson args = {
      {"source", {{"path", "/dbgtest/never_loaded"}}},
      {"breakpoints", dbg::djson::array({{{"line", 1}}})},
  };
  dbg::djson resp = dbg::set_breakpoints_request(args);
  auto& bps = resp["breakpoints"];
  ASSERT_EQ(bps.size(), 1u);
  EXPECT_FALSE(bps[0]["verified"].get<bool>());
}

TEST_F(DebuggerTest, ProgramFreeInvalidatesItsBreakpointAddresses) {
  // A bare, unattached program (same pattern as test_lpc.cc's
  // TestInMemoryCompileFile) isolates the invalidation hook from object
  // lifecycle entirely -- resolution via the real obj_list walk is already
  // covered by SetBreakpointsResolvesExactAndSnapsToNextCodeLine above.
  program_t* prog = compile_file("int bump() { return 1 + 1; }", "dbgtest_freeme_bare");
  ASSERT_NE(prog, nullptr);

  // Populate the address table the way set_breakpoints_request() would.
  char* addr = prog->program;
  dbg::g_session.bp_addrs[addr] = 1;
  dbg::g_session.bp_by_prog[prog].push_back(addr);
  ASSERT_FALSE(dbg::g_session.bp_addrs.empty());

  // Mirrors test_lpc.cc: a bare compiled program is released with
  // deallocate_program(), not free_prog() (no owner holds a counted ref).
  // This exercises the real hook wired into program.cc, not a direct call.
  deallocate_program(prog);

  EXPECT_TRUE(dbg::g_session.bp_addrs.empty())
      << "addresses into a freed program must not remain in the live lookup table";
  EXPECT_TRUE(dbg::g_session.bp_by_prog.empty());
}

// Regression test for a compiler line-attribution bug (now fixed) that
// used to make a breakpoint set inside an #include'd HEADER file never
// resolve. breakpoints.cc's file_ids_for()/line_run_starts() were always
// written generically (they walk every file referenced in a program's
// file_info table, not just prog->filename, and canonical_lpc_path()'s
// strip_ext() only touches .lpc/.c, leaving a .h path equally untouched on
// both the request and compiled-program sides) -- the debugger-side
// machinery was never the problem.
//
// Root cause (compiler-side): a `return <constant>;`/`return;` statement
// can be built ENTIRELY from new_node_no_line() nodes -- CREATE_RETURN
// wrapping a CREATE_NUMBER/CREATE_REAL/CREATE_STRING leaf, or nothing at
// all -- so i_generate_node()'s `if (expr->line && ...)` guard in icode.cc
// never calls switch_to_line() anywhere in that statement's codegen. Its
// bytecode then silently inherits whichever line was already active
// (line_being_generated), which is still its compile-start sentinel value
// of 0 if the statement is the first thing generated in the whole compile
// unit -- exactly what happens when a trivial function is the first thing
// defined in an #include'd header, since the compiler's implicit
// global-include-file prologue contributes no codegen of its own.
// translate_absolute_line() then resolves that bogus abs_line 0 to
// (includer file, line 0) instead of the header's real line.
//
// Fixed by having rule_return_void()/rule_return_expr()
// (grammar_rules_loops.cc) stamp a real line onto every EXPLICIT,
// user-written return statement right after CREATE_RETURN. CREATE_RETURN
// itself deliberately stays new_node_no_line() by default: it also
// synthesizes IMPLICIT returns (rule_func(), rule_primary_expr_anon_func())
// appended well after a body has finished parsing -- sometimes after an
// #include pop into a different file entirely -- where current_line no
// longer means anything for that body, so those call sites must not be
// given a line here.
TEST_F(DebuggerTest, BreakpointInsideIncludedHeaderFileResolves) {
  const char* header_path = "data/dbgtest_header_probe.h";
  {
    std::ofstream out(header_path);
    ASSERT_TRUE(out.good());
    out << "int header_fn() {\n"  // line 1
        << "  return 42;\n"       // line 2: the (currently unreachable) target
        << "}\n";                 // line 3
  }

  object_t* ob = LoadFixture(
      "#include \"/data/dbgtest_header_probe.h\"\n"
      "int use_it() { return header_fn(); }\n",
      "dbgtest_header_includer");
  ASSERT_NE(ob, nullptr);
  ASSERT_NE(ob->prog, nullptr);

  dbg::djson args = {
      {"source", {{"path", "/data/dbgtest_header_probe.h"}}},
      {"breakpoints", dbg::djson::array({{{"line", 2}}})},
  };
  dbg::djson resp = dbg::set_breakpoints_request(args);
  ASSERT_TRUE(resp.contains("breakpoints"));
  auto& bps = resp["breakpoints"];
  ASSERT_EQ(bps.size(), 1u) << resp.dump();
  EXPECT_TRUE(bps[0]["verified"].get<bool>()) << resp.dump();
  EXPECT_EQ(bps[0]["line"].get<int>(), 2) << resp.dump();

  std::remove(header_path);
}

// Companion regression test for the OTHER half of the fix: a function with
// NO explicit return has only the IMPLICIT `return 0;` that rule_func()
// itself synthesizes via CREATE_RETURN -- which deliberately keeps
// new_node_no_line() (see the comment on BreakpointInsideIncludedHeaderFileResolves
// above), since capturing current_line there would risk stamping a
// misleading line from whatever rule_func()'s reduction lookahead has
// already advanced past. Without a further fix this function's whole body
// would still resolve to abs_line 0. rule_func() (grammar_rules.cc) closes
// that gap by stamping the FUNCTION's own declaration line -- captured
// early and safely in rule_func_type(), before any lookahead can drift --
// onto the NODE_FUNCTION wrapper node itself, so i_generate_node()'s
// existing line-switch guard fires at function entry regardless of what
// (if anything) is inside.
TEST_F(DebuggerTest, BreakpointOnImplicitReturnHeaderFunctionResolves) {
  const char* header_path = "data/dbgtest_header_probe2.h";
  {
    std::ofstream out(header_path);
    ASSERT_TRUE(out.good());
    out << "void header_fn_empty() {\n"  // line 1: no explicit return
        << "}\n";                        // line 2
  }

  object_t* ob = LoadFixture(
      "#include \"/data/dbgtest_header_probe2.h\"\n"
      "int use_it() { header_fn_empty(); return 1; }\n",
      "dbgtest_header_includer2");
  ASSERT_NE(ob, nullptr);
  ASSERT_NE(ob->prog, nullptr);

  dbg::djson args = {
      {"source", {{"path", "/data/dbgtest_header_probe2.h"}}},
      {"breakpoints", dbg::djson::array({{{"line", 1}}})},
  };
  dbg::djson resp = dbg::set_breakpoints_request(args);
  ASSERT_TRUE(resp.contains("breakpoints"));
  auto& bps = resp["breakpoints"];
  ASSERT_EQ(bps.size(), 1u) << resp.dump();
  EXPECT_TRUE(bps[0]["verified"].get<bool>()) << resp.dump();
  EXPECT_EQ(bps[0]["line"].get<int>(), 1) << resp.dump();

  std::remove(header_path);
}

namespace {
function_t* find_function(program_t* prog, const char* name) {
  for (int i = 0; i < prog->num_functions_defined; i++) {
    if (prog->function_table[i].funcname && std::string(prog->function_table[i].funcname) == name) {
      return &prog->function_table[i];
    }
  }
  return nullptr;
}

std::string local_name_str(program_t* prog, function_t* fn, int slot) {
  short idx = fn->local_names[slot];
  if (idx < 0 || idx >= prog->num_strings) {
    return "";
  }
  return prog->strings[idx];
}
}  // namespace

// Regression coverage for the compiler-emitted local/argument name table
// (DESIGN.md §9): captured in rule_func() (grammar_rules.cc) whenever
// "debugger port" is set, consumed by inspect.cc's local_name_for().
// Exercised at the compiler level (program_t::function_table[i].local_names)
// rather than through a live stopped-VM session -- inspect.cc's own logic
// for resolving it at a real breakpoint is a thin, separately-reviewed
// lookup over this same table.
TEST_F(DebuggerTest, LocalNamesAreCapturedWhenEnabled) {
  auto saved_port = CONFIG_INT(__RC_DEBUGGER_PORT__);
  CONFIG_INT(__RC_DEBUGGER_PORT__) = 4711;

  program_t* prog = compile_file(
      "int add(int first, int second) {\n"
      "  int total;\n"
      "  total = first + second;\n"
      "  return total;\n"
      "}\n",
      "dbgtest_local_names");

  CONFIG_INT(__RC_DEBUGGER_PORT__) = saved_port;

  ASSERT_NE(prog, nullptr);
  function_t* fn = find_function(prog, "add");
  ASSERT_NE(fn, nullptr);
  ASSERT_EQ(fn->num_arg, 2);
  ASSERT_EQ(fn->num_local, 1);
  ASSERT_NE(fn->local_names, nullptr);

  EXPECT_EQ(local_name_str(prog, fn, 0), "first");
  EXPECT_EQ(local_name_str(prog, fn, 1), "second");
  EXPECT_EQ(local_name_str(prog, fn, 2), "total");

  deallocate_program(prog);
}

// The feature's whole point is zero behavior/cost change when unused:
// "debugger port" 0 (the default) must produce no local-name table at all.
TEST_F(DebuggerTest, LocalNamesNotCapturedWhenDebuggerPortIsZero) {
  auto saved_port = CONFIG_INT(__RC_DEBUGGER_PORT__);
  CONFIG_INT(__RC_DEBUGGER_PORT__) = 0;

  program_t* prog =
      compile_file("int add(int first, int second) { return first + second; }\n",
                   "dbgtest_local_names_off");

  CONFIG_INT(__RC_DEBUGGER_PORT__) = saved_port;

  ASSERT_NE(prog, nullptr);
  ASSERT_GT(prog->num_functions_defined, 0);
  for (int i = 0; i < prog->num_functions_defined; i++) {
    EXPECT_EQ(prog->function_table[i].local_names, nullptr);
  }
  deallocate_program(prog);
}

// Scoping caveat pinned by DESIGN.md §9's "Scoping honesty" note: a local
// declared inside a for() block goes out of scope via pop_n_locals() before
// the function ends, so rule_func()'s capture (which only sees
// locals_ptr[0..current_number_of_locals) at the very end) can't recover its
// name -- but the runtime SLOT it used stays permanently reserved
// (max_num_locals never shrinks), so a later top-level local can land at a
// higher slot number, leaving a real gap in the middle of the table. Must
// not crash walking that gap, and locals declared after it must still
// resolve correctly.
TEST_F(DebuggerTest, LocalNamesLeaveGapForScopedOutLoopVariable) {
  auto saved_port = CONFIG_INT(__RC_DEBUGGER_PORT__);
  CONFIG_INT(__RC_DEBUGGER_PORT__) = 4711;

  program_t* prog = compile_file(
      "int scoped() {\n"
      "  int total;\n"
      "  for (int i = 0; i < 3; i++) {\n"
      "    total += i;\n"
      "  }\n"
      "  int after;\n"
      "  after = total;\n"
      "  return after;\n"
      "}\n",
      "dbgtest_local_names_scoped");

  CONFIG_INT(__RC_DEBUGGER_PORT__) = saved_port;

  ASSERT_NE(prog, nullptr);
  function_t* fn = find_function(prog, "scoped");
  ASSERT_NE(fn, nullptr);
  ASSERT_NE(fn->local_names, nullptr);

  EXPECT_EQ(local_name_str(prog, fn, 0), "total");

  int total_slots = fn->num_arg + fn->num_local;
  bool found_after = false;
  for (int slot = 1; slot < total_slots; slot++) {
    if (local_name_str(prog, fn, slot) == "after") {
      found_after = true;
    }
  }
  EXPECT_TRUE(found_after) << "a local declared after the closed for-loop scope must still "
                              "resolve, even though the loop variable's slot does not";

  deallocate_program(prog);
}

namespace {
svalue_t* fixture_global(object_t* ob, const char* name) {
  unsigned short type;
  int idx = find_global_variable(ob->prog, name, &type, 0);
  return idx == -1 ? nullptr : &ob->variables[idx];
}
}  // namespace

// setVariable's object-global path (VarHandle::kObject), which -- unlike
// the frame-scoped arg/local path -- needs no live/stopped VM frame, just a
// resolvable object_t*, so it's exercisable directly here. The frame-scoped
// path is covered end-to-end by tools/dap-smoke.js (a real breakpoint hit
// against a real stopped VM).
TEST_F(DebuggerTest, SetVariableWritesObjectGlobal) {
  object_t* ob = LoadFixture("int counter = 10;\n", "dbgtest_setvar_global", /*callcreate=*/true);
  ASSERT_NE(ob, nullptr);
  svalue_t* counter = fixture_global(ob, "counter");
  ASSERT_NE(counter, nullptr);
  ASSERT_EQ(counter->type, T_NUMBER);
  ASSERT_EQ(counter->u.number, 10);

  dbg::g_session.handles.push_back({dbg::VarHandle::kObject, 0, ob});
  int ref = dbg::kHandleBase + static_cast<int>(dbg::g_session.handles.size()) - 1;

  std::string err;
  dbg::djson body =
      dbg::set_variable_request({{"variablesReference", ref}, {"name", "counter"}, {"value", "42"}},
                                err);
  EXPECT_FALSE(body.is_null()) << "err: " << err;
  EXPECT_EQ(body["value"].get<std::string>(), "42");
  EXPECT_EQ(counter->type, T_NUMBER);
  EXPECT_EQ(counter->u.number, 42);

  // A string literal must also work, and must correctly release the old
  // (now-overwritten) T_NUMBER value -- nothing to free there, but this
  // pins that assign_svalue(), not a hand-rolled overwrite, is used.
  body = dbg::set_variable_request(
      {{"variablesReference", ref}, {"name", "counter"}, {"value", "\"hello\""}}, err);
  EXPECT_FALSE(body.is_null()) << "err: " << err;
  ASSERT_EQ(counter->type, T_STRING);
  EXPECT_STREQ(counter->u.string, "hello");

  dbg::g_session.handles.clear();
}

TEST_F(DebuggerTest, SetVariableRejectsUnsupportedSyntaxAndLeavesValueUnchanged) {
  object_t* ob = LoadFixture("int counter = 10;\n", "dbgtest_setvar_reject", /*callcreate=*/true);
  ASSERT_NE(ob, nullptr);
  svalue_t* counter = fixture_global(ob, "counter");
  ASSERT_NE(counter, nullptr);

  dbg::g_session.handles.push_back({dbg::VarHandle::kObject, 0, ob});
  int ref = dbg::kHandleBase + static_cast<int>(dbg::g_session.handles.size()) - 1;

  std::string err;
  dbg::djson body = dbg::set_variable_request(
      {{"variablesReference", ref}, {"name", "counter"}, {"value", "({1,2,3})"}}, err);
  EXPECT_TRUE(body.is_null());
  EXPECT_FALSE(err.empty());
  EXPECT_EQ(counter->type, T_NUMBER);
  EXPECT_EQ(counter->u.number, 10) << "a rejected write must not touch the target";

  dbg::g_session.handles.clear();
}

TEST_F(DebuggerTest, SetVariableWritesArrayElement) {
  object_t* ob = LoadFixture("mixed *arr = ({100, 200, 300});\n", "dbgtest_setvar_array",
                             /*callcreate=*/true);
  ASSERT_NE(ob, nullptr);
  svalue_t* arr_sv = fixture_global(ob, "arr");
  ASSERT_NE(arr_sv, nullptr);
  ASSERT_EQ(arr_sv->type, T_ARRAY);

  dbg::g_session.handles.push_back({dbg::VarHandle::kArray, 0, arr_sv->u.arr});
  int ref = dbg::kHandleBase + static_cast<int>(dbg::g_session.handles.size()) - 1;

  std::string err;
  dbg::djson body = dbg::set_variable_request(
      {{"variablesReference", ref}, {"name", "[1]"}, {"value", "999"}}, err);
  EXPECT_FALSE(body.is_null()) << "err: " << err;
  EXPECT_EQ(arr_sv->u.arr->item[0].u.number, 100) << "only index 1 should change";
  EXPECT_EQ(arr_sv->u.arr->item[1].u.number, 999);
  EXPECT_EQ(arr_sv->u.arr->item[2].u.number, 300);

  dbg::g_session.handles.clear();
}

// Use-after-free regression: setVariable writes a scalar over a slot that held
// a COMPOUND value; assign_svalue() frees that compound. Any outstanding
// variablesReference handle that borrowed a raw pointer into the freed value
// (here: a handle for the nested array the client had expanded) is left
// dangling, and the next variables/setVariable on it reads array_t::size off
// freed memory -- a heap-use-after-free (crash / memory disclosure), reachable
// within a single valid stopped session using only documented DAP requests.
// The fix invalidates the whole handle table after any mutating setVariable;
// this test asserts that, and that re-using the stale handle is now a clean
// miss rather than a freed-memory read (it aborts under ASan without the fix).
TEST_F(DebuggerTest, SetVariableOverCompoundInvalidatesAliasingHandles) {
  object_t* ob = LoadFixture("mixed *arr = ({ ({9, 10}), 200 });\n",
                             "dbgtest_setvar_uaf", /*callcreate=*/true);
  ASSERT_NE(ob, nullptr);
  svalue_t* arr_sv = fixture_global(ob, "arr");
  ASSERT_NE(arr_sv, nullptr);
  ASSERT_EQ(arr_sv->type, T_ARRAY);
  ASSERT_EQ(arr_sv->u.arr->item[0].type, T_ARRAY) << "arr[0] must be the nested array";

  // Handle 0: the outer array (client expands `arr`).
  dbg::g_session.handles.push_back({dbg::VarHandle::kArray, 0, arr_sv->u.arr});
  int outer_ref = dbg::kHandleBase + static_cast<int>(dbg::g_session.handles.size()) - 1;
  // Handle 1: the nested array arr[0] (client expands arr[0] -> ({9,10})).
  dbg::g_session.handles.push_back(
      {dbg::VarHandle::kArray, 0, arr_sv->u.arr->item[0].u.arr});
  int inner_ref = dbg::kHandleBase + static_cast<int>(dbg::g_session.handles.size()) - 1;

  // Overwrite the outer element that holds the nested array with a scalar:
  // assign_svalue() frees ({9,10}).  inner_ref now aliases freed memory.
  std::string err;
  dbg::djson body = dbg::set_variable_request(
      {{"variablesReference", outer_ref}, {"name", "[0]"}, {"value", "42"}}, err);
  ASSERT_FALSE(body.is_null()) << "err: " << err;
  EXPECT_EQ(arr_sv->u.arr->item[0].type, T_NUMBER);
  EXPECT_EQ(arr_sv->u.arr->item[0].u.number, 42);

  // The fix: the mutating write dropped every handle, so the dangling
  // inner_ref can no longer be dereferenced.
  EXPECT_TRUE(dbg::g_session.handles.empty())
      << "setVariable must invalidate handles after freeing a compound";

  // Re-using the stale handle must be a clean miss, NOT a freed-memory read
  // (without the fix this line is a heap-use-after-free that ASan aborts on).
  dbg::djson vars = dbg::build_variables(inner_ref, /*start=*/0, /*count=*/10);
  EXPECT_TRUE(vars["variables"].empty());
  dbg::djson sv = dbg::set_variable_request(
      {{"variablesReference", inner_ref}, {"name", "[0]"}, {"value", "7"}}, err);
  EXPECT_TRUE(sv.is_null());

  dbg::g_session.handles.clear();
}

// Regression: a client-supplied array element name whose digits overflow an
// int ("[99999999999]") must be rejected cleanly, not throw. parse_index_name
// used std::stoi, which throws std::out_of_range here; that exception would
// unwind out of set_variable_request -> handle_request -> the libwebsockets C
// callback, aborting the whole driver. An attached client is already
// privileged, so this is a robustness/DoS fix, not a privilege boundary.
TEST_F(DebuggerTest, SetVariableRejectsOutOfRangeArrayIndexWithoutThrowing) {
  object_t* ob = LoadFixture("mixed *arr = ({100, 200, 300});\n",
                             "dbgtest_setvar_bigindex", /*callcreate=*/true);
  ASSERT_NE(ob, nullptr);
  svalue_t* arr_sv = fixture_global(ob, "arr");
  ASSERT_NE(arr_sv, nullptr);
  ASSERT_EQ(arr_sv->type, T_ARRAY);

  dbg::g_session.handles.push_back({dbg::VarHandle::kArray, 0, arr_sv->u.arr});
  int ref = dbg::kHandleBase + static_cast<int>(dbg::g_session.handles.size()) - 1;

  std::string err;
  // Both an int-overflowing index and a long-but-in-range-of-long index that
  // still exceeds INT_MAX must be rejected without throwing.
  for (const char* bad : {"[99999999999]", "[3000000000]"}) {
    err.clear();
    dbg::djson body = dbg::set_variable_request(
        {{"variablesReference", ref}, {"name", bad}, {"value", "1"}}, err);
    EXPECT_TRUE(body.is_null()) << "index " << bad << " should be rejected";
    EXPECT_FALSE(err.empty());
  }
  // Original array untouched.
  EXPECT_EQ(arr_sv->u.arr->item[0].u.number, 100);
  EXPECT_EQ(arr_sv->u.arr->item[1].u.number, 200);
  EXPECT_EQ(arr_sv->u.arr->item[2].u.number, 300);

  dbg::g_session.handles.clear();
}

TEST_F(DebuggerTest, SetVariableWritesMappingValue) {
  object_t* ob = LoadFixture("mapping m = ([\"hello\": \"world\"]);\n", "dbgtest_setvar_mapping",
                             /*callcreate=*/true);
  ASSERT_NE(ob, nullptr);
  svalue_t* map_sv = fixture_global(ob, "m");
  ASSERT_NE(map_sv, nullptr);
  ASSERT_EQ(map_sv->type, T_MAPPING);

  dbg::g_session.handles.push_back({dbg::VarHandle::kMapping, 0, map_sv->u.map});
  int ref = dbg::kHandleBase + static_cast<int>(dbg::g_session.handles.size()) - 1;

  std::string err;
  // The DAP `name` for a mapping entry is the KEY's preview text (a quoted
  // string here), matching what build_variables() renders.
  dbg::djson body = dbg::set_variable_request(
      {{"variablesReference", ref}, {"name", "\"hello\""}, {"value", "\"there\""}}, err);
  EXPECT_FALSE(body.is_null()) << "err: " << err;

  svalue_t* val = find_string_in_mapping(map_sv->u.map, "hello");
  ASSERT_NE(val, nullptr);
  ASSERT_EQ(val->type, T_STRING);
  EXPECT_STREQ(val->u.string, "there");

  dbg::g_session.handles.clear();
}

// Regression: the DAP `variables` request carries client-controlled
// `start`/`count` ints straight from JSON. build_variables() paged with
// `for (i = start; i < end; i++)` where `end = min(size, start + count)` and
// NEVER clamped `start` to >= 0. A hostile-or-buggy attached client sending
// {"start":-1} (or any negative start) drove the array/class/buffer/local/
// global render loops to index BEFORE element 0 -- an out-of-bounds heap read
// whose bytes were then serialized straight back to the client as a variable
// preview (memory disclosure), and could crash the driver outright. A large
// positive `start` additionally overflowed the signed `start + count`. An
// attached client is already privileged (and loopback attaches need no
// password), so this is a robustness/OOB-read fix rather than an auth-boundary
// one, but it MUST NOT read out of bounds regardless. ASan/UBSan over this
// test is what actually proves the negative index is never dereferenced.
TEST_F(DebuggerTest, BuildVariablesClampsHostileStartWithoutReadingOutOfBounds) {
  object_t* ob = LoadFixture("mixed *arr = ({100, 200, 300});\n",
                             "dbgtest_vars_hostile_start", /*callcreate=*/true);
  ASSERT_NE(ob, nullptr);
  svalue_t* arr_sv = fixture_global(ob, "arr");
  ASSERT_NE(arr_sv, nullptr);
  ASSERT_EQ(arr_sv->type, T_ARRAY);

  dbg::g_session.handles.push_back({dbg::VarHandle::kArray, 0, arr_sv->u.arr});
  int ref = dbg::kHandleBase + static_cast<int>(dbg::g_session.handles.size()) - 1;

  // Negative start clamps to 0: the full array renders, in order, and no index
  // before [0] is ever touched.
  dbg::djson neg = dbg::build_variables(ref, /*start=*/-1, /*count=*/100);
  ASSERT_TRUE(neg.contains("variables"));
  ASSERT_EQ(neg["variables"].size(), 3u) << neg.dump();
  EXPECT_EQ(neg["variables"][0]["value"].get<std::string>(), "100");
  EXPECT_EQ(neg["variables"][2]["value"].get<std::string>(), "300");

  // Deeply negative start with a small count: still clamped to [0, 3), never a
  // window of negative indices.
  dbg::djson deep = dbg::build_variables(ref, /*start=*/-1000000, /*count=*/5);
  EXPECT_EQ(deep["variables"].size(), 3u) << deep.dump();

  // Huge positive start must not overflow start + count and must page past the
  // end cleanly (empty), not wrap to a negative window.
  dbg::djson big = dbg::build_variables(ref, /*start=*/2147483647, /*count=*/100);
  EXPECT_EQ(big["variables"].size(), 0u) << big.dump();

  // The array itself must be completely untouched by any of the above.
  EXPECT_EQ(arr_sv->u.arr->item[0].u.number, 100);
  EXPECT_EQ(arr_sv->u.arr->item[1].u.number, 200);
  EXPECT_EQ(arr_sv->u.arr->item[2].u.number, 300);

  dbg::g_session.handles.clear();
}

// Object/file introspection (fluffos_objects / fluffos_object / fluffos_files
// custom DAP requests, §8.3): unlike everything above, these work on a
// LIVE, RUNNING mud -- no stopped frame or handle table involved -- so
// they're exercised directly against build_objects_list()/build_object_info()
// without any g_session setup. tools/dap-smoke.js covers the same requests
// end-to-end over the wire; these pin the underlying obj_list-walk logic in
// isolation.
TEST_F(DebuggerTest, BuildObjectsListFindsLoadedObjectByFilter) {
  object_t* ob = LoadFixture("int x;\n", "dbgtest_objlist_probe");
  ASSERT_NE(ob, nullptr);
  ASSERT_NE(ob->obname, nullptr);

  dbg::djson resp = dbg::build_objects_list("dbgtest_objlist_probe", 0, 0);
  ASSERT_TRUE(resp.contains("objects"));
  auto& objs = resp["objects"];
  ASSERT_EQ(objs.size(), 1u) << resp.dump();
  EXPECT_EQ(objs[0]["name"].get<std::string>(), std::string("/") + ob->obname);
  EXPECT_EQ(objs[0]["program"].get<std::string>(), std::string("/") + ob->prog->filename);
  EXPECT_FALSE(objs[0]["clone"].get<bool>());

  // A filter matching nothing must report zero matches, not everything.
  dbg::djson miss = dbg::build_objects_list("no_such_object_should_exist_zzz", 0, 0);
  EXPECT_EQ(miss["objects"].size(), 0u);
  EXPECT_EQ(miss["total"].get<int>(), 0);
}

TEST_F(DebuggerTest, BuildObjectsListPaginatesWithStartAndCount) {
  // Two objects sharing a filterable prefix so pagination has >1 match to
  // page across; unfiltered would also match every other object already
  // loaded by prior tests/fixtures in this same (long-lived) driver
  // process, which is exactly why a real total/pagination check needs its
  // own distinguishing filter rather than assuming a clean obj_list.
  ASSERT_NE(LoadFixture("int x;\n", "dbgtest_objpage_a"), nullptr);
  ASSERT_NE(LoadFixture("int x;\n", "dbgtest_objpage_b"), nullptr);

  dbg::djson page0 = dbg::build_objects_list("dbgtest_objpage_", 0, 1);
  EXPECT_EQ(page0["objects"].size(), 1u);
  EXPECT_EQ(page0["total"].get<int>(), 2);

  dbg::djson page1 = dbg::build_objects_list("dbgtest_objpage_", 1, 1);
  EXPECT_EQ(page1["objects"].size(), 1u);
  EXPECT_NE(page0["objects"][0]["name"].get<std::string>(),
            page1["objects"][0]["name"].get<std::string>())
      << "different pages must return different objects, not the same one twice";
}

TEST_F(DebuggerTest, BuildObjectInfoReportsFunctionsAndVariables) {
  // Both globals are given real initializers and loaded with callcreate=true
  // so __INIT() actually runs -- an uninitialized LPC variable's RUNTIME
  // representation is always the generic undefined value (T_NUMBER 0)
  // regardless of its declared static type, so an uninitialized `string`
  // would render as type "int" here, not "string".
  object_t* ob = LoadFixture(
      "int counter = 5;\n"
      "string label = \"hi\";\n"
      "int bump() { return ++counter; }\n"
      "int other() { return 0; }\n",
      "dbgtest_objinfo_probe", /*callcreate=*/true);
  ASSERT_NE(ob, nullptr);

  dbg::djson info = dbg::build_object_info(std::string("/") + ob->obname);
  ASSERT_TRUE(info["found"].get<bool>()) << info.dump();
  EXPECT_EQ(info["name"].get<std::string>(), std::string("/") + ob->obname);
  EXPECT_EQ(info["program"].get<std::string>(), std::string("/") + ob->prog->filename);
  EXPECT_FALSE(info["clone"].get<bool>());
  EXPECT_GE(info["functions"].get<int>(), 2) << "bump() and other() must both be counted";

  auto& vars = info["variables"];
  bool found_counter = false, found_label = false;
  for (auto& v : vars) {
    if (v["name"] == "counter") {
      found_counter = true;
      EXPECT_EQ(v["type"], "int");
    }
    if (v["name"] == "label") {
      found_label = true;
      EXPECT_EQ(v["type"], "string");
    }
  }
  EXPECT_TRUE(found_counter) << vars.dump();
  EXPECT_TRUE(found_label) << vars.dump();

  // A leading-slash-optional, non-existent, or destructed object must
  // report found:false cleanly rather than crashing (the object is
  // re-looked-up by name per request, precisely so destruction between two
  // fluffos/object calls is a clean miss, never a dangling pointer -- §8.3).
  dbg::djson missing = dbg::build_object_info("dbgtest_objinfo_probe_does_not_exist");
  EXPECT_FALSE(missing["found"].get<bool>());

  {
    // destruct_object() can run clean_up()/remove()-style LPC, so it needs
    // the same recovery-point guard as LoadFixture() above.
    error_context_t econ{};
    save_context(&econ);
    try {
      destruct_object(ob);
      pop_context(&econ);
    } catch (...) {
      restore_context(&econ);
      ADD_FAILURE() << "destruct_object() threw";
    }
  }
  dbg::djson afterDestruct = dbg::build_object_info(std::string("/") + ob->obname);
  EXPECT_FALSE(afterDestruct["found"].get<bool>())
      << "a destructed object must not be reported as found";
}

TEST_F(DebuggerTest, BuildFileListListsKnownTestsuiteDirectory) {
  dbg::djson resp = dbg::build_file_list("/single");
  ASSERT_TRUE(resp.contains("files"));
  bool found_master = false;
  for (auto& f : resp["files"]) {
    if (f["name"] == "master.lpc") {
      found_master = true;
      EXPECT_FALSE(f["directory"].get<bool>());
    }
  }
  EXPECT_TRUE(found_master) << resp.dump();
}

TEST_F(DebuggerTest, BuildFileListRejectsPathTraversal) {
  dbg::djson resp = dbg::build_file_list("/../../../etc");
  EXPECT_TRUE(resp.contains("error"));
  ASSERT_TRUE(resp.contains("files"));
  EXPECT_EQ(resp["files"].size(), 0u);
}

TEST_F(DebuggerTest, ReadMudlibFileReturnsRealContentAndRejectsTraversal) {
  std::string content, err;
  ASSERT_TRUE(dbg::read_mudlib_file("/single/master.lpc", content, err)) << err;
  EXPECT_NE(content.find("object connect()"), std::string::npos);

  std::string content2, err2;
  EXPECT_FALSE(dbg::read_mudlib_file("/../../../../etc/passwd", content2, err2));
  EXPECT_FALSE(err2.empty());
}

// Optional mudlib-side veto at attach (docs/apply/master/valid_debugger.md,
// DESIGN.md's security model, item D8). No live transport is needed to
// exercise this: dispatch_message() is the exact entry point the real
// WebSocket layer calls into (transport_lws.cc's LWS_CALLBACK_RECEIVE
// handler), and transport_send()/transport_kill_client() both already
// guard on `if (s.client)` -- with no live lws* they reduce to queuing
// into g_session.outq / a no-op, so the response can be read straight back
// out of outq. single/master.lpc's valid_debugger() approves everything
// except a "DENY_ME_DEBUGGER" sentinel, which a real client can never
// present (lws_get_peer_simple() always returns an actual peer address).
TEST_F(DebuggerTest, AttachSucceedsWhenValidDebuggerApproves) {
  auto& s = dbg::g_session;
  s.attached = false;
  s.authed = false;
  s.outq.clear();
  s.client_addr = "203.0.113.7";  // an arbitrary non-sentinel "real" peer IP

  dbg::dispatch_message(R"({"type":"request","seq":1,"command":"attach","arguments":{}})");

  // A successful attach queues the response FIRST, then an "initialized"
  // event (handle_request()'s attach branch) -- outq.front() is the
  // response; outq.back() would be the event, which has no "success" key.
  ASSERT_FALSE(s.outq.empty());
  dbg::djson resp = dbg::djson::parse(s.outq.front());
  EXPECT_TRUE(resp["success"].get<bool>()) << resp.dump();
  EXPECT_TRUE(s.attached);

  // Leave the session as this test found it: update_flags() recomputes
  // g_lpc_debug_flags purely from session state, so a bare `s.attached =
  // false` without it would leave the real (eval-loop-hot-path) flag
  // stuck reporting ATTACHED after this test returns.
  s.attached = false;
  s.authed = false;
  s.outq.clear();
  dbg::update_flags();
}

TEST_F(DebuggerTest, AttachDeniedWhenValidDebuggerRejects) {
  auto& s = dbg::g_session;
  s.attached = false;
  s.authed = false;
  s.outq.clear();
  s.client_addr = "DENY_ME_DEBUGGER";

  dbg::dispatch_message(R"({"type":"request","seq":1,"command":"attach","arguments":{}})");

  ASSERT_FALSE(s.outq.empty());
  dbg::djson resp = dbg::djson::parse(s.outq.back());
  EXPECT_FALSE(resp["success"].get<bool>()) << resp.dump();
  EXPECT_FALSE(s.attached) << "master::valid_debugger() denial must not attach the session";
  EXPECT_FALSE(s.authed);

  s.outq.clear();
}

// Regression: a client may send a DAP field whose JSON type is wrong (a
// numeric "type"/"command", a string "seq"). nlohmann's value<T>() THROWS
// type_error.302 on such a mismatch. Some of these reads happen OUTSIDE
// handle_request()'s try/catch -- dispatch_message()'s "type" probe, and
// send_response()'s "seq"/"command" echo, which handle_request()'s own catch
// handler re-invokes. dispatch_message() is the exact entry point the
// libwebsockets LWS_CALLBACK_RECEIVE handler calls; a throw escaping it would
// unwind through the C callback frame and abort the driver. Each of these once
// crashed a live driver (unauthenticated, pre-attach). If dispatch_message()
// throws here, this test binary aborts -- so reaching the assertions at all is
// the core guarantee; the response-shape checks confirm graceful handling.
TEST_F(DebuggerTest, MalformedFieldTypesAreHandledWithoutThrowing) {
  auto& s = dbg::g_session;
  s.attached = false;
  s.authed = false;
  s.client_addr = "203.0.113.7";

  // 1) Non-string "type": rejected before handle_request(), no response queued.
  s.outq.clear();
  dbg::dispatch_message(R"({"type":123})");
  EXPECT_TRUE(s.outq.empty()) << "non-request message must not produce a response";

  // 2) Non-string "command": handle_request() throws reading it, the catch
  //    handler must produce a failure response (and must not itself re-throw).
  s.outq.clear();
  dbg::dispatch_message(R"({"type":"request","seq":1,"command":123})");
  ASSERT_FALSE(s.outq.empty());
  {
    dbg::djson resp = dbg::djson::parse(s.outq.back());
    EXPECT_FALSE(resp["success"].get<bool>()) << resp.dump();
    EXPECT_EQ(resp["request_seq"].get<int>(), 1);
  }

  // 3) Non-integer "seq" on an otherwise valid request: send_response() must
  //    tolerate it (echo request_seq 0), not throw.
  s.outq.clear();
  dbg::dispatch_message(R"({"type":"request","seq":"abc","command":"initialize"})");
  ASSERT_FALSE(s.outq.empty());
  {
    dbg::djson resp = dbg::djson::parse(s.outq.front());
    EXPECT_TRUE(resp["success"].get<bool>()) << resp.dump();
    EXPECT_EQ(resp["request_seq"].get<int>(), 0);
    EXPECT_EQ(resp["command"].get<std::string>(), "initialize");
  }

  s.attached = false;
  s.authed = false;
  s.outq.clear();
  dbg::update_flags();
}

// A client-supplied JSON *number* whose stored form is float -- an outright
// float ({"line":1e18}) OR an integer literal too big for int64
// ({"start":99999999999999999999}, which nlohmann must store as a float) --
// used to reach nlohmann's static_cast<int>(json_float) in value<int>()/
// get<int>(). That cast is undefined behavior when the value is outside int's
// range: under the sanitizer/CI build (-fsanitize=undefined) it calls abort()
// DIRECTLY, which the dispatch try/catch (a C++ exception boundary) cannot
// intercept -- a hostile-or-buggy attached client could crash the whole
// driver. json_to_int()/json_arg_int() must coerce without ever performing
// that UB. This test fails (and, built with ENABLE_SANITIZER, aborts) if any
// coercion regresses to a raw value<int>()/get<int>().
TEST_F(DebuggerTest, ClientJsonIntCoercionNeverInvokesFloatCastUB) {
  using dbg::json_arg_int;
  using dbg::json_to_int;

  // Floats far outside int range: clamp, never UB.
  EXPECT_EQ(json_to_int(dbg::djson(1e18), 7), INT_MAX);
  EXPECT_EQ(json_to_int(dbg::djson(-1e18), 7), INT_MIN);
  EXPECT_EQ(json_to_int(dbg::djson(1e300), 7), INT_MAX);
  EXPECT_EQ(json_to_int(dbg::djson(-1e300), 7), INT_MIN);
  // In-range float truncates toward zero.
  EXPECT_EQ(json_to_int(dbg::djson(2.9), 7), 2);
  EXPECT_EQ(json_to_int(dbg::djson(-2.9), 7), -2);
  // NaN and non-numbers fall back to the default.
  EXPECT_EQ(json_to_int(dbg::djson(std::nan("")), 7), 7);
  EXPECT_EQ(json_to_int(dbg::djson("abc"), 7), 7);
  EXPECT_EQ(json_to_int(dbg::djson(true), 7), 7);
  EXPECT_EQ(json_to_int(dbg::djson::array(), 7), 7);
  // Ordinary integers pass through; oversized integers clamp.
  EXPECT_EQ(json_to_int(dbg::djson(42), 7), 42);
  EXPECT_EQ(json_to_int(dbg::djson(-5), 7), -5);
  EXPECT_EQ(json_to_int(dbg::djson(5000000000LL), 7), INT_MAX);
  EXPECT_EQ(json_to_int(dbg::djson(-5000000000LL), 7), INT_MIN);

  // An integer literal beyond int64 is parsed by nlohmann as a float; the
  // coercion must still be safe (this is the exact shape that aborted).
  dbg::djson huge = dbg::djson::parse("99999999999999999999999999");
  EXPECT_TRUE(huge.is_number_float()) << "precondition: nlohmann stores it as float";
  EXPECT_EQ(json_to_int(huge, 7), INT_MAX);

  // Object-keyed reader: missing key / non-object / present.
  dbg::djson obj = dbg::djson::parse(R"({"line":1e18,"start":3})");
  EXPECT_EQ(json_arg_int(obj, "line", 0), INT_MAX);
  EXPECT_EQ(json_arg_int(obj, "start", 0), 3);
  EXPECT_EQ(json_arg_int(obj, "missing", 9), 9);
  EXPECT_EQ(json_arg_int(dbg::djson::array(), "line", 9), 9);

  // End-to-end through the real dispatch path: an attached client sending the
  // hostile setBreakpoints / variables / stackTrace / setVariable payloads
  // must get an ordinary response, not a driver abort. (These aborted before
  // the fix, on the sanitizer build.)
  auto& s = dbg::g_session;
  s.attached = true;
  s.authed = true;
  s.stopped = true;  // variables/stackTrace/setVariable are gated on being stopped
  for (const char* payload : {
           R"({"type":"request","seq":1,"command":"setBreakpoints","arguments":{"source":{"path":"/x"},"breakpoints":[{"line":1e18}]}})",
           R"({"type":"request","seq":2,"command":"setBreakpoints","arguments":{"source":{"path":"/x"},"lines":[99999999999999999999]}})",
           R"({"type":"request","seq":3,"command":"variables","arguments":{"variablesReference":1e18,"start":-1e18,"count":1e18}})",
           R"({"type":"request","seq":4,"command":"stackTrace","arguments":{"startFrame":1e18,"levels":-1e18}})",
           R"({"type":"request","seq":5,"command":"setVariable","arguments":{"variablesReference":1e18,"name":"x","value":"1"}})",
       }) {
    s.outq.clear();
    dbg::dispatch_message(payload);  // must not abort
    ASSERT_FALSE(s.outq.empty()) << "no response for: " << payload;
  }
  s.attached = false;
  s.authed = false;
  s.stopped = false;
  s.outq.clear();
  dbg::breakpoints_clear_all();
  dbg::update_flags();
}

// Regression: a deeply-nested "arguments" value must not crash the driver.
// handle_request() used to COPY the arguments object (`djson args =
// msg["arguments"]`). nlohmann's parser and destructor are iterative, but its
// copy constructor recurses once per nesting level, so a client message like
// {"type":"request","command":"initialize","arguments":[[[[...(~100 KB)...]]]]}
// -- well under the 4 MiB inbound cap -- blew the C stack during that copy and
// took the whole driver down with SIGSEGV. Critically the copy happened BEFORE
// the attach/auth gate, so a merely-connected (unauthenticated) client could
// trigger it: a pre-attach DoS. dispatch_message() is the exact entry point
// the transport feeds; the fix binds `args` by const reference (no copy).
// Under ENABLE_SANITIZER this test stack-overflow-aborts without the fix.
TEST_F(DebuggerTest, DeeplyNestedArgumentsDoesNotOverflowStack) {
  auto& s = dbg::g_session;
  s.attached = false;  // prove the pre-attach reachability
  s.authed = false;
  s.stopped = false;
  s.client = nullptr;

  // Depth chosen well past the observed ~50k overflow threshold on an 8 MiB
  // stack, but small enough to stay far under the 4 MiB message cap.
  const int kDepth = 120000;

  // Deeply-nested array as the arguments value.
  {
    std::string payload = R"({"type":"request","seq":1,"command":"initialize","arguments":)";
    payload += std::string(kDepth, '[');
    payload += std::string(kDepth, ']');
    payload += "}";
    s.outq.clear();
    dbg::dispatch_message(payload);  // must not crash
    // initialize ignores arguments and still answers -> a response was queued,
    // proving we got past the (former) copy without dying.
    EXPECT_FALSE(s.outq.empty());
  }

  // Deeply-nested object as the arguments value (copy ctor recurses on objects
  // too).
  {
    std::string payload = R"({"type":"request","seq":2,"command":"initialize","arguments":)";
    for (int i = 0; i < kDepth; i++) payload += R"({"a":)";
    payload += "1";
    for (int i = 0; i < kDepth; i++) payload += "}";
    payload += "}";
    s.outq.clear();
    dbg::dispatch_message(payload);  // must not crash
    EXPECT_FALSE(s.outq.empty());
  }

  s.outq.clear();
  dbg::update_flags();
}
