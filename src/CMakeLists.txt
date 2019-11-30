# List of compile options to tweak
option(MARCH_NATIVE "use march=native for GCC" ON)
option(STATIC "Build static version of driver" OFF)
option(USE_JEMALLOC "Build driver with jemalloc support" ON)
option(ENABLE_SANITIZER "Build driver with sanitizer support" OFF)
option(ENABLE_DTRACE "Build driver with dtrace (system-tap) support" OFF)
# Packages
option(PACKAGE_ASYNC "async package" ON)
option(PACKAGE_COMPRESS "compress package" ON)
option(PACKAGE_CONTRIB "compress package" ON)
# PACKAGE CORE should always be on
set(PACKAGE_CORE ON)
option(PACKAGE_CRYPTO "crypto package" ON)
option(PACKAGE_DB "db package" ON)
set(PACKAGE_DB_MYSQL "1")
set(PACKAGE_DB_POSTGRESQL OFF)
set(PACKAGE_DB_SQLITE OFF)
set(PACKAGE_DB_DEFAULT_DB ${PACKAGE_DB_MYSQL})
option(PACKAGE_DEVELOP "compress package" ON)
option(PACKAGE_MATH "math package" ON)
option(PACKAGE_MATRIX "matrix package" ON)
option(PACKAGE_MUDLIB_STATS "mudlib_stats package" ON)
# Package OPS is always on
set(PACKAGE_OPS ON)
option(PACKAGE_PARSER "parser package" ON)
option(PACKAGE_PCRE "pcre package" ON)
option(PACKAGE_SHA1 "sha1 package" ON)
option(PACKAGE_SOCKETS "sockets package" ON)
option(PACKAGE_TRIM "trim package" ON)
option(PACKAGE_UIDS "uids package" ON)
option(DEBUG OFF)
option(DEBUGMALLOC ${DEBUG})
# End of list of compile options.

# Globally enforce CXX17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Setup include directoires
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

foreach (lang C CXX)
  set(CMAKE_${lang}_VISIBILITY_PRESET hidden)
endforeach ()

set(CMAKE_ENABLE_EXPORTS ON)

if (${CMAKE_C_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_C_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_C_COMPILER_ID} STREQUAL "AppleClang")
  foreach (lang C CXX)
    set(CMAKE_${lang}_FLAGS_DEBUG "-O0 -g3")
    # include (minimal) debug information even for RELEASE build.
    set(CMAKE_${lang}_FLAGS_RELEASE "-O3 -DNDEBUG -g1")
    set(CMAKE_${lang}_FLAGS_RELWITHDEBINFO "${CMAKE_${lang}_FLAGS_RELEASE}")
  endforeach ()

  # Compile options
  add_compile_options(
    "-fno-omit-frame-pointer"
    # We use GNU extensions
    "-D_GNU_SOURCE"
    # strict aliasing is bad!
    "-fno-strict-aliasing"
    "-D_GLIBCXX_ASSERTIONS"
  )

  # Sanitizer support
  if (ENABLE_SANITIZER)
    add_compile_options("-fsanitize=address,undefined")
    # add_linker_options is only avaiable cmake 3.13
    set(CMAKE_EXE_LINKER_FLAGS "-fsanitize=address,undefined")
  else ()
    add_compile_options(
      "-D_FORTIFY_SOURCE=2"
    )
  endif ()

  if (MARCH_NATIVE)
    add_compile_options("-march=native")
    add_compile_options("-mtune=native")
  endif ()

  if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    if (NOT ENABLE_SANITIZER)
      add_compile_options(
        "-fstack-protector-strong"
        "-Wstack-protector"
        "--param" "ssp-buffer-size=4"
      )
      if (WIN32)
        link_libraries(ssp)
      endif () # WIN32
    endif ()
  else ()
    if (NOT ENABLE_SANITIZER)
      add_compile_options(
        "-fstack-protector-all"
        "-Wstack-protector"
        "--param" "ssp-buffer-size=4"
      )
      if (WIN32)
        link_libraries(ssp)
      endif () # WIN32
    endif ()
  endif ()
  # Warnings
  add_compile_options(
    "-Wall"
    "-Wextra"
    "-Wformat"
    "-Werror=format-security"
    "-Werror=implicit-function-declaration"
    # Turn off some warnings from GCC.
    "-Wno-char-subscripts"
    "-Wno-sign-compare"
    #"-Wno-return-type"
    "-Wno-unused-parameter"
    # TODO: Code has a lot of implict zero initilizations
    "-Wno-missing-field-initializers"
    # features
    "-fdiagnostics-show-option"
    "-fmessage-length=0"
    # Less undefined behavior
    "-fsigned-char"
    "-fwrapv")
