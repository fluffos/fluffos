if(${PACKAGE_ASYNC})
    add_library(package_async STATIC
            "async.cc"
            "async.h")
    if(${PACKAGE_DB})
        # to get the compile flags
        target_link_libraries(package_async PUBLIC package_db)
    endif()
endif()
