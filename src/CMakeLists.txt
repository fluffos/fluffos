# List of compile options to tweak
option(MARCH_NATIVE "use march=native for GCC" ON)
option(STATIC "Build static version of driver" OFF)
option(USE_JEMALLOC "Build driver with jemalloc support" ON)
option(ENABLE_SANITIZER "Build driver with sanitizer support" OFF)
# Packages
option(PACKAGE_ASYNC "async package" ON)
option(PACKAGE_COMPRESS "compress package" ON)
option(PACKAGE_CONTRIB "contrib package" ON)
# PACKAGE CORE should always be on
set(PACKAGE_CORE ON)
option(PACKAGE_CRYPTO "crypto package" ON)
option(PACKAGE_DB "db package" ON)
set(PACKAGE_DB_MYSQL "1" CACHE STRING "DB Type for Mysql")
set(PACKAGE_DB_POSTGRESQL "" CACHE STRING "DB Type for POSTGRESQL")
set(PACKAGE_DB_SQLITE "" CACHE STRING "DB Type for SQLITE")
set(PACKAGE_DB_DEFAULT_DB "1" CACHE STRING "Default DB Handle")
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
option(PACKAGE_EXTERNAL "external package" OFF)
option(DEBUG OFF)
option(DEBUGMALLOC ${DEBUG})
# End of list of compile options.

# Globally enforce CXX17 and C11
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Setup include directoires
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

foreach (lang C CXX)
  set(CMAKE_${lang}_VISIBILITY_PRESET hidden)
endforeach ()

set(CMAKE_ENABLE_EXPORTS ON)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang")
  foreach (lang C CXX)
    set(CMAKE_${lang}_FLAGS_DEBUG "-O0 -g3")
    # include (minimal) debug information even for RELEASE build.
    set(CMAKE_${lang}_FLAGS_RELEASE "-O3 -DNDEBUG -g")
    set(CMAKE_${lang}_FLAGS_RELWITHDEBINFO "${CMAKE_${lang}_FLAGS_RELEASE}")
  endforeach ()

  # Compile options
  add_compile_options(
    "-fno-omit-frame-pointer"
    "-D_GNU_SOURCE"
    # strict aliasing is bad!
    "-fno-strict-aliasing"
    "-D_GLIBCXX_ASSERTIONS"
  )

  # Sanitizer support
  if (ENABLE_SANITIZER)
    add_compile_options("-fsanitize=address,undefined")
    # add_linker_options is only avaiable cmake 3.13
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined")
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
    $<$<COMPILE_LANGUAGE:C>:-Werror=implicit-function-declaration>
    # Turn off some warnings from GCC.
    "-Wno-char-subscripts"
    "-Wno-sign-compare"
    "-Wno-unused-parameter"
    # TODO: Code has a lot of implicit zero initializations
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

# build PIC code by default
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-fPIE" SUPPORT_PIE)
if (SUPPORT_PIE)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")
endif()

# STATIC
IF(WIN32)
  set(STATIC ON)
  message(STATUS "Default to build static on windows!")
ENDIF()

