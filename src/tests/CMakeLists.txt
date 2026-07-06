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

  gtest_discover_tests(lpc_tests DISCOVERY_TIMEOUT 60)
  gtest_discover_tests(ofile_tests DISCOVERY_TIMEOUT 60)
  gtest_discover_tests(compiler_tests DISCOVERY_TIMEOUT 60)
  gtest_discover_tests(lexer_tests DISCOVERY_TIMEOUT 60)
endif()

