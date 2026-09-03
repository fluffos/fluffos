#include <gtest/gtest.h>
#include "base/package_api.h"
#include "packages_missing_efuns.autogen.h"

#include "mainlib.h"

#include "comm.h"
#include "user.h"
#include "compiler/internal/compiler.h"

namespace {
// Runs `fn` (arbitrary LPC-triggering driver code -- load_object_from_source,
// destruct_object, free_object, ... can all run create()/__INIT/applies)
// under a proper recovery point, matching the pattern DriverTest's other
// tests use inline. Without this, an error() thrown with no established
// error_context hits the driver's fatal() fallback and aborts the whole
// test binary instead of failing just one check.
template <typename F>
void RunGuarded(F&& fn) {
  error_context_t econ{};
  save_context(&econ);
  try {
    fn();
  } catch (...) {
    restore_context(&econ);
    ADD_FAILURE() << "unexpected error() during test";
    return;
  }
  pop_context(&econ);
}
}  // namespace

// Test fixture class
class DriverTest : public ::testing::Test {
 public:
  static void SetUpTestSuite() {
    chdir(TESTSUITE_DIR);
    // Initialize libevent, This should be done before executing LPC.
    auto* base = init_main("etc/config.test");
    vm_start();
  }

 protected:
  void SetUp() override { clear_state(); }

  void TearDown() override { clear_state(); }
};

TEST_F(DriverTest, TestCompileDumpProgWorks) {
  current_object = master_ob;
  const char* file = "single/master.c";
  struct object_t* obj = nullptr;

  error_context_t econ{};
  save_context(&econ);
  try {
    obj = find_object(file);
  } catch (...) {
    restore_context(&econ);
    FAIL();
  }
  pop_context(&econ);

  ASSERT_NE(obj, nullptr);
  ASSERT_NE(obj->prog, nullptr);

  dump_prog(obj->prog, stdout, 1 | 2);

  free_object(&obj, "DriverTest::TestCompileDumpProgWorks");
}

TEST_F(DriverTest, TestInMemoryCompileFile) {
  program_t* prog = nullptr;

  prog = compile_file("void test() {}", "test");

  ASSERT_NE(prog, nullptr);
  deallocate_program(prog);
}

TEST_F(DriverTest, TestInMemoryCompileFileFail) {
  program_t* prog = nullptr;
  prog = compile_file("aksdljfaljdfiasejfaeslfjsaef", "test");

  ASSERT_EQ(prog, nullptr);
}

TEST_F(DriverTest, TestValidLPC_FunctionDeafultArgument) {
  const char* source = R"(
// default case
void test1() {
}

// default case
void test2(int a, int b) {
  ASSERT_EQ(a, 1);
  ASSERT_EQ(b, 2);
}

// varargs
void test3(int a, int* b ...) {
  ASSERT_EQ(a, 1);
  ASSERT_EQ(b[0], 2);
  ASSERT_EQ(b[1], 3);
  ASSERT_EQ(b[2], 4);
  ASSERT_EQ(b[3], 5);
}

// can have multiple trailing arguments with a FP for calculating default value
void test4(int a, string b: (: "str" :), int c: (: 0 :)) {
  switch(a) {
    case 1: {
      ASSERT_EQ("str", b);
      ASSERT_EQ(0, c);
      break;
    }
    case 2: {
      ASSERT_EQ("aaa", b);
      ASSERT_EQ(0, c);
      break;
    }
    case 3: {
      ASSERT_EQ("bbb", b);
      ASSERT_EQ(3, c);
      break;
    }
  }
}

void do_tests() {
    test1();
    test2(1, 2);
    test3(1, 2, 3, 4, 5);
    // direct call
    test4(1);
    test4(2, "aaa");
    test4(3, "bbb", 3);
    // apply
    this_object()->test4(1);
    this_object()->test4(2, "aaa");
    this_object()->test4(3, "bbb", 3);
}
  )";
  auto* prog = compile_file(source, "test");

  ASSERT_NE(prog, nullptr);
  dump_prog(prog, stdout, 1 | 2);
  deallocate_program(prog);
}

TEST_F(DriverTest, TestLPC_FunctionInherit) {
  // Load the inherited object first
  error_context_t econ{};
  save_context(&econ);
  try {
    auto obj = find_object("/single/tests/compiler/function");
    ASSERT_NE(obj, nullptr);

    auto obj2 = find_object("/single/tests/compiler/function_inherit");
    ASSERT_NE(obj2, nullptr);

    auto obj3 = find_object("/single/tests/compiler/function_inherit_2");
    ASSERT_NE(obj3, nullptr);

    dump_prog(obj3->prog, stdout, 1 | 2);
  } catch (...) {
    restore_context(&econ);
    FAIL();
  }
  pop_context(&econ);
}

