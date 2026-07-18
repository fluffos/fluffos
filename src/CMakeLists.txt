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
option(PACKAGE_DEBUGGER "LPC source-level debugger efuns (debug_break, debugger_attached)" ON)
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
option(PACKAGE_FFI "foreign function interface package (libffi)" ON)
# The JS bridge only exists on the Emscripten/WASM target (LPC <-> page
# JavaScript); default it off everywhere else.
if (EMSCRIPTEN)
  set(_PACKAGE_JSBRIDGE_DEFAULT ON)
else ()
  set(_PACKAGE_JSBRIDGE_DEFAULT OFF)
endif ()
option(PACKAGE_JSBRIDGE "LPC <-> JavaScript bridge (Emscripten/WASM builds only)" ${_PACKAGE_JSBRIDGE_DEFAULT})
option(PACKAGE_DWLIB "discworld mudlib support package" OFF)
option(DEBUG OFF)
option(DEBUGMALLOC ${DEBUG})
# End of list of compile options.

# Emscripten/WASM target (see wasm/README.md): no sockets, no processes,
# no threads. Packages that hard-require those (or system libraries that
# have not been cross-built) are forced off; everything else -- compiler,
# VM, all core efuns, the telnet protocol layer -- builds unchanged.
if (EMSCRIPTEN)
  set(PACKAGE_ASYNC OFF)     # worker threads
  set(PACKAGE_COMPRESS OFF)  # compressing to a same-page client wastes CPU+size
  set(PACKAGE_CRYPTO OFF)    # OpenSSL
  set(PACKAGE_DB OFF)        # MySQL/SQLite/Postgres clients
  set(PACKAGE_EXTERNAL OFF)  # posix_spawn + socketpair
  set(PACKAGE_FFI OFF)       # libffi/dlopen
  set(PACKAGE_SOCKETS OFF)   # BSD sockets
  set(PACKAGE_PCRE OFF)      # libpcre not cross-built (yet)
  set(MARCH_NATIVE OFF)
  set(USE_JEMALLOC OFF)
  set(STATIC OFF)

  # Cross-built static ICU (see tools/wasm/build-deps.sh).
  set(FLUFFOS_WASM_DEPS "/opt/wasm-deps" CACHE PATH
      "Prefix containing wasm-built static ICU")
  list(APPEND CMAKE_FIND_ROOT_PATH "${FLUFFOS_WASM_DEPS}")
  if (NOT ICU_ROOT)
    set(ICU_ROOT "${FLUFFOS_WASM_DEPS}")
  endif ()
endif ()

# Globally enforce CXX17 and C11
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
    add_compile_options("-fno-sanitize-recover=all")
    add_compile_options("-fno-common")

    add_link_options("-fsanitize=address,undefined")
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
    if (NOT ENABLE_SANITIZER AND NOT EMSCRIPTEN)
      add_compile_options(
        "-fstack-protector-strong"
        "-Wstack-protector"
      )
      if (WIN32)
        link_libraries(ssp)
      endif () # WIN32
    endif ()
  else ()
    if (NOT ENABLE_SANITIZER AND NOT EMSCRIPTEN)
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
    "-Wno-trigraphs"
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
    "-fwrapv"
  )
endif ()

if (EMSCRIPTEN)
  # The driver relies on C++ exceptions for LPC error handling; Emscripten
  # disables catching by default. On a modern emsdk use native WebAssembly
  # exceptions (wasm EH) -- much faster than the legacy JS-based unwinding
  # and supported by every current browser and node >= 18. Older toolchains
  # (e.g. distro-packaged 3.1.x) fall back to the JS-based scheme.
  if (EMSCRIPTEN_VERSION VERSION_GREATER_EQUAL "3.1.57")
    add_compile_options("-fwasm-exceptions")
    add_link_options("-fwasm-exceptions")
  else ()
    add_compile_options("-sDISABLE_EXCEPTION_CATCHING=0")
    add_link_options("-sDISABLE_EXCEPTION_CATCHING=0")
  endif ()
endif ()

# LTO on release builds
if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug" AND NOT EMSCRIPTEN)
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
if (SUPPORT_PIE AND NOT EMSCRIPTEN)
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

