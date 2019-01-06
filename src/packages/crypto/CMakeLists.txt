option(PACKAGE_CRYPTO "compress package" OFF)

if(${PACKAGE_CRYPTO})
    add_library(package_crypto STATIC
            "crypto.cc"
            )
    find_package (OPENSSL REQUIRED)
    target_link_libraries(package_crypto ${OPENSSL_LIBRARIES})
endif()