// move_object() lazily calls try_reset() on the destination just before
// linking the moved item into it. reset() is arbitrary LPC and can
// self-destruct the destination as a perfectly ordinary side effect (no
// error() involved, so safe_apply() inside try_reset() doesn't catch it).
// move_object() must recheck O_DESTRUCTED afterward instead of linking the
// item into (or out of) an object that's no longer live.
TEST_F(DriverTest, TestMoveObjectDestructDuringReset) {
  auto saved_lazy_resets = CONFIG_INT(__RC_LAZY_RESETS__);
  auto saved_no_resets = CONFIG_INT(__RC_NO_RESETS__);
  auto saved_gametick = g_current_gametick;
  CONFIG_INT(__RC_LAZY_RESETS__) = 1;
  CONFIG_INT(__RC_NO_RESETS__) = 0;
  // try_reset()'s "is a reset due" check is next_reset < g_current_gametick;
  // the test harness never pumps the backend loop, so make sure the clock
  // side of that comparison is unambiguously past whatever next_reset ends
  // up being.
  g_current_gametick += 1000000;

  current_object = master_ob;
  object_t* dest = nullptr;
  object_t* item = nullptr;
  bool errored = false;

  error_context_t econ{};
  save_context(&econ);
  try {
    dest = find_object("/clone/move_object_reset_dest");
    item = find_object("/clone/move_object_item");
    ASSERT_NE(dest, nullptr);
    ASSERT_NE(item, nullptr);
    // Loading an object already schedules its first reset in the future
    // (call_create() -> set_nextreset()); force it due now so
    // try_reset() actually fires inside move_object() below.
    dest->next_reset = 0;
    dest->flags &= ~O_RESET_STATE;
    move_object(item, dest);
  } catch (...) {
    errored = true;
    restore_context(&econ);
  }
  if (!errored) {
    pop_context(&econ);
  }

  CONFIG_INT(__RC_LAZY_RESETS__) = saved_lazy_resets;
  CONFIG_INT(__RC_NO_RESETS__) = saved_no_resets;
  g_current_gametick = saved_gametick;

  // dest self-destructed out of reset(); move_object() must have errored
  // out instead of linking item into it.
  EXPECT_TRUE(errored);
  ASSERT_NE(dest, nullptr);
  EXPECT_TRUE(dest->flags & O_DESTRUCTED);
  ASSERT_NE(item, nullptr);
  EXPECT_EQ(item->super, nullptr);
}

// issue #968: with "reversible explode string" semantics, a string made
// entirely of delimiters must still split into n+1 empty fields so that
// implode(explode(s, d), d) == s.
TEST_F(DriverTest, ExplodeReversibleAllDelimiters) {
  array_t* v = explode_string("a", 1, "a", 1, true);
  ASSERT_EQ(v->size, 2);
  EXPECT_STREQ(v->item[0].u.string, "");
  EXPECT_STREQ(v->item[1].u.string, "");
  char* joined = implode_string(v, "a", 1);
  EXPECT_STREQ(joined, "a");
  FREE_MSTR(joined);
  free_array(v);

  v = explode_string("abab", 4, "ab", 2, true);
  ASSERT_EQ(v->size, 3);
  joined = implode_string(v, "ab", 2);
  EXPECT_STREQ(joined, "abab");
  FREE_MSTR(joined);
  free_array(v);

  // Non-reversible behavior is unchanged: no fields at all. (The result
  // is the static the_null_array; nothing to free.)
  v = explode_string("a", 1, "a", 1, false);
  EXPECT_EQ(v->size, 0);
}

// issue #1366: explode() became superlinear in token count because
// EGCIterator::reset() rescanned the remaining string for ASCII on every
// delimiter. Pin the many-token ASCII split; the testsuite's
// `maximum array size` is 15000, so stay under that.
TEST_F(DriverTest, ExplodeManyAsciiTokens) {
  constexpr int kTokens = 8000;
  std::string s;
  s.reserve(kTokens * 11);
  for (int i = 0; i < kTokens; i++) {
    if (i) s.push_back(' ');
    s += "abcdefghij";
  }
  array_t* v = explode_string(s.c_str(), static_cast<int>(s.size()), " ", 1, false);
  ASSERT_EQ(v->size, kTokens);
  EXPECT_STREQ(v->item[0].u.string, "abcdefghij");
  EXPECT_STREQ(v->item[kTokens - 1].u.string, "abcdefghij");
  free_array(v);
}

// Regression test for a heap-use-after-free in dealloc_object()
// (src/vm/internal/base/object.cc): destruct_object() pushes the object
// onto the global obj_list_destruct queue (simulate.cc); on the unfixed
// binary, dealloc_object() never unlinked the object from that queue when
// its ref count hit 0, so a same-call-sequence destruct+free of object A
// left obj_list_destruct pointing at freed memory. The very next
// destruct_object() call anywhere -- here, on an unrelated object B --
// then wrote through that dangling head pointer. Fails under ASan on the
// unfixed binary; on a plain build it would silently corrupt whatever
// memory A's address gets reused for.
TEST_F(DriverTest, DestructThenImmediateFreeDoesNotDangleObjListDestruct) {
  object_t* a = nullptr;
  object_t* b = nullptr;
  RunGuarded([&] { a = load_object_from_source("void bump() {}\n", "lifecycle_head_a", 0); });
  RunGuarded([&] { b = load_object_from_source("void bump() {}\n", "lifecycle_head_b", 0); });
  ASSERT_NE(a, nullptr);
  ASSERT_NE(b, nullptr);

  RunGuarded([&] {
    destruct_object(a);
    free_object(&a, "DestructThenImmediateFreeDoesNotDangleObjListDestruct");
  });
  RunGuarded([&] {
    destruct_object(b);
    free_object(&b, "DestructThenImmediateFreeDoesNotDangleObjListDestruct");
  });
}

