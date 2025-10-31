# Distributed under the OSI-approved BSD 3-Clause License.
# See accompanying file LICENSE or https://cmake.org/licensing for details.

#[=======================================================================[.
FindNodejs
----------

Find Node.js and Yarn.

#]=======================================================================]

# Helper macro to ask the shell where an executable is.
macro(find_program_via_shell _var _name)
  if(NOT ${_var})
    message(STATUS "find_program_via_shell: Searching for '${_name}' using shell...")
    if(WIN32)
      # Native Windows (cmd, PowerShell)
      # Explicitly invoke PowerShell to use its path resolution
      execute_process(
        COMMAND powershell.exe -Command "(Get-Command -Type Application ${_name} -ErrorAction SilentlyContinue).Path | Select-Object -First 1"
        OUTPUT_VARIABLE _path
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE _res
      )
      # PowerShell returns 0 even if not found, so check _path content
      if(_res EQUAL 0 AND _path)
        # PowerShell output might have extra newlines, strip them
        string(REGEX REPLACE "\\r?\\n.*" "" _first_path "${_path}")
        if(EXISTS "${_first_path}")
          set(${_var} "${_first_path}" CACHE FILEPATH "Path to ${_name} (found via PowerShell)" FORCE)
          message(STATUS "find_program_via_shell: Found at ${_first_path} (via PowerShell)")
        else()
          message(WARNING "find_program_via_shell: PowerShell found path '${_first_path}' but it does not exist.")
        endif()
      else()
        message(WARNING "find_program_via_shell: PowerShell 'Get-Command' for '${_name}' failed or returned no output.")
      endif()
    else()
      # Linux, macOS, and MSYS/MinGW on Windows
      find_program(BASH_EXECUTABLE bash)
      if(BASH_EXECUTABLE)
        execute_process(
          COMMAND ${BASH_EXECUTABLE} -l -c "which ${_name}"
          OUTPUT_VARIABLE _path
          OUTPUT_STRIP_TRAILING_WHITESPACE
          RESULT_VARIABLE _res
        )
      endif()

      if(_res EQUAL 0 AND _path)
        string(REGEX REPLACE "\\r?\\n.*" "" _first_path "${_path}")
        if(EXISTS "${_first_path}")
          set(${_var} "${_first_path}" CACHE FILEPATH "Path to ${_name} (found via shell)" FORCE)
          message(STATUS "find_program_via_shell: Found at ${_first_path} (via Bash)")
        else()
          message(WARNING "find_program_via_shell: Bash found path '${_first_path}' but it does not exist.")
        endif()
      else()
        message(WARNING "find_program_via_shell: Bash 'which' for '${_name}' failed or returned no output.")
      endif()
    endif()
  endif()
endmacro()

# --- Find Node.js ---
message(STATUS "Searching for Node.js...")
find_program(NODEJS_EXECUTABLE NAMES nodejs node)
if(NOT NODEJS_EXECUTABLE)
    message(STATUS "Node.js not found via find_program. Falling back to shell search.")
    find_program_via_shell(NODEJS_EXECUTABLE node)
endif()

# --- Find Yarn ---
message(STATUS "Searching for Yarn...")
find_program(YARN_EXECUTABLE NAMES yarn.cmd yarn)
if(NOT YARN_EXECUTABLE)
    message(STATUS "Yarn not found via find_program. Falling back to shell search.")
    find_program_via_shell(YARN_EXECUTABLE yarn)
endif()


if(NODEJS_EXECUTABLE)
  execute_process(COMMAND ${NODEJS_EXECUTABLE} --version OUTPUT_VARIABLE NODEJS_VERSION_STRING ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(NODEJS_VERSION_STRING MATCHES "^v([0-9]+.[0-9]+.[0-9]+)")
    set(NODEJS_VERSION_STRING "${CMAKE_MATCH_1}")
  endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Nodejs
  FOUND_VAR Nodejs_FOUND
  REQUIRED_VARS NODEJS_EXECUTABLE YARN_EXECUTABLE
  VERSION_VAR NODEJS_VERSION_STRING
)

if(Nodejs_FOUND)
  message(STATUS "Successfully found Node.js: ${NODEJS_EXECUTABLE}")
  message(STATUS "Successfully found Yarn: ${YARN_EXECUTABLE}")
  if(NOT TARGET Nodejs::Nodejs)
    add_executable(Nodejs::Nodejs IMPORTED GLOBAL)
    set_target_properties(Nodejs::Nodejs PROPERTIES IMPORTED_LOCATION "${NODEJS_EXECUTABLE}")
  endif()
  if(NOT TARGET Nodejs::Yarn)
    add_executable(Nodejs::Yarn IMPORTED GLOBAL)
    set_target_properties(Nodejs::Yarn PROPERTIES IMPORTED_LOCATION "${YARN_EXECUTABLE}")
  endif()
else()
    message(FATAL_ERROR "Could NOT find Node.js or Yarn. Please ensure they are in your PATH and that your shell startup scripts (e.g., .bash_profile) are configured correctly.")
endif()

mark_as_advanced(NODEJS_EXECUTABLE YARN_EXECUTABLE)
