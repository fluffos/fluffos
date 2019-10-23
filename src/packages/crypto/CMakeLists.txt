if(${PACKAGE_CRYPTO})
    add_library(package_crypto STATIC
            "crypto.cc"
            )
    find_package (OpenSSL REQUIRED)
    target_link_libraries(package_crypto ${OPENSSL_LIBRARIES})
    target_include_directories(package_crypto SYSTEM PUBLIC ${OPENSSL_INCLUDE_DIR})
endif()
