if(${PACKAGE_SHA1})
    add_library(package_sha1 STATIC
            "sha1.cc"
            )
endif()