// Regression test for the general (not just head) case of the same bug:
// destructing A, B, C in order chains obj_list_destruct as C -> B -> A.
// Freeing the MIDDLE object (B) directly -- exactly what reclaim_objects()
// does when it finds a stray reference to an already-destructed object --
// must correctly relink C's neighbor pointer to skip the freed B, or a
// later destruct_object() call (which touches the current head's
// neighbor pointers) dereferences freed memory.
TEST_F(DriverTest, MidChainFreeKeepsObjListDestructWalkable) {
  object_t* a = nullptr;
  object_t* b = nullptr;
  object_t* c = nullptr;
  object_t* d = nullptr;
  RunGuarded([&] { a = load_object_from_source("void bump() {}\n", "lifecycle_mid_a", 0); });
  RunGuarded([&] { b = load_object_from_source("void bump() {}\n", "lifecycle_mid_b", 0); });
  RunGuarded([&] { c = load_object_from_source("void bump() {}\n", "lifecycle_mid_c", 0); });
  ASSERT_NE(a, nullptr);
  ASSERT_NE(b, nullptr);
  ASSERT_NE(c, nullptr);

  RunGuarded([&] { destruct_object(a); });
  RunGuarded([&] { destruct_object(b); });
  RunGuarded([&] { destruct_object(c); });

  // Free the middle object directly, simulating reclaim_objects() dropping
  // a stray reference to a destructed object mid-queue.
  RunGuarded([&] { free_object(&b, "MidChainFreeKeepsObjListDestructWalkable"); });

  // Destructing (and freeing) a 4th object exercises the current
  // obj_list_destruct head's neighbor pointers -- on the unfixed binary
  // this is where the stale link left by the mid-chain free above would
  // be dereferenced.
  RunGuarded([&] { d = load_object_from_source("void bump() {}\n", "lifecycle_mid_d", 0); });
  ASSERT_NE(d, nullptr);
  RunGuarded([&] {
    destruct_object(d);
    free_object(&d, "MidChainFreeKeepsObjListDestructWalkable");
  });

  RunGuarded([&] { free_object(&c, "MidChainFreeKeepsObjListDestructWalkable"); });
  RunGuarded([&] { free_object(&a, "MidChainFreeKeepsObjListDestructWalkable"); });
}

// Regression test for an object-ref over-decrement in the destruct sweep:
// the obj_list_destruct queue used to ride on next_all/prev_all, which DEBUG
// builds immediately reuse for the obj_list_dangling leak-hunting list.
// Once a sweep left a still-referenced survivor behind (its next_all now
// encoding the dangling chain), the NEXT sweep's next_all walk strayed from
// the fresh queue into the dangling chain and ran destruct2() -- and its
// free_object() -- on the survivor a second time, deallocating an object
// other holders still referenced (issue #1327's corruption class). The
// queue now rides its own next_destruct link.
TEST_F(DriverTest, SweepDoesNotRevisitSurvivorsOfPreviousSweep) {
  object_t* a = nullptr;
  object_t* b = nullptr;
  RunGuarded([&] { a = load_object_from_source("void bump() {}\n", "lifecycle_sweep_a", 0); });
  RunGuarded([&] { b = load_object_from_source("void bump() {}\n", "lifecycle_sweep_b", 0); });
  ASSERT_NE(a, nullptr);
  ASSERT_NE(b, nullptr);

  // Simulate an LPC variable still referencing a across its destruct.
  add_ref(a, "SweepDoesNotRevisitSurvivorsOfPreviousSweep");

  RunGuarded([&] { destruct_object(a); });
  RunGuarded([&] { remove_destructed_objects(); });  // a survives: we hold a ref

  RunGuarded([&] { destruct_object(b); });
  RunGuarded([&] { remove_destructed_objects(); });  // must not destruct2(a) again

  // Releasing our reference must be the deallocating drop; on the unfixed
  // binary the second sweep already freed a and this is a use-after-free
  // (caught by ASan) / double dealloc.
  RunGuarded([&] { free_object(&a, "SweepDoesNotRevisitSurvivorsOfPreviousSweep"); });
}