if (NOT EMSCRIPTEN)
  # zlib backs the core's gzip'd file support (write_file flag 2,
  # compressed save_object, transparent gz reads) and MCCP; the wasm
  # target drops it entirely and HAVE_ZLIB gates those code paths.
  # This must stay BELOW the STATIC block: find_package before the
  # static library-suffix setup caches the shared libz, which poisons
  # every later try_compile run with -static (libwebsockets' OpenSSL
  # feature checks would all fail and misconfigure it), and defined
  # here (before the package/net subdirectories) so every target sees
  # the macro.
  find_package(ZLIB REQUIRED)
  add_compile_definitions(HAVE_ZLIB)
endif ()

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
if (NOT EMSCRIPTEN)
  find_package(OpenSSL 1.0.1 COMPONENTS SSL REQUIRED)
  include_directories(AFTER SYSTEM ${OPENSSL_INCLUDE_DIR})
endif ()

set(ICU_DEBUG ON)
find_package(ICU REQUIRED COMPONENTS uc dt)
# Make ICU use utf-8 internally
add_compile_options("-DU_CHARSET_IS_UTF8=1")
if(STATIC)
  add_compile_options("-DU_STATIC_IMPLEMENTATION")
ENDIF()

# multiple packages uses this, so we simply do a global inclusion too.
include_directories(AFTER SYSTEM ${ICU_INCLUDE_DIRS})

# Bundled libraries
if (NOT EMSCRIPTEN)
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
endif ()

add_subdirectory(thirdparty/crypt EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/libtelnet EXCLUDE_FROM_ALL)
# TODO: required by windows code
add_compile_options("-DGHC_WIN_DISABLE_WSTRING_STORAGE_TYPE")
add_subdirectory(thirdparty/filesystem EXCLUDE_FROM_ALL)
include_directories(thirdparty/filesystem/include)
add_subdirectory(thirdparty/fmt EXCLUDE_FROM_ALL)
include_directories(thirdparty/fmt/include)
include_directories(AFTER SYSTEM thirdparty/json/include)
if (NOT EMSCRIPTEN)
  # backward-cpp (crash backtraces) needs native unwind support.
  add_subdirectory(thirdparty/backward-cpp EXCLUDE_FROM_ALL)
endif ()
include_directories(AFTER SYSTEM thirdparty/argparse/include)

if (NOT EMSCRIPTEN)
set(DISABLE_WERROR ON CACHE BOOL "" FORCE)
set(LWS_WITH_SHARED OFF CACHE BOOL "" FORCE)
set(LWS_WITHOUT_TESTAPPS ON CACHE BOOL "" FORCE)
set(LWS_IPV6 ON CACHE BOOL "" FORCE)
set(LWS_WITHOUT_EXTENSIONS OFF CACHE BOOL "" FORCE)
set(LWS_WITH_LIBEVENT ON CACHE BOOL "" FORCE)
# Since v4.1 lws defaults to building event libs as runtime plugins on unix;
# we need the libevent event lib compiled statically into the library.
set(LWS_WITH_EVLIB_PLUGINS OFF CACHE BOOL "" FORCE)
set(LWS_STATIC_PIC ON CACHE BOOL "" FORCE)
set(LWS_WITH_DIR OFF CACHE BOOL "" FORCE)
set(LWS_WITH_SSL ON CACHE BOOL "" FORCE)
set(LWS_WITH_HTTP2 ON CACHE BOOL "" FORCE)
# Explicitly OFF (not just implied via LWS_WITHOUT_TESTAPPS): the pruned
# vendor tree no longer ships minimal-examples*, and lws's unguarded
# add_subdirectory would hard-fail if this ever flipped back on.
set(LWS_WITH_MINIMAL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LWS_WITHOUT_TEST_SERVER ON CACHE BOOL "" FORCE)
set(LWS_WITHOUT_TEST_SERVER_EXTPOLL ON CACHE BOOL "" FORCE)
set(LWS_WITHOUT_TEST_PING ON CACHE BOOL "" FORCE)
set(LWS_WITHOUT_TEST_CLIENT ON CACHE BOOL "" FORCE)
set(LWS_WITH_LEJP OFF CACHE BOOL "" FORCE)
set(LWS_WITH_LEJP_CONF OFF CACHE BOOL "" FORCE)
# Defaults ON since v4.3 and force-implies LWS_WITH_GENCRYPTO, whose openssl
# backend trips an #error guard against deprecated EC APIs on OpenSSL 3.0+.
set(LWS_WITH_HTTP_DIGEST_AUTH OFF CACHE BOOL "" FORCE)
# HTML5/PNG display-list parsers (default ON since v4.4) are for embedded
# display targets; LHP also force-implies the Secure Streams subsystem
# (which needs LEJP). None of it is used by the driver.
set(LWS_WITH_LHP OFF CACHE BOOL "" FORCE)
set(LWS_WITH_UPNG OFF CACHE BOOL "" FORCE)
set(LWS_WITH_SECURE_STREAMS OFF CACHE BOOL "" FORCE)
set(LWS_UNIX_SOCK OFF CACHE BOOL "" FORCE)
set(LWS_WITH_NETLINK OFF CACHE BOOL "" FORCE)
set(OPENSSL_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIR} CACHE STRING "" FORCE)
set(OPENSSL_FOUND 1) # hack to avoid lws re-search openssl
set(LIBEVENT_FOUND 1) # hack to stop libwebsockets from finding libevent
set(LIBEVENT_LIBRARIES "event_core_static")
add_subdirectory(thirdparty/libwebsockets EXCLUDE_FROM_ALL)
include_directories("thirdparty/libwebsockets/include" SYSTEM)
include_directories("${CMAKE_CURRENT_BINARY_DIR}/thirdparty/libwebsockets")
endif () # NOT EMSCRIPTEN (libwebsockets)

