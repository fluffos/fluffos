if(${PACKAGE_TRIM})
    add_library(package_trim STATIC
            "trim.cc"
            )
endif()