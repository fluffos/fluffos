# the current build target is ubuntu 22.04 LTS, it ships with cmake 3.22
cmake_minimum_required(VERSION 3.22)

if (POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

project(fluffos
        DESCRIPTION "FluffOS LPC Driver"
        LANGUAGES C CXX)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Default Build Type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

# Export compile_commands.json for clangd / language servers (Makefile + Ninja only)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Target: symlink compile_commands.json from the build dir to the project root
# so that editors and tools find it without extra configuration.
if(NOT WIN32)
  add_custom_target(compile_commands
    COMMAND "${CMAKE_COMMAND}" -E create_symlink
            "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${CMAKE_SOURCE_DIR}/compile_commands.json"
    COMMENT "Symlinking compile_commands.json to project root"
    VERBATIM
  )
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "..." FORCE)
endif()

if(WIN32)
  # minimal support version is Windows 7
  add_compile_options("-DWINVER=0x0601")
  add_compile_options("-D_WIN32_WINNT=0x0601")
  add_compile_options("-D_UNICODE")
  add_compile_options("-DUNICODE")
endif()

#
# Try to find the current Git hash
#

find_program (BASH_EXECUTABLE bash REQUIRED)
if(NOT WIN32)
  find_package(Git)
else()
  execute_process(
          COMMAND ${BASH_EXECUTABLE} -c "git status"
          RESULT_VARIABLE STATUS
          OUTPUT_VARIABLE OUTPUT2
          ERROR_QUIET )
  if(STATUS AND NOT STATUS EQUAL 0)
  else()
    set(GIT_EXECUTABLE "git")
  endif()
endif()
if(GIT_EXECUTABLE)
  execute_process(
          WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
          COMMAND ${BASH_EXECUTABLE} -c "${GIT_EXECUTABLE} log --committer='Yucong Sun' -1 --date=format:%Y%m%d --format=%cd-%h"
          OUTPUT_VARIABLE GIT_TIMESTAMP
          OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(
          WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
          COMMAND ${BASH_EXECUTABLE} -c "if [[ $(${GIT_EXECUTABLE} log --committer='Yucong Sun' -1 --format=%h) != $( ${GIT_EXECUTABLE} log -1 --format=%h ) ]]; then echo -$(${GIT_EXECUTABLE} log -1 --format=%h); fi"
          OUTPUT_VARIABLE GIT_LOCAL_COMMIT
          OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(
          WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
          COMMAND ${BASH_EXECUTABLE} -c "${GIT_EXECUTABLE} diff-index --quiet HEAD -- || echo -uncommited"
          OUTPUT_VARIABLE GIT_DIRTY
          OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(PROJECT_VERSION "${GIT_TIMESTAMP}${GIT_LOCAL_COMMIT}${GIT_DIRTY}")
else()
  set(PROJECT_VERSION "unknown")
endif()

message("FluffOS ${PROJECT_VERSION} (${CMAKE_BUILD_TYPE})")
foreach(lang C CXX)
message("  ${lang}: '${CMAKE_${lang}_COMPILER}' (${CMAKE_${lang}_COMPILER_ID} ${CMAKE_${lang}_COMPILER_VERSION})")
endforeach()
message("Building on: '${CMAKE_HOST_SYSTEM}' for '${CMAKE_SYSTEM}'")
message("Installing to ${CMAKE_INSTALL_PREFIX}")

find_package(GTest)
if(${GTEST_FOUND})
  include(GoogleTest)
  enable_testing()
endif()

add_subdirectory(src)

if (CMAKE_CROSSCOMPILING)
  # No native driver binary to run the LPC testsuite with (the WASM
  # driver runs it in the page / under node instead).
  return()
endif ()

add_custom_target(driver-testsuite
        COMMAND $<TARGET_FILE:driver> etc/config.test
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/testsuite"
        COMMENT "Running driver with testsuite"
)

add_custom_target(driver-autotest
        COMMAND $<TARGET_FILE:driver> etc/config.test -ftest
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/testsuite"
        COMMENT "Running driver automated tests"
)

# The LPC testsuite as a first-class ctest test: `ctest -R testsuite`
# (equivalently `ctest -L testsuite`; plain `ctest` includes it) runs the
# whole mudlib suite alongside the GTest binaries. The runner prints gtest-style [ RUN ]/[ OK ]/[ FAILED ]
# blocks; "Checks succeeded." plus a clean exit is the pass signal, and
# any recorded failure prints a [  FAILED  ] recap and exits nonzero.
enable_testing()
add_test(NAME testsuite
        COMMAND $<TARGET_FILE:driver> etc/config.test -ftest
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/testsuite")
set_tests_properties(testsuite PROPERTIES
        PASS_REGULAR_EXPRESSION "Checks succeeded\\."
        FAIL_REGULAR_EXPRESSION "\\[  FAILED  \\]"
        TIMEOUT 600
        LABELS "testsuite")

# lpcshell drives the REPL binary itself in script mode against the same
# mudlib: the wrapping it is being tested for (deciding whether typed text
# is an expression or a statement, and reporting what happened) lives in
# C++ in main_lpcshell.cc, so the LPC testsuite above cannot reach it.
# `ctest -L lpcshell` runs just these.
#
# Both assert on OUTPUT rather than exit status -- lpcshell-errors evaluates
# deliberately-failing statements and so exits nonzero by design, and CMake
# ignores the exit code once PASS_REGULAR_EXPRESSION is set.
add_test(NAME lpcshell-eval
        COMMAND $<TARGET_FILE:lpcshell> etc/config.test lpcshell/eval.lpcs
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/testsuite")
set_tests_properties(lpcshell-eval PROPERTIES
        PASS_REGULAR_EXPRESSION "EVAL_OK"
        FAIL_REGULAR_EXPRESSION "EVAL_BAD;syntax error"
        TIMEOUT 120
        LABELS "lpcshell")

# The regression guard. A bare expression that error()s must report the
# real message and must NOT be retried in statement form: the retry would
# re-run whatever side effects the expression already had (asserted via
# n=1 -- n=2 means it ran twice) and report a bogus "syntax error,
# unexpected '}'" against the generated wrapper's closing brace, burying
# the real error entirely.
add_test(NAME lpcshell-errors
        COMMAND $<TARGET_FILE:lpcshell> etc/config.test lpcshell/errors.lpcs
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/testsuite")
set_tests_properties(lpcshell-errors PROPERTIES
        PASS_REGULAR_EXPRESSION "error: boom"
        FAIL_REGULAR_EXPRESSION "syntax error;n=0;n=2"
        TIMEOUT 120
        LABELS "lpcshell")
