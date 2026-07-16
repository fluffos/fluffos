if(${PACKAGE_CONTRIB})
    add_library(package_contrib STATIC
            "contrib.cc"
            "cycles.cc"
            )
endif()
