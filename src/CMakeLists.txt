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
option(PACKAGE_EXTERNAL "external package" ON)
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
  list(APPEND CMAKE_REQUIRED_DEFINITIONS "-D_GNU_SOURCE")

  # Sanitizer support
  if (ENABLE_SANITIZER)
    add_compile_options("-fsanitize=address,undefined")
    add_compile_options("-fno-common")
    # add_linker_options is only avaiable cmake 3.13
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined -fno-common")
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
    "-Wno-unknown-warning-option"
    "-Wno-char-subscripts"
    "-Wno-sign-compare"
    "-Wno-unused-function"
    "-Wno-unused-parameter"
    "-Wno-unused-variable"
    "-Wno-unused-but-set-variable"
    # TODO: Code has a lot of implicit zero initializations
    "-Wno-missing-field-initializers"
    "-Wno-missing-braces"
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

# System headers

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

# System Libraries
find_package(OpenSSL 1.0.1 COMPONENTS SSL REQUIRED)
include_directories(SYSTEM ${OPENSSL_INCLUDE_DIR})

find_package(ICU REQUIRED COMPONENTS uc dt)
# Make ICU use utf-8 internally
add_compile_options("-DU_CHARSET_IS_UTF8=1")
if(STATIC)
  add_compile_options("-DU_STATIC_IMPLEMENTATION")
ENDIF()

# multiple packages uses this, so we simply do a global inclusion too.
include_directories(SYSTEM ${ICU_INCLUDE_DIRS})

# Bundled libraries
set(EVENT__DISABLE_BENCHMARK ON CACHE BOOL "" FORCE)
set(EVENT__DISABLE_MBEDTLS ON CACHE BOOL "" FORCE)
set(EVENT__DISABLE_BENCHMARK ON CACHE BOOL "" FORCE)
set(EVENT__DISABLE_TESTS ON CACHE BOOL "" FORCE)
set(EVENT__DISABLE_REGRESS ON CACHE BOOL "" FORCE)
set(EVENT__DISABLE_SAMPLES ON CACHE BOOL "" FORCE)
set(EVENT__DISABLE_MM_REPLACEMENT ON CACHE BOOL "" FORCE)
include_directories(SYSETEM ${CMAKE_CURRENT_BINARY_DIR}/thirdparty/libevent/include)
include_directories(SYSETEM thirdparty/libevent/include)
add_subdirectory(thirdparty/libevent EXCLUDE_FROM_ALL)

add_subdirectory(thirdparty/crypt EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/libtelnet EXCLUDE_FROM_ALL)
# TODO: required by windows code
add_compile_options("-DGHC_WIN_DISABLE_WSTRING_STORAGE_TYPE")
add_subdirectory(thirdparty/filesystem EXCLUDE_FROM_ALL)
include_directories(thirdparty/filesystem/include)
add_subdirectory(thirdparty/fmt EXCLUDE_FROM_ALL)
include_directories(thirdparty/fmt/include)
include_directories(SYSTEM thirdparty/json/include)
add_subdirectory(thirdparty/backward-cpp EXCLUDE_FROM_ALL)

set(DISABLE_WERROR ON CACHE BOOL "" FORCE)
set(LWS_WITH_SHARED OFF CACHE BOOL "" FORCE)
set(LWS_WITHOUT_TESTAPPS ON CACHE BOOL "" FORCE)
set(LWS_IPV6 ON CACHE BOOL "" FORCE)
set(LWS_WITHOUT_EXTENSIONS OFF CACHE BOOL "" FORCE)
set(LWS_WITH_LIBEVENT ON CACHE BOOL "" FORCE)
set(LWS_STATIC_PIC ON CACHE BOOL "" FORCE)
set(LWS_WITH_DIR OFF CACHE BOOL "" FORCE)
set(LWS_WITH_SSL ON CACHE BOOL "" FORCE)
set(LWS_WITH_HTTP2 ON CACHE BOOL "" FORCE)
set(LWS_WITHOUT_TESTAPPS ON CACHE BOOL "" FORCE)
set(LWS_WITHOUT_TEST_SERVER ON CACHE BOOL "" FORCE)
set(LWS_WITHOUT_TEST_SERVER_EXTPOLL ON CACHE BOOL "" FORCE)
set(LWS_WITHOUT_TEST_PING ON CACHE BOOL "" FORCE)
set(LWS_WITHOUT_TEST_CLIENT ON CACHE BOOL "" FORCE)
set(LWS_WITH_LEJP OFF CACHE BOOL "" FORCE)
set(LWS_WITH_LEJP_CONF OFF CACHE BOOL "" FORCE)
set(LWS_UNIX_SOCK OFF CACHE BOOL "" FORCE)
set(LWS_WITH_NETLINK OFF CACHE BOOL "" FORCE)
set(OPENSSL_FOUND 0) # hack to force lws re-search openssl
set(LIBEVENT_FOUND 1) # hack to stop libwebsockets from finding libevent
set(LIBEVENT_LIBRARIES "event_core_static")
add_subdirectory(thirdparty/libwebsockets EXCLUDE_FROM_ALL)
include_directories("thirdparty/libwebsockets/include" SYSTEM)
include_directories("${CMAKE_CURRENT_BINARY_DIR}/thirdparty/libwebsockets")

