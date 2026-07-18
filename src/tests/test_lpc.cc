#include <gtest/gtest.h>
#include "base/package_api.h"

#include "mainlib.h"

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