# Build phase

set(GENERATED_HEADERS
        "${CMAKE_CURRENT_BINARY_DIR}/config.h"
        "${CMAKE_CURRENT_BINARY_DIR}/packages/packages.autogen.h"
        )

# Tools
#
# The codegen tools (make_func, build_applies, make_options_defs) run at
# BUILD time, so when cross-compiling (Emscripten) they must come from a
# previously-built native tree: pass -DFLUFFOS_HOST_TOOLS_DIR=<dir>
# pointing at a native build's src/tools directory (see wasm/README.md
# and tools/wasm/build.sh).
if (CMAKE_CROSSCOMPILING)
  set(FLUFFOS_HOST_TOOLS_DIR "" CACHE PATH "Directory with native make_func/build_applies/make_options_defs binaries")
  if (NOT FLUFFOS_HOST_TOOLS_DIR)
    message(FATAL_ERROR "Cross-compiling requires -DFLUFFOS_HOST_TOOLS_DIR=<native build>/src/tools")
  endif ()
  set(TOOL_MAKE_FUNC "${FLUFFOS_HOST_TOOLS_DIR}/make_func")
  set(TOOL_BUILD_APPLIES "${FLUFFOS_HOST_TOOLS_DIR}/build_applies")
  set(TOOL_MAKE_OPTIONS_DEFS "${FLUFFOS_HOST_TOOLS_DIR}/make_options_defs")
else ()
  add_subdirectory(tools)
  set(TOOL_MAKE_FUNC make_func)
  set(TOOL_BUILD_APPLIES build_applies)
  set(TOOL_MAKE_OPTIONS_DEFS make_options_defs)
endif ()

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

set(GENERATRED_DOCS
  "${CMAKE_CURRENT_BINARY_DIR}/keywords.json"
)

add_custom_command(
  OUTPUT "applies_table.autogen.cc" "applies_table.autogen.h"
  COMMAND "${TOOL_BUILD_APPLIES}" "${CMAKE_CURRENT_SOURCE_DIR}"
  DEPENDS "vm/internal/applies" "${TOOL_BUILD_APPLIES}"
)

find_program(BASH_EXECUTABLE bash REQUIRED)
add_custom_command(
  OUTPUT "packages.fullspec"
  COMMAND "${BASH_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/build_packages_genfiles.sh" "${CMAKE_CURRENT_SOURCE_DIR}/../" "${CMAKE_CXX_COMPILER}" "-I${CMAKE_CURRENT_BINARY_DIR}" "-I${CMAKE_CURRENT_SOURCE_DIR}"
  # packages.autogen.h carries the enabled-package set: without this
  # dependency, toggling a package option left a stale efun table.
  DEPENDS "tools/build_packages_genfiles.sh" "base/internal/options_incl.h"
          "${CMAKE_CURRENT_BINARY_DIR}/packages/packages.autogen.h"
)
file(GLOB ALL_SPEC_FILES files "packages/*/*.spec")
foreach (file ${ALL_SPEC_FILES})
  add_custom_command(OUTPUT "packages.fullspec"
          DEPENDS "${file}" APPEND)
endforeach ()

add_custom_command(
  OUTPUT "efuns.autogen.cc" "efuns.autogen.h"
  COMMAND "${TOOL_MAKE_FUNC}" "${CMAKE_CURRENT_BINARY_DIR}/packages.fullspec"
  DEPENDS "${TOOL_MAKE_FUNC}" "packages.fullspec"
)