endif ()

# LTO on release builds
if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  include(CheckIPOSupported)
  check_ipo_supported(RESULT result)
  if (result)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
  endif ()
endif()

# build PIE code by default
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# STATIC
if (STATIC)
  message(STATUS "Linking driver STATICLY.")
  if (WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
  else ()
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
  endif ()
endif ()
# end of STATIC

# Configuration phase
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  # Force enable DEBUG related flags
  set(DEBUG ON)
  set(DEBUGMALLOC ON)
else ()
  set(DEBUG OFF)
endif ()

add_subdirectory(thirdparty/filesystem EXCLUDE_FROM_ALL)
include_directories(thirdparty/filesystem/include)

INCLUDE(CheckIncludeFileCXX)
# usage: CHECK_INCLUDE_FILES (<header> <RESULT_VARIABLE> )
CHECK_INCLUDE_FILE_CXX(dirent.h HAVE_DIRENT_H)
CHECK_INCLUDE_FILE_CXX(time.h HAVE_TIME_H)
CHECK_INCLUDE_FILE_CXX(signal.h HAVE_SIGNAL_H)
CHECK_INCLUDE_FILE_CXX(sys/resource.h HAVE_SYS_RESOURCE_H)
CHECK_INCLUDE_FILE_CXX(sys/rusage.h HAVE_SYS_RUSAGE_H)
CHECK_INCLUDE_FILE_CXX(sys/stat.h HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE_CXX(sys/time.h HAVE_SYS_TIME_H)
if (HAVE_TIME_H AND HAVE_SYS_TIME_H)
  set(TIME_WITH_SYS_TIME 1)
endif ()
#CHECK_INCLUDE_FILES ("sys/param.h;sys/mount.h" HAVE_SYS_MOUNT_H)
# AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h limits.h locale.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/param.h sys/socket.h sys/time.h unistd.h])


set(GENERATED_HEADERS
  "${CMAKE_CURRENT_BINARY_DIR}/config.h"
  "${CMAKE_CURRENT_BINARY_DIR}/packages/packages.autogen.h"
  )

# Libraries
find_package(Libevent 2.0 REQUIRED COMPONENTS libevent)
# multiple packages uses this, so we simply do a global inclusion
include_directories(SYSTEM ${LIBEVENT_INCLUDE_DIR})

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
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tools/build_packages_genfiles.sh" "${CMAKE_CURRENT_SOURCE_DIR}/../" "${CMAKE_CXX_COMPILER}" "-I${CMAKE_CURRENT_BINARY_DIR}" "-I${CMAKE_CURRENT_SOURCE_DIR}"
  DEPENDS "tools/build_packages_genfiles.sh" "base/internal/options_incl.h"
)

add_custom_command(
  OUTPUT "efuns.autogen.cc" "efuns.autogen.h"
  COMMAND "make_func" "${CMAKE_CURRENT_BINARY_DIR}/packages.fullspec"
  DEPENDS "make_func" "packages.fullspec"
)

add_custom_command(
  OUTPUT "options.autogen.h"
  COMMAND "make_options_defs" "-I${CMAKE_CURRENT_BINARY_DIR}" "-I${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/base/internal/options_incl.h"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/base/internal/options_incl.h"
)

add_custom_command(
  OUTPUT "grammar.autogen.y"
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tools/make_grammar.sh" "${CMAKE_CURRENT_SOURCE_DIR}/../" "${CMAKE_CURRENT_SOURCE_DIR}/vm/internal/compiler/grammar.y.pre" "${CMAKE_CXX_COMPILER}" "-I${CMAKE_CURRENT_BINARY_DIR}" "-I${CMAKE_CURRENT_SOURCE_DIR}"
  DEPENDS "tools/make_grammar.sh" "vm/internal/compiler/grammar.y.pre"
)

find_package(BISON REQUIRED)
BISON_TARGET(Grammar ${CMAKE_CURRENT_BINARY_DIR}/grammar.autogen.y ${CMAKE_CURRENT_BINARY_DIR}/grammar.autogen.cc
  DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/grammar.autogen.h")

set(GENERATRED_GRAMMAR_FILES
  "${CMAKE_CURRENT_BINARY_DIR}/grammar.autogen.y"
  "${BISON_Grammar_OUTPUT_SOURCE}"
  "${BISON_Grammar_OUTPUT_HEADER}"
  )

set(SRC
  "backend.cc"
  "comm.cc"
  "fliconv.cc"
  "main.cc"
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
  "vm/internal/posix_timers.cc"
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

set_source_files_properties(${GENERATED_HEADERS} ${GENERATRED_SOURCE}
  ${GENERATRED_GRAMMAR_FILES} PROPERTIES GENERATED TRUE)

add_library(autogen ${GENERATED_HEADERS} ${GENERATRED_SOURCE}
  ${GENERATRED_GRAMMAR_FILES})
add_dependencies(autogen make_options_defs)

add_executable(driver
  ${SRC} ${SRC_HEADER}
  )

target_link_libraries(driver PRIVATE ghc_filesystem)

target_link_libraries(driver PUBLIC autogen)

add_subdirectory(packages) # provides FLUFFOS_PACKAGES
message(STATUS "ALL PACKAGES: ${FLUFFOS_PACKAGES}")
foreach (PACKAGE ${FLUFFOS_PACKAGES})
  # TODO: packages files currently depenends on generated headers, so they must be built after driver
  add_dependencies("${PACKAGE}" autogen)
  target_link_libraries(driver PUBLIC "${PACKAGE}")
endforeach ()

add_subdirectory(thirdparty/crypt)
target_link_libraries(driver PRIVATE crypt)

# DTRACE support
if (ENABLE_DTRACE)
  message(STATUS "DTRACE Support: ON")
  add_subdirectory(tracing)
  target_link_libraries(driver PRIVATE tracing)
endif ()

# Winsocks
IF (WIN32)
  target_link_libraries(driver PRIVATE wsock32 ws2_32)
ENDIF ()

# Third party libraries
add_subdirectory(thirdparty/libtelnet EXCLUDE_FROM_ALL)
target_link_libraries(driver PUBLIC libtelnet)

add_subdirectory(thirdparty/backward-cpp EXCLUDE_FROM_ALL)
add_backward(driver)

# Link statictly
if (STATIC)
  if (WIN32)
    # TODO
    message(FATAL_ERROR "Build staticly on Windows is not currently supported")
  else ()
    if (APPLE)
      message(FATAL_ERROR "Build staticly on OSX is not supported")
    else ()
      # target_link_options only avaiable at cmake 3.13.3
      target_link_libraries(driver PUBLIC "-static-libgcc -static-libstdc++ -Wl,-Bstatic")
    endif ()
  endif ()
endif ()

# JEMALLOC
if (CYGWIN)
  set(USE_JEMALLOC OFF)
  message(STATUS "JEMALLOC is not supported on CYGWIN.")
endif ()
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(USE_JEMALLOC OFF)
  message(STATUS "JEMALLOC is disabled on debug build.")
endif ()
if (USE_JEMALLOC)
  find_package(jemalloc REQUIRED)
  set(HAVE_JEMALLOC TRUE) # used in config.h.in
  include_directories(SYSTEM ${JEMALLOC_INCLUDE_DIR})
  target_link_libraries(driver PRIVATE ${JEMALLOC_LIBRARIES})
endif ()
# end of JEMALLOC

# libevent
target_link_libraries(driver PUBLIC ${LIBEVENT_LIBRARIES})

find_package(Threads REQUIRED)
target_link_libraries(driver PRIVATE Threads::Threads)

include(CheckLibraryExists)
CHECK_LIBRARY_EXISTS(rt clock_gettime "time.h" HAVE_CLOCK_GETTIME_IN_RT)
if (HAVE_CLOCK_GETTIME_IN_RT)
  target_link_libraries(driver PRIVATE rt)
endif ()

# GTEST
#include(GoogleTest)
#enable_testing()
#gtest_discover_tests(driver)

include(helper)

message("Final Compile Flags: ")
print_target_properties(driver)

install(TARGETS driver
  RUNTIME DESTINATION bin)

if (NOT WIN32)
  add_executable(portbind "portbind.cc")
  install(TARGETS portbind
    RUNTIME DESTINATION bin)
endif ()

# Must keep this line at the end, so we get all the variables.
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
