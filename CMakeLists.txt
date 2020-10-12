# v2019 is targeted to ubuntu 18.04 LTS, it ships with cmake 3.10
cmake_minimum_required(VERSION 3.10)

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

find_package(Git)
if(GIT_EXECUTABLE)
  execute_process(
          WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
          COMMAND "${GIT_EXECUTABLE}" describe --tags --always
          OUTPUT_VARIABLE GIT_HASH
          OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(PROJECT_VERSION ${GIT_HASH})
  message("Git commit hash: ${PROJECT_VERSION}")
endif()

message("FluffOS ${PROJECT_VERSION} (${CMAKE_BUILD_TYPE})")
foreach(lang C CXX)
message("  ${lang}: '${CMAKE_${lang}_COMPILER}' (${CMAKE_${lang}_COMPILER_ID} ${CMAKE_${lang}_COMPILER_VERSION})")
endforeach()
message("Building on: '${CMAKE_HOST_SYSTEM}' for '${CMAKE_SYSTEM}'")
message("Installing to ${CMAKE_INSTALL_PREFIX}")

add_subdirectory(src)
