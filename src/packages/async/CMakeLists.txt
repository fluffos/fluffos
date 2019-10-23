if(${PACKAGE_ASYNC})
    add_library(package_async STATIC
            "async.cc"
            "async.h")
    if(${PACKAGE_DB})
        add_dependencies(package_async package_db)
    endif()
endif()