// Companion GTest for the "intended use" replace_program() demo
// (testsuite/clone/replace_program/demo_poly_*.lpc) -- the
// legitimate runtime-polymorphism counterpart to the redundant
// self-target no-op bug fixed elsewhere in replace_program.cc. See the
// shell fixture's header comment for the full pattern writeup.
//
// /single/tests/efuns/replace_program_polymorphism_demo.lpc (the LPC-level
// companion) can only observe that a swap got PENDING: replace_programs()
// only runs off the backend's gametick queue (vm.cc's
// remove_destructed_objects(), reached via add_gametick_event() /
// call_remove_destructed_objects() in backend.cc), and the LPC test
// harness's single top-level -ftest run never yields back to the real
// event loop mid-suite, so no gametick ever elapses between two test
// files. This GTest uses the same technique this file already
// establishes for the sibling obj_list_destruct sweep (see
// SweepDoesNotRevisitSurvivorsOfPreviousSweep above): call
// remove_destructed_objects() directly to force replace_programs() to run
// synchronously, so we can prove the shell ACTUALLY becomes its chosen
// concrete class -- not merely that a swap got queued.
//
// Coverage note: relying solely on the shell's real random() path would
// make branch coverage probabilistic -- each of goblin/wolf/ghost only
// ~1/3 likely per run, and wolf/ghost specifically (the two species whose
// var_offset within the shell is nonzero, see
// demo_poly_shell.lpc's inherit order) hit only
// ~2/3 of runs combined, so a var_offset regression could pass a given CI
// run roughly 1/3 of the time. This test keeps ONE random-path clone (to
// keep exercising that real spawn-table randomness genuinely works) and
// adds THREE deterministic clones via create()'s forced_species argument
// (one per species), so all three -- including both nonzero-var_offset
// ones -- land unconditionally on every run.
TEST_F(DriverTest, ReplaceProgramPolymorphicShellDemoLandsForReal) {
  object_t* ob = nullptr;
  // find_object() loads (and runs create() on) the shell's blueprint.
  // With no forced_species argument, create() exercises the real runtime
  // decision via random() among the 3 concrete candidate types -- which
  // one it lands on doesn't matter below, only that it lands on exactly
  // one of them, consistently, before vs. after the swap.
  RunGuarded(
      [&] { ob = find_object("/clone/replace_program/demo_poly_shell"); });
  ASSERT_NE(ob, nullptr);

  // Pre-sweep: this is still, literally, the shell's own program -- which
  // already contains all 3 candidates' code (replace_program() can only
  // target something already inherited, see the shell fixture's header),
  // so all 3 markers resolve...
  EXPECT_NE(safe_apply("is_goblin", ob, 0, ORIGIN_DRIVER), nullptr);
  EXPECT_NE(safe_apply("is_wolf", ob, 0, ORIGIN_DRIVER), nullptr);
  EXPECT_NE(safe_apply("is_ghost", ob, 0, ORIGIN_DRIVER), nullptr);
  // ...and so does the shell's own (about-to-be-shed) machinery.
  svalue_t* shell_marker = safe_apply("shell_only_marker", ob, 0, ORIGIN_DRIVER);
  ASSERT_NE(shell_marker, nullptr);
  ASSERT_EQ(shell_marker->type, T_STRING);
  EXPECT_STREQ(shell_marker->u.string, "shell");

  // Deterministic clones: one per species, via create()'s forced_species
  // argument (demo_poly_shell.lpc's create(string
  // forced_species)). clone_object() (vm/internal/simulate.h) takes its
  // create() arguments off the top of the LPC stack -- push the species
  // string, then pass a count of 1 -- and, unlike find_object(), asserts a
  // live current_object (DEBUG_CHECK in simulate.cc), so set one first,
  // matching the convention other tests in this file use (e.g.
  // TestCompileDumpProgWorks above).
  object_t* goblin_ob = nullptr;
  object_t* wolf_ob = nullptr;
  object_t* ghost_ob = nullptr;
  RunGuarded([&] {
    current_object = master_ob;
    push_constant_string("goblin");
    goblin_ob = clone_object("/clone/replace_program/demo_poly_shell", 1);
  });
  RunGuarded([&] {
    current_object = master_ob;
    push_constant_string("wolf");
    wolf_ob = clone_object("/clone/replace_program/demo_poly_shell", 1);
  });
  RunGuarded([&] {
    current_object = master_ob;
    push_constant_string("ghost");
    ghost_ob = clone_object("/clone/replace_program/demo_poly_shell", 1);
  });
  ASSERT_NE(goblin_ob, nullptr);
  ASSERT_NE(wolf_ob, nullptr);
  ASSERT_NE(ghost_ob, nullptr);

  // Force the deferred sweep that an ordinary run only ever reaches via
  // the backend's tick queue. replace_programs() (replace_program.cc)
  // walks the ENTIRE obj_list_replace queue unconditionally, so one call
  // lands all 4 pending objects (the random one plus the 3 forced ones)
  // together.
  RunGuarded([&] { remove_destructed_objects(); });

  // Post-sweep: the shell has genuinely become exactly ONE concrete
  // class -- exactly one of the 3 markers still resolves...
  bool goblin = safe_apply("is_goblin", ob, 0, ORIGIN_DRIVER) != nullptr;
  bool wolf = safe_apply("is_wolf", ob, 0, ORIGIN_DRIVER) != nullptr;
  bool ghost = safe_apply("is_ghost", ob, 0, ORIGIN_DRIVER) != nullptr;
  EXPECT_EQ(goblin + wolf + ghost, 1)
      << "expected exactly one candidate to survive the swap; got goblin="
      << goblin << " wolf=" << wolf << " ghost=" << ghost;

  // ...and attack_damage()/query_hp() agree with whichever one it is --
  // proving the swap didn't just add the chosen candidate's marker
  // alongside the others, it made this object literally BE that
  // candidate's own compiled program (var_offset-correct and all).
  //
  // apply()/safe_apply() both return a pointer into ONE shared static
  // (apply_ret_value, see vm/internal/apply.cc) -- it must be consumed
  // (copied out) before the next apply() call, or a second call silently
  // overwrites what the first call's pointer points to.
  svalue_t* dmg_sv = safe_apply("attack_damage", ob, 0, ORIGIN_DRIVER);
  ASSERT_NE(dmg_sv, nullptr);
  ASSERT_EQ(dmg_sv->type, T_NUMBER);
  LPC_INT dmg = dmg_sv->u.number;

  svalue_t* hp_sv = safe_apply("query_hp", ob, 0, ORIGIN_DRIVER);
  ASSERT_NE(hp_sv, nullptr);
  ASSERT_EQ(hp_sv->type, T_NUMBER);
  LPC_INT hp = hp_sv->u.number;

  if (goblin) {
    EXPECT_EQ(dmg, 3);
    EXPECT_EQ(hp, 8);
  } else if (wolf) {
    EXPECT_EQ(dmg, 5);
    EXPECT_EQ(hp, 12);
  } else if (ghost) {
    EXPECT_EQ(dmg, 2);
    EXPECT_EQ(hp, 6);
  }

  // The shell's own machinery, and the other 2 candidates', are
  // genuinely GONE -- shed for real, not merely shadowed by an override
  // -- proving this was a real structural transformation, matching what
  // replace_programs()'s variable-trimming code actually does, not just
  // an additive change.
  EXPECT_EQ(safe_apply("shell_only_marker", ob, 0, ORIGIN_DRIVER), nullptr);

  // Now the deterministic part: each forced clone must have landed on
  // EXACTLY its forced species, not merely "some" species -- this is what
  // actually pins var_offset correctness for wolf/ghost on every run.
  struct Expected {
    object_t* obj;
    const char* label;
    bool want_goblin, want_wolf, want_ghost;
    LPC_INT want_dmg, want_hp;
  };
  Expected cases[] = {
      {goblin_ob, "goblin", true, false, false, 3, 8},
      {wolf_ob, "wolf", false, true, false, 5, 12},
      {ghost_ob, "ghost", false, false, true, 2, 6},
  };
  for (auto& c : cases) {
    bool c_goblin = safe_apply("is_goblin", c.obj, 0, ORIGIN_DRIVER) != nullptr;
    bool c_wolf = safe_apply("is_wolf", c.obj, 0, ORIGIN_DRIVER) != nullptr;
    bool c_ghost = safe_apply("is_ghost", c.obj, 0, ORIGIN_DRIVER) != nullptr;
    EXPECT_EQ(c_goblin, c.want_goblin) << "forced_species=" << c.label;
    EXPECT_EQ(c_wolf, c.want_wolf) << "forced_species=" << c.label;
    EXPECT_EQ(c_ghost, c.want_ghost) << "forced_species=" << c.label;

    svalue_t* c_dmg_sv = safe_apply("attack_damage", c.obj, 0, ORIGIN_DRIVER);
    ASSERT_NE(c_dmg_sv, nullptr) << "forced_species=" << c.label;
    ASSERT_EQ(c_dmg_sv->type, T_NUMBER);
    EXPECT_EQ(c_dmg_sv->u.number, c.want_dmg) << "forced_species=" << c.label;

    svalue_t* c_hp_sv = safe_apply("query_hp", c.obj, 0, ORIGIN_DRIVER);
    ASSERT_NE(c_hp_sv, nullptr) << "forced_species=" << c.label;
    ASSERT_EQ(c_hp_sv->type, T_NUMBER);
    EXPECT_EQ(c_hp_sv->u.number, c.want_hp) << "forced_species=" << c.label;

    EXPECT_EQ(safe_apply("shell_only_marker", c.obj, 0, ORIGIN_DRIVER), nullptr)
        << "forced_species=" << c.label;
  }
}

