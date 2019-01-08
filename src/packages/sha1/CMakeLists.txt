option(PACKAGE_SHA1 "sha1 package" ON)

if(${PACKAGE_SHA1})
    add_library(package_sha1 STATIC
            "sha1.cc"
            )
endif()