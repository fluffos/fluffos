cmake_minimum_required(VERSION 3.6)   # CMake version check
project(fluffos CXX)
set(CMAKE_CXX_STANDARD 11)            # Enable c++11 standard

file(GLOB_RECURSE SRC
        "*.h"
        "*.hh"
        "*.cc"
        )
include_directories(BEFORE ".")

add_executable(driver ${SRC})
