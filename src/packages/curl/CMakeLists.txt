if(${PACKAGE_CURL})
    add_library(package_curl STATIC
            "curl.cc"
            )
    find_package (CURL REQUIRED)
    target_link_libraries(package_curl ${CURL_LIBRARIES})
    target_include_directories(package_curl SYSTEM PUBLIC ${CURL_INCLUDE_DIRS})
endif()