# Build phase

set(GENERATED_HEADERS
        "${CMAKE_CURRENT_BINARY_DIR}/config.h"
        "${CMAKE_CURRENT_BINARY_DIR}/packages/packages.autogen.h"
        )

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

find_package(BISON 3.8)
if (${BISON_FOUND})
  message(STATUS "BISON: ${BISON_VERSION}")
  BISON_TARGET(Grammar "${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/grammar.y"
          ${CMAKE_CURRENT_BINARY_DIR}/grammar.autogen.cc
          DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/grammar.autogen.h"
          COMPILE_FLAGS "-Wall --color=auto -rall"
          VERBOSE REPORT_FILE "${CMAKE_CURRENT_BINARY_DIR}/grammar.report"
          )
  set(GENERATRED_GRAMMAR_FILES
          "${BISON_Grammar_OUTPUT_SOURCE}"
          "${BISON_Grammar_OUTPUT_HEADER}"
          )
else()
  message(STATUS "BISON: version too low, using pre-generated source.")
  set(GENERATRED_GRAMMAR_FILES
          "${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/grammar.autogen.cc"
          "${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/grammar.autogen.h")
endif()

set(SRC
  "backend.cc"
  "comm.cc"
  "mainlib.cc"
  "user.cc"
  "symbol.cc"
  "net/msp.cc"
  "net/tls.cc"
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
  "compiler/internal/grammar_rules.cc"
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
add_dependencies(libdriver autogen)

list(APPEND FLUFFOS_LIBRARIES libdriver)

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

add_compile_options("-DFMT_HEADER_ONLY=1")
target_link_libraries(libdriver PRIVATE fmt::fmt-header-only)

list(APPEND FLUFFOS_LIBRARIES autogen)

add_subdirectory(packages) # provides FLUFFOS_PACKAGES
message(STATUS "ALL PACKAGES: ${FLUFFOS_PACKAGES}")
foreach (PACKAGE ${FLUFFOS_PACKAGES})
  # TODO: packages files currently depenends on generated headers, so they must be built after driver
  add_dependencies("${PACKAGE}" autogen)
  list(APPEND FLUFFOS_LIBRARIES "${PACKAGE}")
  target_link_libraries(libdriver PUBLIC "${PACKAGE}")
endforeach ()


target_link_libraries(libdriver PRIVATE event_core_static event_extra_static event_openssl_static)
if (NOT WIN32)
  target_link_libraries(libdriver PRIVATE event_pthreads_static)
endif()
target_link_libraries(libdriver PRIVATE crypt)
target_link_libraries(libdriver PUBLIC ${LIBWEBSOCKETS_LIBRARIES_STATIC})
target_link_libraries(libdriver PUBLIC libtelnet)
target_compile_definitions(libdriver PUBLIC ${BACKWARD_DEFINITIONS})
target_include_directories(libdriver PUBLIC ${BACKWARD_INCLUDE_DIRS})
target_link_libraries(libdriver PUBLIC ${BACKWARD_LIBRARIES})

# System libraries
target_link_libraries(libdriver PUBLIC OpenSSL::SSL)
target_link_libraries(libdriver PUBLIC ${ICU_LIBRARIES})
# target_link_libraries(libdriver PUBLIC ${LIBEVENT_LIBRARIES})

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

if(APPLE)
  set(FLUFFOS_LINK ${LINK_WHOLE_ARCHIVE} ${FLUFFOS_LIBRARIES} ${LINK_NO_WHOLE_ARCHIVE})
else()
  set(FLUFFOS_LINK -Wl,--start-group ${FLUFFOS_LIBRARIES} -Wl,--end-group)
endif()

add_executable(driver "main.cc")
target_link_libraries(driver PUBLIC ${FLUFFOS_LINK})

add_executable(lpcc "main_lpcc.cc")
target_link_libraries(lpcc PUBLIC ${FLUFFOS_LINK})

add_executable(symbol "main_symbol.cc")
target_link_libraries(symbol PUBLIC ${FLUFFOS_LINK})

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

# Update generated grammar files
if (${BISON_FOUND})
  add_custom_command(TARGET "driver" POST_BUILD
          COMMAND "${CMAKE_COMMAND}" -E copy "${BISON_Grammar_OUTPUT_SOURCE}" ${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/
          COMMAND "${CMAKE_COMMAND}" -E copy "${BISON_Grammar_OUTPUT_HEADER}" ${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/
          COMMENT "Copying to output directory")
endif()

# Must keep this line at the end, so we get all the variables.
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
