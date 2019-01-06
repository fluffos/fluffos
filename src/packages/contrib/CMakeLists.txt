option(PACKAGE_CONTRIB "compress package" ON)

if(${PACKAGE_CONTRIB})
    add_library(package_contrib STATIC
            "contrib.cc"
            )
endif()
