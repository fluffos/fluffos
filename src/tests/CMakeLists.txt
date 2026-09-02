# GTEST
find_package(GTest)

if(${GTEST_FOUND})
  include(GoogleTest)
  enable_testing()

  # Which GTest to link. Everywhere except Windows this is just the
  # imported targets find_package() gave us.
  #
  # Windows/MSYS2 needs care, because the allocator has to MATCH the
  # driver's. USE_JEMALLOC links libjemalloc.a statically into each
  # executable, where it replaces malloc/free for everything linked INTO
  # that binary -- but only for what is linked in. MSYS2's
  # GTestConfig.cmake declares its imported targets SHARED, pointing at
  # libgtest.dll, and a DLL binds its own CRT at its own link time. So
  # gtest.dll allocates and frees on msvcrt's heap while the exe around
  # it uses jemalloc's, and any heap-allocated object handed across that
  # boundary is freed by the wrong allocator. SCOPED_TRACE does exactly
  # that -- it builds a Message/std::string in our code and passes it to
  # gtest.dll's ScopedTrace, which destroys it -- and the NT heap reports
  # the resulting cross-heap free as STATUS_HEAP_CORRUPTION, exit
  # 0xc0000374. That is issue #1345: only the RelWithDebInfo job failed,
  # because src/CMakeLists.txt force-disables jemalloc for Debug and for
  # sanitizer builds, and only one test used SCOPED_TRACE.
  #
  # The MSYS2 package does ship static archives (libgtest.a,
  # libgtest_main.a); its CMake config just never exposes them. Link
  # those directly so gtest's code lands inside the executable and binds
  # to the same jemalloc as the driver -- one heap, no mismatch. Note
  # this must NOT define GTEST_LINKED_AS_SHARED_LIBRARY, which the SHARED
  # imported targets add via INTERFACE_COMPILE_DEFINITIONS; bypassing the
  # targets drops that define with them, which is what a static link
  # wants.
  set(FLUFFOS_GTEST_LIBS GTest::GTest GTest::Main)
  if(WIN32)
    # find_library() on MinGW prefers the .dll.a import lib, so restrict
    # the suffix list. "libgtest.dll.a" cannot match NAMES gtest under a
    # bare ".a" suffix (that would need the name "gtest.dll"), so this
    # selects the real static archive.
    set(_fluffos_saved_lib_suffixes "${CMAKE_FIND_LIBRARY_SUFFIXES}")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    find_library(FLUFFOS_GTEST_STATIC NAMES gtest)
    find_library(FLUFFOS_GTEST_MAIN_STATIC NAMES gtest_main)
    set(CMAKE_FIND_LIBRARY_SUFFIXES "${_fluffos_saved_lib_suffixes}")
    find_path(FLUFFOS_GTEST_INCLUDE_DIR NAMES gtest/gtest.h)

    if(FLUFFOS_GTEST_STATIC AND FLUFFOS_GTEST_MAIN_STATIC AND FLUFFOS_GTEST_INCLUDE_DIR)
      add_library(fluffos_gtest_static INTERFACE)
      # gtest_main must precede gtest for a static link.
      target_link_libraries(fluffos_gtest_static
                            INTERFACE "${FLUFFOS_GTEST_MAIN_STATIC}" "${FLUFFOS_GTEST_STATIC}")
      if(TARGET Threads::Threads)
        target_link_libraries(fluffos_gtest_static INTERFACE Threads::Threads)
      endif()
      target_include_directories(fluffos_gtest_static SYSTEM
                                 INTERFACE "${FLUFFOS_GTEST_INCLUDE_DIR}")
      set(FLUFFOS_GTEST_LIBS fluffos_gtest_static)
      message(STATUS "GTest: linking static archives (${FLUFFOS_GTEST_STATIC}) "
                     "so gtest shares the driver's allocator -- see issue #1345")
    else()
      message(WARNING
              "GTest: static archives not found; falling back to the shared imported "
              "targets. On a USE_JEMALLOC build that mismatches gtest.dll's msvcrt heap "
              "against the executable's jemalloc heap and can crash with "
              "STATUS_HEAP_CORRUPTION (0xc0000374) -- see issue #1345.")
    endif()
  endif()

  add_executable(lpc_tests test_lpc.cc)
  target_link_libraries(lpc_tests PRIVATE ${FLUFFOS_LINK} ${FLUFFOS_GTEST_LIBS})
  target_compile_definitions(lpc_tests PRIVATE -DTESTSUITE_DIR="${CMAKE_SOURCE_DIR}/testsuite")

  add_executable(ofile_tests test_ofile.cc)
  target_link_libraries(ofile_tests PRIVATE ${FLUFFOS_LINK} ${FLUFFOS_GTEST_LIBS})
  target_compile_definitions(ofile_tests PRIVATE -DTESTSUITE_DIR="${CMAKE_SOURCE_DIR}/testsuite")

  add_executable(compiler_tests test_compiler.cc)
  target_link_libraries(compiler_tests PRIVATE ${FLUFFOS_LINK} ${FLUFFOS_GTEST_LIBS})
  target_compile_definitions(compiler_tests PRIVATE -DTESTSUITE_DIR="${CMAKE_SOURCE_DIR}/testsuite")

  add_executable(lexer_tests test_lexer.cc)
  target_link_libraries(lexer_tests PRIVATE ${FLUFFOS_LINK} ${FLUFFOS_GTEST_LIBS})
  target_compile_definitions(lexer_tests PRIVATE -DTESTSUITE_DIR="${CMAKE_SOURCE_DIR}/testsuite")

  add_executable(strutils_tests test_strutils.cc)
  target_link_libraries(strutils_tests PRIVATE ${FLUFFOS_LINK} ${FLUFFOS_GTEST_LIBS})

  # Scratchpad-vs-malloc micro-benchmark: proves the compile arena beats
  # heap allocation for the compiler's allocation patterns. Timing-based,
  # so it is NOT registered with ctest -- run manually (RelWithDebInfo):
  #   ./src/tests/bench_scratchpad [compiles-per-workload]
  add_executable(bench_scratchpad bench_scratchpad.cc)
  target_link_libraries(bench_scratchpad PRIVATE ${FLUFFOS_LINK})

  # Whole-compile throughput across many rounds (real compile_file on a
  # representative program; proves flat steady-state + zero chunk mallocs
  # after warmup). Manual, RelWithDebInfo:
  #   ./src/tests/bench_compile [rounds]
  add_executable(bench_compile bench_compile.cc)
  target_link_libraries(bench_compile PRIVATE ${FLUFFOS_LINK})
  target_compile_definitions(bench_compile PRIVATE -DTESTSUITE_DIR="${CMAKE_SOURCE_DIR}/testsuite")

  gtest_discover_tests(lpc_tests DISCOVERY_TIMEOUT 60)
  gtest_discover_tests(ofile_tests DISCOVERY_TIMEOUT 60)
  gtest_discover_tests(compiler_tests DISCOVERY_TIMEOUT 60)
  gtest_discover_tests(lexer_tests DISCOVERY_TIMEOUT 60)
  gtest_discover_tests(strutils_tests DISCOVERY_TIMEOUT 60)
endif()