// Regression GTest for Bug 1 and Bug 2 (see
// replace_program_bug1_last_call_wins.lpc / replace_program_bug2_private_flags.lpc
// and the /clone/replace_program/replace_bug1_*/replace_bug2_* fixtures for the full
// scenario writeups). Both bugs are fixes to the SAME no-op fast path in
// f_replace_program() (src/packages/core/replace_program.cc), both are
// only observable by forcing the deferred sweep synchronously (see
// ReplaceProgramPolymorphicShellDemoLandsForReal above for why an
// LPC-level test alone can't observe this), and both are otherwise
// mechanically identical: clone a fixture, assert its pre-swap state,
// force ONE shared remove_destructed_objects() sweep (it walks the
// entire obj_list_replace queue unconditionally, so a single call lands
// every pending entry from both scenarios together -- exactly what a
// real backend tick does when multiple objects have pending swaps), then
// assert each fixture's post-swap state. Kept as two clearly-labeled
// SCOPED_TRACE blocks rather than two full test functions: a failure in
// either block still names exactly which scenario broke.
//
// Bug 1: the no-op fast path used to return early WITHOUT consulting an
// already-pending entry, so a genuine first call followed by a no-op
// second call left the FIRST call's stale target pending instead of the
// second (last) call's target -- violating
// retrieve_replace_program_entry()'s "last call wins" contract, which
// every OTHER repeated-replace_program()-call case in the driver honors.
//
// Bug 2: replace_program_is_noop() used to compare only resolved
// function_t* IDENTITY between cur_prog and new_prog, missing that the
// SAME function can carry genuinely different, behaviorally-relevant
// ACCESS flags depending on which program's own function_flags[] you
// read it through -- copy_new_function() (compiler.cc) demotes 'private'
// to 'hidden' when a function is copied down an inherit edge, purely as
// a side effect of being inherited, with no change to the underlying
// bytecode/function_t. Bug 2's block proves this is a REAL, observable
// behavioral difference (not just a theoretical flag mismatch) by
// exercising apply.cc's actual permission check via a self-call-other,
// both pre- and post-swap.
// Debug-only: stack_in_use_as_temporary and break_point() both exist only
// under DEBUG (interpret.cc), so there is nothing to check in a release
// build -- and nothing to link against either.
#ifdef DEBUG
TEST_F(DriverTest, ForeachTemporariesRestoredOnUnwind) {
  // foreach bumps stack_in_use_as_temporary so break_point() knows the
  // temporaries sitting above fp are legitimate, and F_EXIT_FOREACH retires
  // them. An error thrown out of the loop never reaches that opcode, so only
  // the unwind can put the counter back -- and error_context_t does not carry
  // it. Before this was fixed, one catch() of an error inside a foreach left
  // the count at 1 for the life of the process, and break_point() only checks
  // the stack when the count is ZERO: the check silently stopped running for
  // all later LPC. Nothing in the LPC suite can observe that, hence a unit
  // test reading the counter directly.
  extern int stack_in_use_as_temporary;
  error_context_t econ{};
  save_context(&econ);
  try {
    auto* ob = find_object("/clone/foreach_unwind");
    ASSERT_NE(ob, nullptr);
    current_object = master_ob;

    int const before = stack_in_use_as_temporary;
    apply("caught_in_foreach", ob, 0, ORIGIN_DRIVER);
    EXPECT_EQ(before, stack_in_use_as_temporary)
        << "catch() of an error inside a foreach leaked temporaries";

    apply("caught_in_nested_foreach", ob, 0, ORIGIN_DRIVER);
    EXPECT_EQ(before, stack_in_use_as_temporary)
        << "the same, unwinding through two open loops";
    pop_context(&econ);
  } catch (const char* e) {
    restore_context(&econ);
    pop_context(&econ);
    FAIL() << "unexpected error: " << e;
  }
}
#endif  // DEBUG

