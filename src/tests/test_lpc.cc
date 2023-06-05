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

  error_context_t econ{};
  save_context(&econ);
  try {
    std::istringstream source("void test() {}");
    auto stream = std::make_unique<IStreamLexStream>(source);
    prog = compile_file(std::move(stream), "test");
  } catch (...) {
    restore_context(&econ);
    FAIL();
  }
  pop_context(&econ);

  ASSERT_NE(prog, nullptr);
  dump_prog(prog, stdout, 1 | 2);

  deallocate_program(prog);
}
