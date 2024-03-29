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
