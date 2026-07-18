#include <gtest/gtest.h>
#include "base/package_api.h"

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
  object_t* LoadFixture(const std::string& source, const char* virtual_name) {
    object_t* ob = nullptr;
    error_context_t econ{};
    save_context(&econ);
    try {
      ob = load_object_from_source(source, virtual_name, 0);
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
