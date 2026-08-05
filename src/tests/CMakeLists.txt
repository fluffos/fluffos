# GTEST
find_package(GTest)

if(${GTEST_FOUND})
  include(GoogleTest)
  enable_testing()

  add_executable(lpc_tests test_lpc.cc)
  target_link_libraries(lpc_tests PRIVATE ${FLUFFOS_LINK} GTest::GTest  GTest::Main)
  target_compile_definitions(lpc_tests PRIVATE -DTESTSUITE_DIR="${CMAKE_SOURCE_DIR}/testsuite")

  add_executable(ofile_tests test_ofile.cc)
  target_link_libraries(ofile_tests PRIVATE ${FLUFFOS_LINK} GTest::GTest  GTest::Main)
  target_compile_definitions(ofile_tests PRIVATE -DTESTSUITE_DIR="${CMAKE_SOURCE_DIR}/testsuite")

  add_executable(compiler_tests test_compiler.cc)
  target_link_libraries(compiler_tests PRIVATE ${FLUFFOS_LINK} GTest::GTest GTest::Main)
  target_compile_definitions(compiler_tests PRIVATE -DTESTSUITE_DIR="${CMAKE_SOURCE_DIR}/testsuite")

  add_executable(lexer_tests test_lexer.cc)
  target_link_libraries(lexer_tests PRIVATE ${FLUFFOS_LINK} GTest::GTest GTest::Main)
  target_compile_definitions(lexer_tests PRIVATE -DTESTSUITE_DIR="${CMAKE_SOURCE_DIR}/testsuite")

  add_executable(strutils_tests test_strutils.cc)
  target_link_libraries(strutils_tests PRIVATE ${FLUFFOS_LINK} GTest::GTest GTest::Main)

  if(NOT EMSCRIPTEN)
    add_executable(debugger_tests test_debugger.cc)
    target_link_libraries(debugger_tests PRIVATE ${FLUFFOS_LINK} GTest::GTest GTest::Main)
    target_compile_definitions(debugger_tests PRIVATE -DTESTSUITE_DIR="${CMAKE_SOURCE_DIR}/testsuite")
  endif()

  # Scratchpad-vs-malloc micro-benchmark: proves the compile arena beats
  # heap allocation for the compiler's allocation patterns. Timing-based,
  # so it is NOT registered with ctest -- run manually (RelWithDebInfo):
  #   ./src/tests/bench_scratchpad [compiles-per-workload]
  add_executable(bench_scratchpad bench_scratchpad.cc)
  target_link_libraries(bench_scratchpad PRIVATE ${FLUFFOS_LINK})

  # Whole-compile throughput across many rounds (real compile_file on a
  # representative program; proves flat steady-state + zero chunk mallocs
  # after warmup). Manual, RelWithDebInfo:
  #   ./src/tests/bench_compile [rounds]
  add_executable(bench_compile bench_compile.cc)
  target_link_libraries(bench_compile PRIVATE ${FLUFFOS_LINK})
  target_compile_definitions(bench_compile PRIVATE -DTESTSUITE_DIR="${CMAKE_SOURCE_DIR}/testsuite")

  gtest_discover_tests(lpc_tests DISCOVERY_TIMEOUT 60)
  gtest_discover_tests(ofile_tests DISCOVERY_TIMEOUT 60)
  gtest_discover_tests(compiler_tests DISCOVERY_TIMEOUT 60)
  gtest_discover_tests(lexer_tests DISCOVERY_TIMEOUT 60)
  gtest_discover_tests(strutils_tests DISCOVERY_TIMEOUT 60)
  if(NOT EMSCRIPTEN)
    gtest_discover_tests(debugger_tests DISCOVERY_TIMEOUT 60)
  endif()
endif()

