function(process_bison_file FILE_IN FILE_OUT REPO_ROOT ACTION BUILD_ROOT SRC_FILE)
  if (NOT EXISTS "${FILE_IN}")
    message(WARNING "File does not exist: ${FILE_IN}")
    return()
  endif()

  # Resolve REPO_ROOT to an absolute path first
  get_filename_component(REPO_ROOT_ABS "${REPO_ROOT}" ABSOLUTE)
  string(REPLACE "\\" "/" REPO_ROOT_NORM "${REPO_ROOT_ABS}")
  if (REPO_ROOT_NORM MATCHES "/$")
    string(REGEX REPLACE "/$" "" REPO_ROOT_NORM "${REPO_ROOT_NORM}")
  endif()

  # Resolve the build root too: it must be tokenized separately (and
  # first, since it usually nests inside the repo root) so the committed
  # file does not depend on the build directory's name or location.
  set(BUILD_ROOT_NORM "")
  if (BUILD_ROOT)
    get_filename_component(BUILD_ROOT_ABS "${BUILD_ROOT}" ABSOLUTE)
    string(REPLACE "\\" "/" BUILD_ROOT_NORM "${BUILD_ROOT_ABS}")
    if (BUILD_ROOT_NORM MATCHES "/$")
      string(REGEX REPLACE "/$" "" BUILD_ROOT_NORM "${BUILD_ROOT_NORM}")
    endif()
  endif()

  file(READ "${FILE_IN}" CONTENT)

  if (ACTION STREQUAL "NORMALIZE")
    # When the generator input (.y/.l) is known, gate the copy-back on it:
    # if the committed file already records the same input hash, the
    # committed file is up to date with respect to the grammar/lexer
    # SOURCE, and regeneration differences (bison/flex build variations
    # between hosts that report the same version) must NOT churn it.
    if (SRC_FILE AND EXISTS "${SRC_FILE}" AND EXISTS "${FILE_OUT}")
      file(SHA256 "${SRC_FILE}" SRC_HASH)
      file(READ "${FILE_OUT}" EXISTING_OUT)
      string(REGEX MATCH "FluffOS generated-from [^ ]+ sha256=([0-9a-f]+)" _stamp "${EXISTING_OUT}")
      if ("${CMAKE_MATCH_1}" STREQUAL "${SRC_HASH}")
        return()
      endif()
    endif()

    # Replace absolute paths with standard tokens (build root first).
    set(CONTENT_SUB "${CONTENT}")
    if (BUILD_ROOT_NORM)
      string(REPLACE "${BUILD_ROOT_NORM}" "$BUILD_ROOT$" CONTENT_SUB "${CONTENT_SUB}")
    endif()
    string(REPLACE "${REPO_ROOT_NORM}" "$REPO_ROOT$" CONTENT_SUB "${CONTENT_SUB}")
    # Bison mangles the build-dir path into the header guard (uppercased, slashes→underscores).
    # Replace it with a fixed token so the committed file is platform-independent.
    string(REGEX REPLACE
      "YY_YY_[A-Z0-9_]+_GRAMMAR_AUTOGEN_H_INCLUDED"
      "YY_YY_GRAMMAR_AUTOGEN_H_INCLUDED"
      CONTENT_SUB "${CONTENT_SUB}")
    # Record which input produced this file so later builds can tell "the
    # grammar/lexer changed" apart from "a different generator build
    # produced cosmetically different output".
    if (SRC_FILE AND EXISTS "${SRC_FILE}")
      file(SHA256 "${SRC_FILE}" SRC_HASH)
      get_filename_component(SRC_NAME "${SRC_FILE}" NAME)
      string(APPEND CONTENT_SUB "/* FluffOS generated-from ${SRC_NAME} sha256=${SRC_HASH} */\n")
    endif()
  elseif (ACTION STREQUAL "DENORMALIZE")
    # Replace standard tokens with the actual absolute local paths
    set(CONTENT_SUB "${CONTENT}")
    if (BUILD_ROOT_NORM)
      string(REPLACE "$BUILD_ROOT$" "${BUILD_ROOT_NORM}" CONTENT_SUB "${CONTENT_SUB}")
    endif()
    string(REPLACE "$REPO_ROOT$" "${REPO_ROOT_NORM}" CONTENT_SUB "${CONTENT_SUB}")
  else()
    message(FATAL_ERROR "Invalid ACTION: ${ACTION}. Must be NORMALIZE or DENORMALIZE")
  endif()

  # Skip the write when the destination already has identical content --
  # rewriting identical bytes still bumps the mtime and cascades a
  # pointless rebuild/relink of everything downstream on the next build.
  if (EXISTS "${FILE_OUT}")
    file(READ "${FILE_OUT}" EXISTING_CONTENT)
    if ("${EXISTING_CONTENT}" STREQUAL "${CONTENT_SUB}")
      return()
    endif()
  endif()

  file(WRITE "${FILE_OUT}" "${CONTENT_SUB}")
  if (ACTION STREQUAL "NORMALIZE")
    message(STATUS "Normalized and copied ${FILE_IN} -> ${FILE_OUT}")
  else()
    message(STATUS "Denormalized and copied ${FILE_IN} -> ${FILE_OUT}")
  endif()
endfunction()

if (CMAKE_SCRIPT_MODE_FILE)
  if (NOT DEFINED ACTION OR NOT DEFINED FILE_IN OR NOT DEFINED FILE_OUT OR NOT DEFINED REPO_ROOT)
    message(FATAL_ERROR "Required variables: ACTION, FILE_IN, FILE_OUT, REPO_ROOT")
  endif()
  if (NOT DEFINED BUILD_ROOT)
    set(BUILD_ROOT "")
  endif()
  if (NOT DEFINED SRC_FILE)
    set(SRC_FILE "")
  endif()

  process_bison_file("${FILE_IN}" "${FILE_OUT}" "${REPO_ROOT}" "${ACTION}" "${BUILD_ROOT}" "${SRC_FILE}")
endif()
