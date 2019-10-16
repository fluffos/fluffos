# List of compile options to tweak
option(STATIC "Build static version of driver" OFF)
option(USE_JEMALLOC "Build driver with jemalloc support" ON)
# End of list of compile options.

# Globally enforce CXX11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Setup include directoires
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

# include common compiler flags/definiations
include(common)

target_include_directories(common INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

# STATIC
if(STATIC)
  message(STATUS "Linking driver STATICLY.")
  if(WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
  else()
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
  endif()
endif()
# end of STATIC

# JEMALLOC
if (CYGWIN)
  set(USE_JEMALLOC OFF)
  message(STATUS "JEMALLOC is not supported on CYGWIN.")
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(USE_JEMALLOC OFF)
  message(STATUS "JEMALLOC is disabled on debug build.")
endif()
if (USE_JEMALLOC)
  find_package(jemalloc REQUIRED)
  set(HAVE_JEMALLOC TRUE) # used in config.h.in
  target_include_directories(common INTERFACE ${JEMALLOC_INCLUDE_DIR})
  target_link_libraries(common INTERFACE ${JEMALLOC_LIBRARIES})
endif()
# end of JEMALLOC

# Configuration phase
INCLUDE (CheckIncludeFileCXX)
# usage: CHECK_INCLUDE_FILES (<header> <RESULT_VARIABLE> )
CHECK_INCLUDE_FILE_CXX (crypt.h HAVE_CRYPT_H)
CHECK_INCLUDE_FILE_CXX (dirent.h HAVE_DIRENT_H)
CHECK_INCLUDE_FILE_CXX (execinfo.h HAVE_EXECINFO_H)
CHECK_INCLUDE_FILE_CXX (time.h HAVE_TIME_H)
CHECK_INCLUDE_FILE_CXX (signal.h HAVE_SIGNAL_H)
CHECK_INCLUDE_FILE_CXX (sys/resource.h HAVE_SYS_RESOURCE_H)
CHECK_INCLUDE_FILE_CXX (sys/rusage.h HAVE_SYS_RUSAGE_H)
CHECK_INCLUDE_FILE_CXX (sys/stat.h HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE_CXX (sys/time.h HAVE_SYS_TIME_H)
if( HAVE_TIME_H AND HAVE_SYS_TIME_H )
    set(TIME_WITH_SYS_TIME 1)
endif()
#CHECK_INCLUDE_FILES ("sys/param.h;sys/mount.h" HAVE_SYS_MOUNT_H)
# AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h limits.h locale.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/param.h sys/socket.h sys/time.h unistd.h])
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(GENERATED_HEADERS
        "${CMAKE_CURRENT_BINARY_DIR}/config.h"
        # This file is generated in packages/CMakeLists.txt , however set it here will complain.
        #"${CMAKE_CURRENT_BINARY_DIR}/packages/packages.autogen.h"
        )

# Build phase

# Tools
add_subdirectory(tools)

# Generated sources

set(TOOL_SRC
        "tools/build_applies.cc"
        "tools/make_func.cc"
        )

set(GENERATRED_SOURCE
        "${CMAKE_CURRENT_BINARY_DIR}/packages.fullspec"
        "${CMAKE_CURRENT_BINARY_DIR}/applies_table.autogen.cc"
        "${CMAKE_CURRENT_BINARY_DIR}/applies_table.autogen.h"
        "${CMAKE_CURRENT_BINARY_DIR}/efuns.autogen.cc"
        "${CMAKE_CURRENT_BINARY_DIR}/efuns.autogen.h"
        "${CMAKE_CURRENT_BINARY_DIR}/options.autogen.h"
        )

add_custom_command(
        OUTPUT "applies_table.autogen.cc" "applies_table.autogen.h"
        COMMAND "build_applies" "${CMAKE_CURRENT_SOURCE_DIR}"
        DEPENDS "vm/internal/applies"
)

add_custom_command(
        OUTPUT "packages.fullspec"
        COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tools/build_packages_genfiles.sh" "${CMAKE_CURRENT_SOURCE_DIR}/../" "${CMAKE_CXX_COMPILER}"  "-I${CMAKE_CURRENT_BINARY_DIR}" "-I${CMAKE_CURRENT_SOURCE_DIR}"
        DEPENDS "tools/build_packages_genfiles.sh" "base/internal/options_incl.h"
)

add_custom_command(
        OUTPUT "efuns.autogen.cc" "efuns.autogen.h"
        COMMAND "make_func" "${CMAKE_CURRENT_BINARY_DIR}/packages.fullspec"
        DEPENDS "make_func" "packages.fullspec"
)

add_custom_command(
        OUTPUT "options.autogen.h"
        COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tools/make_options_def.sh" "${CMAKE_CURRENT_SOURCE_DIR}/../" "${CMAKE_CURRENT_SOURCE_DIR}/tools/make_options_def.py" "options.autogen.h" "${CMAKE_CXX_COMPILER}" "-I${CMAKE_CURRENT_BINARY_DIR}" "-I${CMAKE_CURRENT_SOURCE_DIR}"
        DEPENDS "tools/make_options_def.sh" "tools/make_options_def.py"
)

add_custom_command(
        OUTPUT "grammar.autogen.y"
        COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tools/make_grammar.sh" "${CMAKE_CURRENT_SOURCE_DIR}/../" "${CMAKE_CURRENT_SOURCE_DIR}/vm/internal/compiler/grammar.y.pre" "${CMAKE_CXX_COMPILER}" "-I${CMAKE_CURRENT_BINARY_DIR}" "-I${CMAKE_CURRENT_SOURCE_DIR}"
        DEPENDS "tools/make_grammar.sh" "vm/internal/compiler/grammar.y.pre"
)

find_package(BISON REQUIRED)
BISON_TARGET(Grammar ${CMAKE_CURRENT_BINARY_DIR}/grammar.autogen.y ${CMAKE_CURRENT_BINARY_DIR}/grammar.autogen.cc
        DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/grammar.autogen.h)

set(GENERATRED_GRAMMAR_FILES
        "${CMAKE_CURRENT_BINARY_DIR}/grammar.autogen.y"
        "${BISON_Grammar_OUTPUT_SOURCE}"
        "${BISON_Grammar_OUTPUT_HEADER}"
        )

set(SRC
        "backend.cc"
        "comm.cc"
        "fliconv.cc"
        "mainlib.cc"
        "user.cc"
        "net/telnet.cc"
        "base/internal/debugmalloc.cc"
        "base/internal/external_port.cc"
        "base/internal/file.cc"
        "base/internal/hash.cc"
        "base/internal/log.cc"
        "base/internal/md.cc"
        "base/internal/outbuf.cc"
        "base/internal/port.cc"
        "base/internal/rc.cc"
        "base/internal/stats.cc"
        "base/internal/stralloc.cc"
        "base/internal/strput.cc"
        "vm/internal/apply.cc"
        "vm/internal/base/apply_cache.cc"
        "vm/internal/base/array.cc"
        "vm/internal/base/buffer.cc"
        "vm/internal/base/class.cc"
        "vm/internal/base/function.cc"
        "vm/internal/base/interpret.cc"
        "vm/internal/base/mapping.cc"
        "vm/internal/base/object.cc"
        "vm/internal/base/program.cc"
        "vm/internal/base/svalue.cc"
        "vm/internal/compiler/compiler.cc"
        "vm/internal/compiler/generate.cc"
        "vm/internal/compiler/icode.cc"
        "vm/internal/compiler/lex.cc"
        "vm/internal/compiler/scratchpad.cc"
        "vm/internal/compiler/trees.cc"
        "vm/internal/eval_limit.cc"
        "vm/internal/master.cc"
        "vm/internal/otable.cc"
        "vm/internal/simul_efun.cc"
        "vm/internal/simulate.cc"
        "vm/internal/trace.cc"
        "vm/internal/vm.cc"
        )

set(TEST_SRC
        "vm/internal/otable_test.cc"
        )

file(GLOB_RECURSE SRC_HEADER
        "*.h"
        )

add_library(autogen  ${GENERATED_HEADERS} ${GENERATRED_SOURCE}
  ${GENERATRED_GRAMMAR_FILES})

add_library(driver SHARED
        ${SRC} ${SRC_HEADER}
        )

target_link_libraries(driver PRIVATE autogen)

add_subdirectory(packages) # provides FLUFFOS_PACKAGES
message(STATUS "ALL PACKAGES: ${FLUFFOS_PACKAGES}")
add_library(package_all INTERFACE)
foreach(PACKAGE ${FLUFFOS_PACKAGES})
    # TODO: packages files currently depenends on generated headers, so they must be built after driver
    add_dependencies("${PACKAGE}" autogen)
    target_link_libraries(package_all INTERFACE "${PACKAGE}")
endforeach()

target_link_libraries(driver PRIVATE package_all)

add_subdirectory(thirdparty/libtelnet)
target_link_libraries(driver PRIVATE libtelnet)

# Compiler definitions
target_link_libraries(driver PRIVATE common)

# Libraries
find_package(Libevent 2.0 REQUIRED COMPONENTS libevent)
target_link_libraries(driver PRIVATE ${LIBEVENT_LIBRARIES})
target_include_directories(driver PRIVATE ${LIBEVENT_INCLUDE_DIR})

# Async use this?
find_package (Threads REQUIRED)
target_link_libraries(driver PRIVATE ${CMAKE_THREAD_LIBS_INIT})

# IPO??
include(CheckIPOSupported)
check_ipo_supported(RESULT result)
if (result)
    set_target_properties(driver PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()

# PIE code
set_target_properties(driver PROPERTIES POSITION_INDEPENDENT_CODE TURE)

# Link statictly
if(STATIC)
  if(WIN32)
    # TODO
  else()
    # target_link_options only avaiable at cmake 3.13.3
    target_link_libraries(driver "-static-libgcc -static-libstdc++ -static")
  endif()
endif()

# GTEST
#include(GoogleTest)
#enable_testing()
#gtest_discover_tests(driver)

include(helper)

message("Final Compile Flags: ")
print_iterface_target_properties(common)
print_target_properties(driver)

add_executable(portbind "portbind.cc")

install (TARGETS driver
  LIBRARY DESTINATION lib)