TEST_F(DriverTest, MissingEfunTableNamesOnlyAbsentPackages) {
  // packages_missing_efuns.autogen.h lists the efuns of packages this build
  // does NOT have, so "Undefined function hash" can say which package `hash`
  // would have come from (#1352). The note is only trustworthy if a hit means
  // exactly "not compiled in" -- an entry naming an efun this driver DOES
  // have would attach it to an ordinary typo'd call instead. The generator
  // scans only disabled packages' spec files; this pins that it stays so.
  //
  // A build with every package enabled leaves the table empty, which is a
  // valid outcome and not a reason to fail.
  for (const auto& entry : missing_efuns) {
    if (entry.name == nullptr) {
      break;  // sentinel; an all-packages-enabled build has only this
    }
    ASSERT_NE(entry.package, nullptr);
    EXPECT_STRNE(entry.name, "") << "an entry with no efun name";
    EXPECT_EQ(0, strncmp(entry.package, "PACKAGE_", 8))
        << "package should be a PACKAGE_* option, got: " << entry.package;

    ident_hash_elem_t* ihe = lookup_ident(entry.name);
    EXPECT_FALSE(ihe != nullptr && (ihe->token & IHE_EFUN))
        << entry.name << " is listed as needing " << entry.package
        << " but this driver HAS it as an efun -- the note would be wrong";
  }
}

TEST_F(DriverTest, ReplaceProgramLandedSwapSemantics) {
  // --- Bug 1 setup: last-call-wins -------------------------------------
  object_t* bug1_ob = nullptr;
  // find_object() loads (and runs create() on) the shell's blueprint --
  // same technique as the polymorphism demo test above.
  RunGuarded([&] { bug1_ob = find_object("/clone/replace_program/replace_bug1_shell"); });
  ASSERT_NE(bug1_ob, nullptr);
  {
    SCOPED_TRACE("bug1: last-call-wins after a no-op second call");
    // Pre-sweep: still literally the shell's own program, which already
    // contains both Y's and BASE's content (replace_program() can only
    // target something already inherited). Note base_marker() is present
    // both pre-sweep AND if the object ends up landing on BASE -- it
    // does NOT discriminate "never swapped" from "correctly swapped to
    // BASE", by design (the shell is a bare passthrough of BASE, so they
    // are functionally identical). Neither does y_marker(): BASE itself
    // inherits Y, so y_marker() stays resolvable on EVERY possible
    // outcome here (unswapped shell, landed on Y, or landed on BASE) --
    // it cannot discriminate anything either. The only way to tell
    // exactly which program the object structurally ended up with is to
    // check ob->prog->filename directly, below.
    EXPECT_NE(safe_apply("y_marker", bug1_ob, 0, ORIGIN_DRIVER), nullptr);
    EXPECT_NE(safe_apply("base_marker", bug1_ob, 0, ORIGIN_DRIVER), nullptr);
    ASSERT_NE(bug1_ob->prog, nullptr);
    EXPECT_STREQ(bug1_ob->prog->filename, "clone/replace_program/replace_bug1_shell.lpc");
  }

  // --- Bug 2 setup: private access flags survive the swap --------------
  object_t* bug2_ob = nullptr;
  RunGuarded([&] { bug2_ob = find_object("/clone/replace_program/replace_bug2_wrapper"); });
  ASSERT_NE(bug2_ob, nullptr);
  {
    SCOPED_TRACE("bug2: private-flags-affect-self-call-permission, pre-swap");
    // Pre-sweep: ob->prog is still literally the wrapper's own compiled
    // program. Its own function_flags entry for secret() was demoted
    // from the base's DECL_PRIVATE to DECL_HIDDEN by copy_new_function()
    // purely because it got copied down an inherit edge -- apply()'s
    // permission check requires DECL_PRIVATE for a self-call-other, and
    // DECL_HIDDEN doesn't meet that bar, so this must be REJECTED
    // (evaluates to 0), not succeed.
    svalue_t* pre = safe_apply("try_call_secret_via_self", bug2_ob, 0, ORIGIN_DRIVER);
    ASSERT_NE(pre, nullptr);
    ASSERT_EQ(pre->type, T_NUMBER);
    EXPECT_EQ(pre->u.number, 0)
        << "expected the self-call to be rejected pre-swap (the wrapper's "
           "own copy of secret() is DECL_HIDDEN, not DECL_PRIVATE)";
  }

  // Force the deferred sweep that an ordinary run only ever reaches via
  // the backend's tick queue -- ONE call lands both bug1_ob's and
  // bug2_ob's pending entries together.
  RunGuarded([&] { remove_destructed_objects(); });

  {
    SCOPED_TRACE("bug1: last-call-wins after a no-op second call");
    // Post-sweep: last-call-wins means the object must have
    // STRUCTURALLY become BASE's own compiled program (the second/last
    // call's target) -- not Y's (the stale first call's target, what the
    // unfixed driver incorrectly produced), and not still the shell's
    // own (i.e. not "no swap happened at all", which a wrongly-taken
    // no-op fast path on BOTH calls could also produce).
    ASSERT_NE(bug1_ob->prog, nullptr);
    EXPECT_STREQ(bug1_ob->prog->filename, "clone/replace_program/replace_bug1_base.lpc")
        << "expected the object to have structurally become BASE (the "
           "second/last call's target); landing on Y or staying as the "
           "shell both indicate the no-op second call was mishandled";
  }

  {
    SCOPED_TRACE("bug2: private-flags-affect-self-call-permission, post-swap");
    // Post-sweep: Bug 2's fix correctly refused the no-op fast path (the
    // DECL_PRIVATE vs. DECL_HIDDEN mismatch on secret() was detected),
    // so a real pending swap got registered and landed -- ob->prog is
    // genuinely the base program now, whose own function_flags entry for
    // secret() is DECL_PRIVATE, which DOES meet apply()'s self-call bar.
    ASSERT_NE(bug2_ob->prog, nullptr);
    EXPECT_STREQ(bug2_ob->prog->filename, "clone/replace_program/replace_bug2_private_base.lpc");
    svalue_t* post = safe_apply("try_call_secret_via_self", bug2_ob, 0, ORIGIN_DRIVER);
    ASSERT_NE(post, nullptr);
    ASSERT_EQ(post->type, T_NUMBER);
    EXPECT_EQ(post->u.number, 42)
        << "expected the self-call to succeed post-swap (the base's own "
           "secret() is genuinely DECL_PRIVATE)";
  }
}

