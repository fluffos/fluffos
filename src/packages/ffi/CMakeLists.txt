if(${PACKAGE_FFI})
    add_library(package_ffi STATIC
            "ffi.cc"
            "ffi.h")

    # libffi provides the runtime call-frame builder (ffi_prep_cif /
    # ffi_call / ffi_closure). Found via pkg-config on every platform.
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FFI REQUIRED libffi)
    target_include_directories(package_ffi SYSTEM PUBLIC ${FFI_INCLUDE_DIRS})
    target_link_libraries(package_ffi PRIVATE ${FFI_LIBRARIES})
    target_link_directories(package_ffi PRIVATE ${FFI_LIBRARY_DIRS})

    # dlopen/dlsym: on most POSIX systems these live in libdl.
    if(NOT WIN32)
        target_link_libraries(package_ffi PRIVATE ${CMAKE_DL_LIBS})
    endif()
endif()
