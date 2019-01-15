option(MTUNE_NATIVE "mtune for GCC" ON)

add_library(common INTERFACE)

# Compile options
target_compile_options(common INTERFACE
        # General debuginfo
        "-g"
        "-DNDEBUG"
        "-fno-omit-frame-pointer"
        # We use GNU extensions
        "-D_GNU_SOURCE"
        # Enforce strict aliasing
        "-fstrict-aliasing"
        )

if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    target_compile_options(common INTERFACE
            "-O3"
            "-funroll-loops"
            "-fstack-protector-strong"
            )
else()
    target_compile_options(common INTERFACE
            "-Og"
            #"-DDEBUG"
            "-D_FORTIFY_SOURCE=2"
            "-fstack-protector-all"
            "-Wstack-protector"
            "--param" "ssp-buffer-size=4"
            # TODO: This doesn't work with CYGWIN and make_func
#            "-fsanitize=address,undefined"
            )
#    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS}
#            "-fsanitize=address,undefined"
#            )
endif()

# Warnings
target_compile_options(common INTERFACE
        "-Wall"
        "-Wextra"
        "-Wformat"
        "-Werror=format-security"
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
        "-funsigned-char"
        "-fwrapv")

# Optimization flags
if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  # MTUNE native
  if(MTUNE_NATIVE)
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-mtune=native" SUPPORT_MTUNE_NATIVE)
    if(SUPPORT_MTUNE)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=${MTUNE}")
    else()
      message(STATUS "The compiler dones't support -mtune=${MTUNE} ")
    endif()
  endif()
endif()