// ---------------------------------------------------------------------------
// net_dead teardown stress tests (issue #1327).
//
// remove_interactive(ob, 0) runs the net_dead() apply -- arbitrary LPC --
// and then tears the connection down. The classic linkdead idiom has
// net_dead() call exec() to re-home the connection into a fresh "ghost"
// body; exec() releases ob's interactive reference and moves both ip->ob
// and the counted ref to the new body. The teardown used to keep operating
// on the ORIGINAL ob: it freed the ip the new body still pointed at, nulled
// the wrong object's ->interactive, and decremented ob's refcount a second
// time -- draining a live object toward ref 0 (the "ref count 0, but not
// destructed" fatal). These tests fabricate a minimal transport-less
// interactive (all the teardown's transport/telnet/translator branches are
// null-guarded) and hammer every net_dead shape, asserting exact refcount
// deltas each iteration.
// ---------------------------------------------------------------------------

namespace {

interactive_t* fabricate_interactive(object_t* ob) {
  interactive_t* ip = user_add();  // zeroed; transport/telnet/trans all null
  ip->ob = ob;
  ip->prompt = "> ";
  ob->interactive = ip;
  ob->flags |= O_ONCE_INTERACTIVE;
  add_ref(ob, "fabricate_interactive");  // the connection's ref, as new_user takes
  return ip;
}

// Reads an object-typed global out of a (possibly destructed, not yet swept)
// object's variable block without running LPC.
object_t* read_object_global(object_t* ob, int index) {
  if (!ob->prog || index >= ob->prog->num_variables_total) {
    return nullptr;
  }
  svalue_t* v = &ob->variables[index];
  return v->type == T_OBJECT ? v->u.ob : nullptr;
}

}  // namespace

// net_dead() exec()s the connection into a ghost: the teardown must release
// the ghost's interactive state, not the old body's.
TEST_F(DriverTest, NetDeadExecGhostStress) {
  const char* src =
      "object ghost;\n"
      "void net_dead() { ghost = new(\"/clone/testob1\"); exec(ghost, this_object()); }\n";

  for (int i = 0; i < 150; i++) {
    object_t* body = nullptr;
    std::string name = "netdead_exec_" + std::to_string(i);
    RunGuarded([&] { body = load_object_from_source(src, name.c_str(), 0); });
    ASSERT_NE(body, nullptr);

    uint32_t const ref_before = body->ref;
    fabricate_interactive(body);
    ASSERT_EQ(body->ref, ref_before + 1);

    RunGuarded([&] { remove_interactive(body, 0); });

    // exec() released the interactive ref net_dead's connection held on
    // body; the teardown must have released the GHOST's, not body's again.
    EXPECT_EQ(body->interactive, nullptr);
    EXPECT_FALSE(body->flags & O_DESTRUCTED);
    EXPECT_EQ(body->ref, ref_before) << "iteration " << i;
    EXPECT_TRUE(users().empty());

    object_t* ghost = read_object_global(body, 0);
    ASSERT_NE(ghost, nullptr);
    EXPECT_FALSE(ghost->flags & O_DESTRUCTED);
    // The unfixed teardown left ghost->interactive pointing at the freed ip.
    EXPECT_EQ(ghost->interactive, nullptr) << "iteration " << i;
    EXPECT_TRUE(ghost->flags & O_ONCE_INTERACTIVE);

    RunGuarded([&] { destruct_object(ghost); });
    RunGuarded([&] {
      destruct_object(body);
      free_object(&body, "NetDeadExecGhostStress");
    });
    if (i % 16 == 0) {
      RunGuarded([&] { remove_destructed_objects(); });
    }
  }
  RunGuarded([&] { remove_destructed_objects(); });
}

