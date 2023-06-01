# GTEST
find_package(GTest)

if(${GTEST_FOUND})
  include(GoogleTest)
  enable_testing()

  add_executable(lpc_tests test_lpc.cc)
  target_link_libraries(lpc_tests PRIVATE ${FLUFFOS_LINK} GTest::GTest  GTest::Main)
  target_compile_definitions(lpc_tests PRIVATE -DTESTSUITE_DIR="${CMAKE_SOURCE_DIR}/testsuite")
  gtest_discover_tests(lpc_tests)
endif()

