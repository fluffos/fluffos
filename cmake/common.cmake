add_library(common INTERFACE)

# include directories
target_include_directories(common INTERFACE ".")

# Compiler Features
target_compile_features(common INTERFACE cxx_std_11)

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

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(common INTERFACE
            "-O3"
            "-funroll-loops>"
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
            # TODO: This doesn't work with CYGWIN
            "-fsanitize=address,undefined"
            )
    set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS}
            "-fsanitize=address,undefined"
            )
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
        "-Wno-return-type"
        "-Wno-unused-parameter"
        "-Wno-missing-field-initializers"
        # GCC's warning produces too many false positives:
        # https://groups.google.com/a/google.com/forum/#!topic/c-users/jLRfP804wuc
        # https://groups.google.com/a/google.com/d/msg/c-users/jLRfP804wuc/sKe4Ba2PKwAJ
        "-Wno-overloaded-virtual"
        # GCC's warning produces false positives:
        # https://groups.google.com/a/google.com/d/msg/llvm-team/Mq_7JNPqE2o/lh2XpArujl8J
        "-Wno-non-virtual-dtor"
        # features
        "-fdiagnostics-show-option"
        "-fmessage-length=0"
        # Less undefined behavior
        "-funsigned-char"
        "-fwrapv")