add_custom_command(
  OUTPUT "options.autogen.h"
  COMMAND "${TOOL_MAKE_OPTIONS_DEFS}" "-I${CMAKE_CURRENT_BINARY_DIR}" "-I${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/base/internal/options_incl.h"
  # options_incl.h includes the generated package list and config.h; the
  # LPC __PACKAGE_*__ predefines go stale if those change without this
  # command re-running (e.g. toggling a package in an existing build dir).
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/base/internal/options_incl.h"
          "${CMAKE_CURRENT_BINARY_DIR}/packages/packages.autogen.h"
          "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

if (NOT CMAKE_CROSSCOMPILING)
add_custom_command(
  OUTPUT "keywords.json"
  COMMAND "generate_keywords"
  DEPENDS "generate_keywords" "${CMAKE_CURRENT_BINARY_DIR}/efuns.autogen.h"
)
endif ()

find_package(BISON 3.8)
# The committed generated file pins the generator version that produced it:
# a host with an OLDER bison must not regenerate (it cannot reproduce the
# committed output and would churn it on the post-build copy-back); it
# builds from the committed copy instead. Hosts at or above the pin still
# regenerate, so CI keeps validating grammar.y against a real toolchain.
set(FLUFFOS_PINNED_BISON_VERSION "0")
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/grammar.autogen.cc"
     _fluffos_bison_pin_line REGEX "made by GNU Bison" LIMIT_COUNT 1)
if (_fluffos_bison_pin_line MATCHES "GNU Bison ([0-9]+\\.[0-9]+(\\.[0-9]+)?)")
  set(FLUFFOS_PINNED_BISON_VERSION "${CMAKE_MATCH_1}")
endif()
set(FLUFFOS_BISON_REGEN FALSE)
if (BISON_FOUND AND NOT BISON_VERSION VERSION_LESS FLUFFOS_PINNED_BISON_VERSION)
  set(FLUFFOS_BISON_REGEN TRUE)
endif()
if (FLUFFOS_BISON_REGEN)
  message(STATUS "BISON: ${BISON_VERSION} (committed files pinned at ${FLUFFOS_PINNED_BISON_VERSION})")
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

  # Generate grammar.xml for EBNF / tooling (streamed by generate_ebnf.py)
  set(GRAMMAR_XML "${CMAKE_CURRENT_BINARY_DIR}/grammar.xml")
  add_custom_command(
    OUTPUT  "${GRAMMAR_XML}"
    COMMAND "${BISON_EXECUTABLE}"
            "--xml=${GRAMMAR_XML}"
            "-o" "${CMAKE_CURRENT_BINARY_DIR}/grammar_xml_dummy.cc"
            "${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/grammar.y"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/grammar.y"
    COMMENT "Generating grammar.xml from grammar.y"
  )

  find_package(Python3 QUIET COMPONENTS Interpreter)
  if(Python3_FOUND)
    add_custom_target(generate_ebnf
      COMMAND "${Python3_EXECUTABLE}"
              "${CMAKE_SOURCE_DIR}/tools/lpc-syntax/generate_ebnf.py"
              "${GRAMMAR_XML}"
              "${CMAKE_SOURCE_DIR}/tools/lpc-syntax/grammar.ebnf"
              "${CMAKE_SOURCE_DIR}/tools/lpc-syntax/lpc-grammar.json"
      DEPENDS "${GRAMMAR_XML}"
              "${CMAKE_SOURCE_DIR}/tools/lpc-syntax/grammar_lexical.ebnf.in"
      COMMENT "Generating grammar.ebnf (3 layers) + lpc-grammar.json from grammar.xml"
    )
  endif()