if (STATIC)
  message(STATUS "Linking static driver.")
  if (WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
  elseif (APPLE)
    message(FATAL_ERROR "Build static driver on OSX is not supported")
  else ()
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
  endif ()
  # target_link_options only available at cmake 3.13.3
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++ -Wl,-Bstatic")
else()
  message(STATUS "Linking dynamic driver.")
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

set(DISABLE_WERROR ON CACHE BOOL "" FORCE)
set(LWS_WITH_SHARED OFF CACHE BOOL "" FORCE)
set(LWS_WITHOUT_TESTAPPS ON CACHE BOOL "" FORCE)
set(LWS_IPV6 ON CACHE BOOL "" FORCE)
set(LWS_WITHOUT_EXTENSIONS OFF CACHE BOOL "" FORCE)
set(LWS_WITH_LIBEVENT ON CACHE BOOL "" FORCE)
set(LWS_STATIC_PIC ON CACHE BOOL "" FORCE)
set(LWS_WITH_DIR OFF CACHE BOOL "" FORCE)
set(LWS_WITH_SSL OFF CACHE BOOL "" FORCE)
set(LWS_WITHOUT_TESTAPPS ON CACHE BOOL "" FORCE)
set(LWS_WITHOUT_TEST_SERVER ON CACHE BOOL "" FORCE)
set(LWS_WITHOUT_TEST_SERVER_EXTPOLL ON CACHE BOOL "" FORCE)
set(LWS_WITHOUT_TEST_PING ON CACHE BOOL "" FORCE)
set(LWS_WITHOUT_TEST_CLIENT ON CACHE BOOL "" FORCE)
set(LWS_WITH_LEJP OFF CACHE BOOL "" FORCE)
set(LWS_WITH_LEJP_CONF OFF CACHE BOOL "" FORCE)
set(LWS_UNIX_SOCK OFF CACHE BOOL "" FORCE)
add_subdirectory(thirdparty/libwebsockets EXCLUDE_FROM_ALL)
include_directories("thirdparty/libwebsockets/include" SYSTEM)
include_directories("${CMAKE_CURRENT_BINARY_DIR}/thirdparty/libwebsockets")

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
if(WIN32)
  find_package(Libevent 2.0 REQUIRED COMPONENTS libevent)
else()
  find_package(Libevent 2.0 REQUIRED COMPONENTS libevent pthreads)
endif()
# multiple packages uses this, so we simply do a global inclusion
include_directories(SYSTEM ${LIBEVENT_INCLUDE_DIR})

# Libraries
find_package(ICU REQUIRED COMPONENTS uc dt)
# Make ICU use utf-8 internally
add_compile_options("-DU_CHARSET_IS_UTF8=1")
# multiple packages uses this, so we simply do a global inclusion too.
include_directories(SYSTEM ${ICU_INCLUDE_DIRS})

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
  DEPENDS "vm/internal/applies" "build_applies"
)

add_custom_command(
  OUTPUT "packages.fullspec"
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tools/build_packages_genfiles.sh" "${CMAKE_CURRENT_SOURCE_DIR}/../" "${CMAKE_CXX_COMPILER}" "-I${CMAKE_CURRENT_BINARY_DIR}" "-I${CMAKE_CURRENT_SOURCE_DIR}"
  DEPENDS "tools/build_packages_genfiles.sh" "base/internal/options_incl.h"
)
file(GLOB ALL_SPEC_FILES files "packages/*/*.spec")
foreach (file ${ALL_SPEC_FILES})
  add_custom_command(OUTPUT "packages.fullspec"
          DEPENDS "${file}" APPEND)
