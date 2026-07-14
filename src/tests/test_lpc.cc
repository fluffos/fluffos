#include <gtest/gtest.h>
#include "base/package_api.h"

#include "mainlib.h"

#include "compiler/internal/compiler.h"

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