else()
  if (BISON_FOUND)
    message(STATUS "BISON: ${BISON_VERSION} is older than the pinned ${FLUFFOS_PINNED_BISON_VERSION}, using pre-generated source.")
  else()
    message(STATUS "BISON: version too low, using pre-generated source.")
  endif()
  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/grammar.autogen.cc"
           "${CMAKE_CURRENT_BINARY_DIR}/grammar.autogen.h"
    COMMAND "${CMAKE_COMMAND}" -D ACTION=DENORMALIZE -D FILE_IN="${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/grammar.autogen.cc" -D FILE_OUT="${CMAKE_CURRENT_BINARY_DIR}/grammar.autogen.cc" -D REPO_ROOT="${CMAKE_SOURCE_DIR}" -D BUILD_ROOT="${CMAKE_BINARY_DIR}" -P "${CMAKE_SOURCE_DIR}/cmake/util.cmake"
    COMMAND "${CMAKE_COMMAND}" -D ACTION=DENORMALIZE -D FILE_IN="${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/grammar.autogen.h" -D FILE_OUT="${CMAKE_CURRENT_BINARY_DIR}/grammar.autogen.h" -D REPO_ROOT="${CMAKE_SOURCE_DIR}" -D BUILD_ROOT="${CMAKE_BINARY_DIR}" -P "${CMAKE_SOURCE_DIR}/cmake/util.cmake"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/grammar.autogen.cc"
            "${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/grammar.autogen.h"
    COMMENT "Restoring Bison-generated grammar files with local paths"
  )
  set(GENERATRED_GRAMMAR_FILES
          "${CMAKE_CURRENT_BINARY_DIR}/grammar.autogen.cc"
          "${CMAKE_CURRENT_BINARY_DIR}/grammar.autogen.h")
endif()

# ── Flex (LPC lexer) ──────────────────────────────────────────────────────
find_package(FLEX 2.6)
# Same version-pin scheme as bison above, read from the committed scanner's
# YY_FLEX_*_VERSION defines.
set(FLUFFOS_PINNED_FLEX_VERSION "0")
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/lexer.autogen.cc"
     _fluffos_flex_pin_lines REGEX "#define YY_FLEX_(MAJOR|MINOR|SUBMINOR)_VERSION")
if ("${_fluffos_flex_pin_lines}" MATCHES "MAJOR_VERSION ([0-9]+)")
  set(_fluffos_flex_pin_major "${CMAKE_MATCH_1}")
  if ("${_fluffos_flex_pin_lines}" MATCHES "MINOR_VERSION ([0-9]+)")
    set(_fluffos_flex_pin_minor "${CMAKE_MATCH_1}")
    if ("${_fluffos_flex_pin_lines}" MATCHES "SUBMINOR_VERSION ([0-9]+)")
      set(FLUFFOS_PINNED_FLEX_VERSION
          "${_fluffos_flex_pin_major}.${_fluffos_flex_pin_minor}.${CMAKE_MATCH_1}")
    endif()
  endif()
endif()
set(FLUFFOS_FLEX_REGEN FALSE)
if (FLEX_FOUND AND NOT FLEX_VERSION VERSION_LESS FLUFFOS_PINNED_FLEX_VERSION)
  set(FLUFFOS_FLEX_REGEN TRUE)
endif()
if (FLUFFOS_FLEX_REGEN)
  message(STATUS "FLEX: ${FLEX_VERSION} (committed files pinned at ${FLUFFOS_PINNED_FLEX_VERSION})")
  FLEX_TARGET(LpcLexer
    "${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/lexer.l"
    "${CMAKE_CURRENT_BINARY_DIR}/lexer.autogen.cc"
    COMPILE_FLAGS "-Ca"
  )
  if (FLUFFOS_BISON_REGEN)
    # Only valid when BISON_TARGET(Grammar) actually ran -- on the
    # bison-too-old path (macOS's stock bison) the Grammar target doesn't
    # exist and newer CMake makes this a hard configure error; the header
    # dependency is covered by the pre-generated copy's DEPENDS there.
    ADD_FLEX_BISON_DEPENDENCY(LpcLexer Grammar)
  endif()
  set(GENERATED_LEXER_FILE "${FLEX_LpcLexer_OUTPUTS}")
else()
  if (FLEX_FOUND)
    message(STATUS "FLEX: ${FLEX_VERSION} is older than the pinned ${FLUFFOS_PINNED_FLEX_VERSION}, using pre-generated source.")
  else()
    message(STATUS "FLEX: not found, using pre-generated source.")
  endif()
  add_custom_command(
    OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/lexer.autogen.cc"
    COMMAND "${CMAKE_COMMAND}" -D ACTION=DENORMALIZE -D FILE_IN="${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/lexer.autogen.cc" -D FILE_OUT="${CMAKE_CURRENT_BINARY_DIR}/lexer.autogen.cc" -D REPO_ROOT="${CMAKE_SOURCE_DIR}" -D BUILD_ROOT="${CMAKE_BINARY_DIR}" -P "${CMAKE_SOURCE_DIR}/cmake/util.cmake"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/lexer.autogen.cc"
  )
  set(GENERATED_LEXER_FILE "${CMAKE_CURRENT_BINARY_DIR}/lexer.autogen.cc")
