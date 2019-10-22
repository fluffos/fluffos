option(PACKAGE_ASYNC "async package" ON)

if(${PACKAGE_ASYNC})
    add_library(package_async STATIC
            "async.cc"
            "async.h")
    target_link_libraries(package_async PUBLIC package_db)
endif()