endforeach ()

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
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tools/make_grammar.sh" "${CMAKE_CURRENT_SOURCE_DIR}/../" "${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/grammar.y.pre" "${CMAKE_CXX_COMPILER}" "-I${CMAKE_CURRENT_BINARY_DIR}" "-I${CMAKE_CURRENT_SOURCE_DIR}"
  DEPENDS "tools/make_grammar.sh" "compiler/internal/grammar.y.pre"
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
  "mainlib.cc"
  "user.cc"
  "net/telnet.cc"
  "net/websocket.cc"
  "net/ws_ascii.cc"
  "base/internal/debugmalloc.cc"
  "base/internal/external_port.cc"
  "base/internal/file.cc"
  "base/internal/hash.cc"
  "base/internal/log.cc"
  "base/internal/md.cc"
  "base/internal/outbuf.cc"
  "base/internal/port.cc"
  "base/internal/rc.cc"
  "base/internal/rusage.cc"
  "base/internal/stats.cc"
  "base/internal/stralloc.cc"
  "base/internal/strput.cc"
  "base/internal/strutils.cc"
  "base/internal/tracing.cc"
  "compiler/internal/compiler.cc"
  "compiler/internal/disassembler.cc"
  "compiler/internal/generate.cc"
  "compiler/internal/icode.cc"
  "compiler/internal/lex.cc"
  "compiler/internal/scratchpad.cc"
  "compiler/internal/trees.cc"
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
  "vm/internal/apply.cc"
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

add_library(libdriver
  ${SRC} ${SRC_HEADER}
  )

# JEMALLOC must be the first!
if (CYGWIN)
  set(USE_JEMALLOC OFF)
  message(STATUS "JEMALLOC is not supported on CYGWIN.")
endif ()
if(ENABLE_SANITIZER)
  set(USE_JEMALLOC OFF)
  message(STATUS "AddressSanitizer doesn't work with JEMALLOC, force disabling!")
endif()

if (USE_JEMALLOC)
  find_package(jemalloc REQUIRED)
  set(HAVE_JEMALLOC TRUE) # used in config.h.in
  message(STATUS "Jemalloc Version: ${JEMALLOC_VERSION}, linking ${JEMALLOC_LIBRARIES}")
  include_directories(SYSTEM ${JEMALLOC_INCLUDE_DIRS})
  target_link_libraries(libdriver PUBLIC ${JEMALLOC_LIBRARIES})
  target_link_libraries(libdriver PUBLIC ${CMAKE_DL_LIBS})
endif ()
# end of JEMALLOC

target_link_libraries(libdriver PUBLIC ghc_filesystem)

target_link_libraries(libdriver PUBLIC autogen)

add_subdirectory(packages) # provides FLUFFOS_PACKAGES
message(STATUS "ALL PACKAGES: ${FLUFFOS_PACKAGES}")
foreach (PACKAGE ${FLUFFOS_PACKAGES})
  # TODO: packages files currently depenends on generated headers, so they must be built after driver
  add_dependencies("${PACKAGE}" autogen)
  target_link_libraries(libdriver PUBLIC "${PACKAGE}")
endforeach ()

add_subdirectory(thirdparty/crypt)
target_link_libraries(libdriver PRIVATE crypt)

target_link_libraries(libdriver PUBLIC ${LIBWEBSOCKETS_LIBRARIES_STATIC})
# Third party libraries
add_subdirectory(thirdparty/libtelnet EXCLUDE_FROM_ALL)
target_link_libraries(libdriver PUBLIC libtelnet)

target_link_libraries(libdriver PUBLIC ${ICU_LIBRARIES})

add_subdirectory(thirdparty/backward-cpp EXCLUDE_FROM_ALL)
target_compile_definitions(libdriver PUBLIC ${BACKWARD_DEFINITIONS})
target_include_directories(libdriver PUBLIC ${BACKWARD_INCLUDE_DIRS})
target_link_libraries(libdriver PUBLIC ${BACKWARD_LIBRARIES})

# libevent
target_link_libraries(libdriver PUBLIC ${LIBEVENT_LIBRARIES})

# Platform Libraries

# Winsocks
IF (WIN32)
  target_link_libraries(libdriver PUBLIC ws2_32)
ENDIF ()

if(NOT WIN32)
  find_package(Threads REQUIRED)
  target_link_libraries(libdriver PUBLIC Threads::Threads)
endif()

include(CheckLibraryExists)
CHECK_LIBRARY_EXISTS(rt clock_gettime "time.h" HAVE_CLOCK_GETTIME_IN_RT)
if (HAVE_CLOCK_GETTIME_IN_RT)
  target_link_libraries(libdriver PUBLIC rt)
endif ()

# GTEST
#include(GoogleTest)
#enable_testing()
#gtest_discover_tests(driver)

set(LINK_WHOLE_ARCHIVE -Wl,--whole-archive)
set(LINK_NO_WHOLE_ARCHIVE -Wl,--no-whole-archive)
if (APPLE)
  set(LINK_WHOLE_ARCHIVE -Wl,-force_load)
  set(LINK_NO_WHOLE_ARCHIVE "")
endif()

add_executable(driver "main.cc")
target_link_libraries(driver PUBLIC ${LINK_WHOLE_ARCHIVE} libdriver ${LINK_NO_WHOLE_ARCHIVE})

add_executable(lpcc "main_lpcc.cc")
target_link_libraries(lpcc PUBLIC ${LINK_WHOLE_ARCHIVE} libdriver ${LINK_NO_WHOLE_ARCHIVE})

include(helper)

message("Final Compile Flags: ")
print_target_properties(driver)

install(TARGETS driver lpcc
  RUNTIME DESTINATION bin)

# LPC stdlib files
install(DIRECTORY ../testsuite/std
        DESTINATION bin)

# LPC include files
install(DIRECTORY include
        DESTINATION bin)

# Websocket http files
install(DIRECTORY www
        DESTINATION bin)

if (NOT WIN32)
  add_executable(portbind "portbind.cc")
  install(TARGETS portbind
    RUNTIME DESTINATION bin)
endif ()

# Must keep this line at the end, so we get all the variables.
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
