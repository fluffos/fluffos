option(PACKAGE_COMPRESS "compress package" ON)

set("SAVE_GZ_EXTENSION" ".o.gz")

if(${PACKAGE_COMPRESS})
    add_library(package_compress STATIC
            "compress.cc"
            )
    find_package (ZLIB REQUIRED)
    target_link_libraries(package_compress ${ZLIB_LIBRARIES})
    target_compile_definitions(package_compress PUBLIC SAVE_GZ_EXTENSION="${SAVE_GZ_EXTENSION}")
endif()