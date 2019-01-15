option(PACKAGE_CRYPTO "crypto package" ON)

if(${PACKAGE_CRYPTO})
    add_library(package_crypto STATIC
            "crypto.cc"
            )
    find_package (OpenSSL REQUIRED)
    target_link_libraries(package_crypto ${OPENSSL_LIBRARIES})
endif()
