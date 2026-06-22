function(process_bison_file FILE_IN FILE_OUT REPO_ROOT ACTION)
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
  
  file(READ "${FILE_IN}" CONTENT)
  
  if (ACTION STREQUAL "NORMALIZE")
    # Replace absolute path with standard $REPO_ROOT$ token
    string(REPLACE "${REPO_ROOT_NORM}" "$REPO_ROOT$" CONTENT_SUB "${CONTENT}")
    # Bison mangles the build-dir path into the header guard (uppercased, slashes→underscores).
    # Replace it with a fixed token so the committed file is platform-independent.
    string(REGEX REPLACE
      "YY_YY_[A-Z0-9_]+_GRAMMAR_AUTOGEN_H_INCLUDED"
      "YY_YY_GRAMMAR_AUTOGEN_H_INCLUDED"
      CONTENT_SUB "${CONTENT_SUB}")
  elseif (ACTION STREQUAL "DENORMALIZE")
    # Replace standard $REPO_ROOT$ with actual absolute path of local repo
    string(REPLACE "$REPO_ROOT$" "${REPO_ROOT_NORM}" CONTENT_SUB "${CONTENT}")
  else()
    message(FATAL_ERROR "Invalid ACTION: ${ACTION}. Must be NORMALIZE or DENORMALIZE")
  endif()
  
  # Write back if content changed or if we are copying to a different location
  if (NOT "${CONTENT}" STREQUAL "${CONTENT_SUB}" OR NOT "${FILE_IN}" STREQUAL "${FILE_OUT}")
    file(WRITE "${FILE_OUT}" "${CONTENT_SUB}")
    if (ACTION STREQUAL "NORMALIZE")
      message(STATUS "Normalized and copied ${FILE_IN} -> ${FILE_OUT}")
    else()
      message(STATUS "Denormalized and copied ${FILE_IN} -> ${FILE_OUT}")
    endif()
  endif()
endfunction()

if (CMAKE_SCRIPT_MODE_FILE)
  if (NOT DEFINED ACTION OR NOT DEFINED FILE_IN OR NOT DEFINED FILE_OUT OR NOT DEFINED REPO_ROOT)
    message(FATAL_ERROR "Required variables: ACTION, FILE_IN, FILE_OUT, REPO_ROOT")
  endif()

  process_bison_file("${FILE_IN}" "${FILE_OUT}" "${REPO_ROOT}" "${ACTION}")
endif()
