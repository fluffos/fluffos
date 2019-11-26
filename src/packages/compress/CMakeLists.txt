if(${PACKAGE_COMPRESS})
    add_library(package_compress STATIC
            "compress.cc"
            )
    find_package (ZLIB REQUIRED)
    target_link_libraries(package_compress ${ZLIB_LIBRARIES})
endif()
