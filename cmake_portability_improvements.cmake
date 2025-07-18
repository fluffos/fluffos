# FluffOS CMake Portability Improvements
# This file contains proposed improvements to make the build system more portable

# 1. Portable Architecture-Specific Optimizations
function(add_architecture_optimizations)
    if(MARCH_NATIVE)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i386|i686")
            if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
                add_compile_options("-march=native" "-mtune=native")
            elseif(MSVC)
                add_compile_options("/arch:AVX2")  # or /arch:AVX depending on requirements
            endif()
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|ARM")
            if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
                add_compile_options("-mcpu=native")
            endif()
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
            if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
                add_compile_options("-mcpu=native")
            endif()
        endif()
    endif()
endfunction()

# 2. Portable Compiler Flags
function(add_portable_compiler_flags)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${PROJECT_NAME} PRIVATE
            "-fno-omit-frame-pointer"
            "-fno-strict-aliasing"
            "-Wall"
            "-Wextra"
            "-Wformat"
            "-Werror=format-security"
            "-fdiagnostics-show-option"
            "-fmessage-length=0"
            "-fsigned-char"
            "-fwrapv"
        )
        
        if(UNIX)
            target_compile_definitions(${PROJECT_NAME} PRIVATE "_GNU_SOURCE")
        endif()
        
    elseif(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE
            "/W3"           # Warning level 3
            "/permissive-"  # Strict conformance
            "/Zc:__cplusplus"  # Correct __cplusplus value
            "/utf-8"        # Source and execution character sets are UTF-8
        )
        
        target_compile_definitions(${PROJECT_NAME} PRIVATE
            "_CRT_SECURE_NO_WARNINGS"
            "_CRT_NONSTDC_NO_DEPRECATE"
            "NOMINMAX"
            "_WIN32_WINNT=0x0601"  # Windows 7
            "WINVER=0x0601"
        )
    endif()
endfunction()

# 3. Portable Link Flags
function(setup_portable_linking target)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(LINK_WHOLE_ARCHIVE "-Wl,--whole-archive")
        set(LINK_NO_WHOLE_ARCHIVE "-Wl,--no-whole-archive")
        if(APPLE)
            set(LINK_WHOLE_ARCHIVE "-Wl,-force_load")
            set(LINK_NO_WHOLE_ARCHIVE "")
        endif()
    elseif(MSVC)
        set(LINK_WHOLE_ARCHIVE "/WHOLEARCHIVE:")
        set(LINK_NO_WHOLE_ARCHIVE "")
    endif()
    
    set(LINK_WHOLE_ARCHIVE ${LINK_WHOLE_ARCHIVE} PARENT_SCOPE)
    set(LINK_NO_WHOLE_ARCHIVE ${LINK_NO_WHOLE_ARCHIVE} PARENT_SCOPE)
endfunction()

# 4. Portable Static/Dynamic Linking
function(setup_portable_static_linking)
    if(STATIC)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            if(NOT APPLE)  # Apple doesn't support static linking well
                set(CMAKE_EXE_LINKER_FLAGS 
                    "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++"
                    PARENT_SCOPE)
            endif()
        elseif(MSVC)
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
            set(CMAKE_EXE_LINKER_FLAGS 
                "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL:NO /LTCG"
                PARENT_SCOPE)
        endif()
    endif()
endfunction()

# 5. Portable Sanitizer Support
function(add_portable_sanitizer_support target)
    if(ENABLE_SANITIZER)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${target} PRIVATE
                "-fsanitize=address"
                "-fno-common"
            )
            target_link_options(${target} PRIVATE
                "-fsanitize=address"
            )
        elseif(MSVC)
            target_compile_options(${target} PRIVATE
                "/fsanitize=address"
            )
        endif()
    endif()
endfunction()

# 6. Improved Package Linkage
function(link_package_properly target package)
    # Most packages should be PRIVATE unless they expose public API
    set(LINKAGE_TYPE PRIVATE)
    
    # Only these packages need PUBLIC linkage because they expose public headers
    set(PUBLIC_PACKAGES "package_core" "package_ops")
    
    if(package IN_LIST PUBLIC_PACKAGES)
        set(LINKAGE_TYPE PUBLIC)
    endif()
    
    target_link_libraries(${target} ${LINKAGE_TYPE} ${package})
endfunction()

# 7. Portable Version Detection (alternative to bash/git)
function(get_portable_version)
    find_package(Git QUIET)
    if(Git_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --always
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(GIT_VERSION)
            set(PROJECT_VERSION ${GIT_VERSION} PARENT_SCOPE)
        else()
            set(PROJECT_VERSION "unknown" PARENT_SCOPE)
        endif()
    else()
        set(PROJECT_VERSION "unknown" PARENT_SCOPE)
    endif()
endfunction()

# 8. Portable Build Type Setup
function(setup_portable_build_types)
    # Set portable debug/release flags
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3" PARENT_SCOPE)
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -g" PARENT_SCOPE)
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG" PARENT_SCOPE)
    elseif(MSVC)
        set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /RTC1" PARENT_SCOPE)
        set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG" PARENT_SCOPE)
        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Zi /DNDEBUG" PARENT_SCOPE)
    endif()
endfunction()

# Usage example:
# include(cmake_portability_improvements.cmake)
# add_architecture_optimizations()
# add_portable_compiler_flags()
# setup_portable_linking(${TARGET_NAME})
# setup_portable_static_linking()
# add_portable_sanitizer_support(${TARGET_NAME})
# get_portable_version()
# setup_portable_build_types()