endif()
# ─────────────────────────────────────────────────────────────────────────

set(SRC
  "backend.cc"
  "comm.cc"
  "mainlib.cc"
  "user.cc"
  "symbol.cc"
  "ofile.cc"
  "net/msp.cc"
  "net/telnet.cc"
  "base/internal/debugmalloc.cc"
  "base/internal/external_port.cc"
  "base/internal/file.cc"
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
  "compiler/internal/grammar_rules_types.cc"
  "compiler/internal/grammar_rules_decls.cc"
  "compiler/internal/grammar_rules_loops.cc"
  "compiler/internal/grammar_rules_switch.cc"
  "compiler/internal/grammar_rules_exprs.cc"
  "compiler/internal/compiler.cc"
  "compiler/internal/compiler_utils.cc"
  "compiler/internal/disassembler.cc"
  "compiler/internal/generate.cc"
  "compiler/internal/icode.cc"
  "${GENERATED_LEXER_FILE}"          # Flex-generated lexer.autogen.cc
  "compiler/internal/lexer_utils.cc" # global state + helpers lexer.l calls into, plus predefines/include-path support
  "compiler/internal/lexer_rules.cc" # substantive logic behind lexer.l's rule actions (escape decoding, number parsing, etc.)
  "compiler/internal/lexer_rules_pp.cc" # preprocessing logic and expression parser
  "compiler/internal/stage_output.cc" # lpcc -E/--tokens staged dumps (lex+pp, no parse)
  "compiler/internal/scratchpad.cc"
  "compiler/internal/trees.cc"
  "vm/internal/base/apply_cache.cc"
  "vm/internal/base/array.cc"
  "vm/internal/base/buffer.cc"
  "vm/internal/base/class.cc"
  "vm/internal/base/debug.cc"
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

# Per-target implementations (link-time dispatch): the event loop, the
# connection transports, TLS and crash handling each have a native and a
# WASM implementation of the same interface.
if (EMSCRIPTEN)
  list(APPEND SRC
    "wasm/backend_wasm.cc"        # host-driven tick loop
    "wasm/comm_wasm.cc"           # JS byte transport (the page is the peer)
    "wasm/crash_handler_wasm.cc"  # traps reported by the JS host
    "debugger/debugger_stub.cc"   # no debugger transport in the browser (yet)
    )
else ()
  list(APPEND SRC
    "backend_libevent.cc"
    "net/transport_libevent.cc"
    "net/tls.cc"
    "net/websocket.cc"
    "net/ws_ascii.cc"
    "net/ws_telnet.cc"
    "base/internal/crash_handler.cc"
    "debugger/debug_server.cc"    # DAP-over-websocket LPC debugger
    "debugger/transport_lws.cc"
    "debugger/breakpoints.cc"
    "debugger/inspect.cc"
    )
endif ()

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
if (NOT CMAKE_CROSSCOMPILING)
  add_dependencies(autogen make_options_defs)
endif ()

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
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(USE_JEMALLOC OFF)
  message(STATUS "Debug build, force disabling JEMALLOC!")
endif ()
if (ENABLE_SANITIZER)
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
target_link_libraries(libdriver PUBLIC fmt::fmt)

list(APPEND FLUFFOS_LIBRARIES autogen)

add_subdirectory(packages) # provides FLUFFOS_PACKAGES
message(STATUS "ALL PACKAGES: ${FLUFFOS_PACKAGES}")
foreach (PACKAGE ${FLUFFOS_PACKAGES})
  # TODO: packages files currently depenends on generated headers, so they must be built after driver
  add_dependencies("${PACKAGE}" autogen)
  list(APPEND FLUFFOS_LIBRARIES "${PACKAGE}")
  target_link_libraries(libdriver PUBLIC "${PACKAGE}")
endforeach ()


if (NOT EMSCRIPTEN)
  target_link_libraries(libdriver PRIVATE event_core_static event_extra_static event_openssl_static)
  if (NOT WIN32)
    target_link_libraries(libdriver PRIVATE event_pthreads_static)
  endif()
  target_link_libraries(libdriver PUBLIC ${LIBWEBSOCKETS_LIBRARIES_STATIC})
endif ()
target_link_libraries(libdriver PRIVATE crypt)
target_link_libraries(libdriver PUBLIC libtelnet)
if (NOT EMSCRIPTEN)
  # The core uses zlib directly (see HAVE_ZLIB above) -- link it
  # explicitly instead of relying on the compress package or libtelnet
  # (both optional) to drag it in.
  target_link_libraries(libdriver PUBLIC ZLIB::ZLIB)
endif ()
if (NOT EMSCRIPTEN)
  target_link_libraries(libdriver PUBLIC ${BACKWARD_ENABLE})
  add_backward(libdriver)
endif ()

# System libraries
if (NOT EMSCRIPTEN)
  target_link_libraries(libdriver PUBLIC OpenSSL::SSL)
endif ()
target_link_libraries(libdriver PUBLIC ${ICU_LIBRARIES})

# Platform Libraries

# Winsocks
IF (WIN32)
  target_link_libraries(libdriver PUBLIC ws2_32)
ENDIF ()

if(NOT WIN32 AND NOT EMSCRIPTEN)
  find_package(Threads REQUIRED)
  target_link_libraries(libdriver PUBLIC Threads::Threads)
endif()

include(CheckLibraryExists)
CHECK_LIBRARY_EXISTS(rt clock_gettime "time.h" HAVE_CLOCK_GETTIME_IN_RT)
if (HAVE_CLOCK_GETTIME_IN_RT)
  target_link_libraries(libdriver PUBLIC rt)
endif ()

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

if (EMSCRIPTEN)
  # The web driver: a MODULARIZE'd JS+wasm pair. The page instantiates it
  # with createFluffOS(), mounts a mudlib into MEMFS, and drives the
  # exported fluffos_* entry points (see wasm/main_wasm.cc).
  add_executable(driver-web "wasm/main_wasm.cc")
  target_link_libraries(driver-web PUBLIC ${FLUFFOS_LINK})
  set_target_properties(driver-web PROPERTIES OUTPUT_NAME "fluffos" SUFFIX ".js")
  # TOTAL_STACK was renamed STACK_SIZE in emscripten 3.1.27.
  if (EMSCRIPTEN_VERSION VERSION_GREATER_EQUAL "3.1.27")
    set(FLUFFOS_WASM_STACK_FLAG "SHELL:-s STACK_SIZE=16MB")
  else ()
    set(FLUFFOS_WASM_STACK_FLAG "SHELL:-s TOTAL_STACK=16MB")
  endif ()
  target_link_options(driver-web PRIVATE
    "SHELL:-s MODULARIZE=1"
    "SHELL:-s EXPORT_NAME=createFluffOS"
    "SHELL:-s ALLOW_MEMORY_GROWTH=1"
    # Heap headroom for the compiler/VM; grows on demand (the ICU data
    # segment is small now that the archive is brkitr-filtered).
    "SHELL:-s INITIAL_MEMORY=64MB"
    "${FLUFFOS_WASM_STACK_FLAG}"
    "SHELL:-s FORCE_FILESYSTEM=1"
    "SHELL:-s EXPORTED_FUNCTIONS=_fluffos_boot,_fluffos_flag,_fluffos_tick,_fluffos_connect,_fluffos_input,_fluffos_disconnect,_fluffos_shutdown,_malloc,_free"
    "SHELL:-s EXPORTED_RUNTIME_METHODS=ccall,cwrap,FS,UTF8ToString,stringToUTF8,lengthBytesUTF8"
    "--no-entry"
    # binaryen's wasm-opt cannot update full DWARF at -O2/-O3 (it asserts in
    # wasm-debug.cpp, notably with wasm EH enabled), and RelWithDebInfo's -g
    # would feed it DWARF. Drop DWARF at link and keep the function-name
    # section instead -- readable browser/node stack traces, no crash, and a
    # much smaller fluffos.wasm.
    "-g0"
    "--profiling-funcs"
  )

  # lpcc for node: the compiler front-end (diagnostics + -E/--tokens/--ast/
  # -O0/--json stage outputs) as a plain node CLI program. NODERAWFS maps
  # the wasm FS straight onto the host filesystem, so
  # `node lpcc.js <config> <file>` behaves exactly like the native binary
  # -- this is the artifact the fluffos-vscode extension ships so its
  # Compiler Explorer and save-time diagnostics need no native build.
  add_executable(lpcc-web "main_lpcc.cc")
  target_link_libraries(lpcc-web PUBLIC ${FLUFFOS_LINK})
  set_target_properties(lpcc-web PROPERTIES OUTPUT_NAME "lpcc" SUFFIX ".js")
  target_link_options(lpcc-web PRIVATE
    "SHELL:-s ALLOW_MEMORY_GROWTH=1"
    "SHELL:-s INITIAL_MEMORY=64MB"
    "${FLUFFOS_WASM_STACK_FLAG}"
    "SHELL:-s NODERAWFS=1"
    "SHELL:-s ENVIRONMENT=node"
    "SHELL:-s EXIT_RUNTIME=1"
    # Same DWARF note as driver-web: keep names, drop debug sections.
    "-g0"
    "--profiling-funcs"
  )
else ()
  add_executable(driver "main.cc")
  target_link_libraries(driver PUBLIC ${FLUFFOS_LINK})

  add_executable(lpcc "main_lpcc.cc")
  target_link_libraries(lpcc PUBLIC ${FLUFFOS_LINK})

  add_executable(lpcshell "main_lpcshell.cc")
  target_link_libraries(lpcshell PUBLIC ${FLUFFOS_LINK})

  add_executable(symbol "main_symbol.cc")
  target_link_libraries(symbol PUBLIC ${FLUFFOS_LINK})

  add_executable(generate_keywords "main_generate_keywords.cc")
  target_link_libraries(generate_keywords PUBLIC ${FLUFFOS_LINK})

  add_executable(o2json "main_o2json.cc")
  target_link_libraries(o2json PUBLIC ${FLUFFOS_LINK})

  add_executable(json2o "main_json2o.cc")
  target_link_libraries(json2o PUBLIC ${FLUFFOS_LINK})
endif ()

include(helper)

message(STATUS "Final Compile Flags for libdriver: ")
print_iterface_target_properties(libdriver)

if (NOT EMSCRIPTEN)
message(STATUS "Final Compile Flags for driver: ")
print_target_properties(driver)

install(TARGETS ${GENERATED_DOCS}
        RUNTIME DESTINATION bin)

install(TARGETS driver lpcc lpcshell symbol o2json json2o
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
endif () # NOT EMSCRIPTEN

if (NOT WIN32 AND NOT EMSCRIPTEN)
  add_executable(portbind "portbind.cc")
  install(TARGETS portbind
    RUNTIME DESTINATION bin)
endif ()

# Update generated grammar files. The copy-back is gated twice: only hosts
# whose generator is at/above the committed pin regenerate at all, and
# util.cmake skips the write unless the .y/.l INPUT hash changed -- so
# same-version generators with cosmetically different output (distro
# patches) can never churn the committed files.
if (FLUFFOS_BISON_REGEN AND NOT EMSCRIPTEN)
  add_custom_command(TARGET "driver" POST_BUILD
          COMMAND "${CMAKE_COMMAND}" -D ACTION=NORMALIZE -D FILE_IN="${BISON_Grammar_OUTPUT_SOURCE}" -D FILE_OUT="${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/grammar.autogen.cc" -D REPO_ROOT="${CMAKE_SOURCE_DIR}" -D BUILD_ROOT="${CMAKE_BINARY_DIR}" -D SRC_FILE="${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/grammar.y" -P "${CMAKE_SOURCE_DIR}/cmake/util.cmake"
          COMMAND "${CMAKE_COMMAND}" -D ACTION=NORMALIZE -D FILE_IN="${BISON_Grammar_OUTPUT_HEADER}" -D FILE_OUT="${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/grammar.autogen.h" -D REPO_ROOT="${CMAKE_SOURCE_DIR}" -D BUILD_ROOT="${CMAKE_BINARY_DIR}" -D SRC_FILE="${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/grammar.y" -P "${CMAKE_SOURCE_DIR}/cmake/util.cmake"
          COMMENT "Copying and sanitizing generated grammar files")
endif()

if (FLUFFOS_FLEX_REGEN AND NOT EMSCRIPTEN)
  add_custom_command(TARGET "driver" POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -D ACTION=NORMALIZE -D FILE_IN="${CMAKE_CURRENT_BINARY_DIR}/lexer.autogen.cc" -D FILE_OUT="${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/lexer.autogen.cc" -D REPO_ROOT="${CMAKE_SOURCE_DIR}" -D BUILD_ROOT="${CMAKE_BINARY_DIR}" -D SRC_FILE="${CMAKE_CURRENT_SOURCE_DIR}/compiler/internal/lexer.l" -P "${CMAKE_SOURCE_DIR}/cmake/util.cmake"
    COMMENT "Copying and sanitizing Flex-generated lexer.autogen.cc")
endif()

find_package(GTest)
if(${GTEST_FOUND})
  include(GoogleTest)
  enable_testing()
  add_subdirectory(tests)
endif()

# Must keep this line at the end, so we get all the variables.
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