// Baseline: net_dead() that leaves the connection alone. The teardown
// releases exactly the one interactive ref.
TEST_F(DriverTest, NetDeadPlainStress) {
  const char* src = "int dead;\nvoid net_dead() { dead++; }\n";

  for (int i = 0; i < 100; i++) {
    object_t* body = nullptr;
    std::string name = "netdead_plain_" + std::to_string(i);
    RunGuarded([&] { body = load_object_from_source(src, name.c_str(), 0); });
    ASSERT_NE(body, nullptr);

    uint32_t const ref_before = body->ref;
    fabricate_interactive(body);
    RunGuarded([&] { remove_interactive(body, 0); });

    EXPECT_EQ(body->interactive, nullptr);
    EXPECT_EQ(body->ref, ref_before) << "iteration " << i;
    EXPECT_TRUE(users().empty());
    // net_dead must actually have run.
    EXPECT_EQ(body->variables[0].type, T_NUMBER);
    EXPECT_EQ(body->variables[0].u.number, 1);

    RunGuarded([&] {
      destruct_object(body);
      free_object(&body, "NetDeadPlainStress");
    });
    if (i % 16 == 0) {
      RunGuarded([&] { remove_destructed_objects(); });
    }
  }
  RunGuarded([&] { remove_destructed_objects(); });
}

// net_dead() destructs the body itself: the recursive remove_interactive()
// from destruct_object() must be absorbed by the CLOSING guard, and the
// outer teardown still releases exactly one interactive ref.
TEST_F(DriverTest, NetDeadSelfDestructStress) {
  const char* src = "void net_dead() { destruct(this_object()); }\n";

  for (int i = 0; i < 100; i++) {
    object_t* body = nullptr;
    std::string name = "netdead_selfdestruct_" + std::to_string(i);
    RunGuarded([&] { body = load_object_from_source(src, name.c_str(), 0); });
    ASSERT_NE(body, nullptr);

    uint32_t const ref_before = body->ref;
    fabricate_interactive(body);
    RunGuarded([&] { remove_interactive(body, 0); });

    EXPECT_TRUE(body->flags & O_DESTRUCTED);
    EXPECT_EQ(body->interactive, nullptr);
    EXPECT_EQ(body->ref, ref_before) << "iteration " << i;
    EXPECT_TRUE(users().empty());

    RunGuarded([&] { free_object(&body, "NetDeadSelfDestructStress"); });
    if (i % 16 == 0) {
      RunGuarded([&] { remove_destructed_objects(); });
    }
  }
  RunGuarded([&] { remove_destructed_objects(); });
}

// net_dead() exec()s to a ghost AND destructs the old body: teardown must
// follow the connection to the ghost while the destruct path skips the
// (already cleared) old body's interactive.
TEST_F(DriverTest, NetDeadExecThenDestructStress) {
  const char* src =
      "object ghost;\n"
      "void net_dead() {\n"
      "  ghost = new(\"/clone/testob1\");\n"
      "  exec(ghost, this_object());\n"
      "  destruct(this_object());\n"
      "}\n";

  for (int i = 0; i < 100; i++) {
    object_t* body = nullptr;
    std::string name = "netdead_execdestruct_" + std::to_string(i);
    RunGuarded([&] { body = load_object_from_source(src, name.c_str(), 0); });
    ASSERT_NE(body, nullptr);

    fabricate_interactive(body);
    RunGuarded([&] { remove_interactive(body, 0); });

    EXPECT_TRUE(body->flags & O_DESTRUCTED);
    EXPECT_EQ(body->interactive, nullptr);
    EXPECT_TRUE(users().empty());

    // body is destructed but unswept, so its variable block still holds the
    // ghost; the teardown must have cleaned the ghost's connection state.
    object_t* ghost = read_object_global(body, 0);
    ASSERT_NE(ghost, nullptr);
    EXPECT_FALSE(ghost->flags & O_DESTRUCTED);
    EXPECT_EQ(ghost->interactive, nullptr) << "iteration " << i;

    RunGuarded([&] { destruct_object(ghost); });
    RunGuarded([&] { free_object(&body, "NetDeadExecThenDestructStress"); });
    if (i % 16 == 0) {
      RunGuarded([&] { remove_destructed_objects(); });
    }
  }
  RunGuarded([&] { remove_destructed_objects(); });
}

// The destruct-driven teardown (dested=1) never runs net_dead() and always
// operates on the original body.
TEST_F(DriverTest, RemoveInteractiveDestructedStress) {
  const char* src = "int dead;\nvoid net_dead() { dead++; }\n";

  for (int i = 0; i < 100; i++) {
    object_t* body = nullptr;
    std::string name = "netdead_dested_" + std::to_string(i);
    RunGuarded([&] { body = load_object_from_source(src, name.c_str(), 0); });
    ASSERT_NE(body, nullptr);

    uint32_t const ref_before = body->ref;
    fabricate_interactive(body);
    RunGuarded([&] { remove_interactive(body, 1); });

    EXPECT_EQ(body->interactive, nullptr);
    EXPECT_EQ(body->ref, ref_before) << "iteration " << i;
    EXPECT_TRUE(users().empty());
    // dested=1 must NOT run the net_dead apply.
    EXPECT_EQ(body->variables[0].u.number, 0);

    RunGuarded([&] {
      destruct_object(body);
      free_object(&body, "RemoveInteractiveDestructedStress");
    });
    if (i % 16 == 0) {
      RunGuarded([&] { remove_destructed_objects(); });
    }
  }
  RunGuarded([&] { remove_destructed_objects(); });
}
