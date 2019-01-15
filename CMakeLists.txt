cmake_minimum_required(VERSION 3.10)   # CMake version check

# Enable CMAKE Policy
if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(fluffos VERSION 2019.2019011401
        DESCRIPTION "FluffOS LPC Driver"
        LANGUAGES C CXX)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Default Build Type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

message("FluffOS ${PROJECT_VERSION} (${CMAKE_BUILD_TYPE})")
message("  C: '${CMAKE_C_COMPILER}'")
message("  C++: '${CMAKE_CXX_COMPILER}'")

add_subdirectory(src)
