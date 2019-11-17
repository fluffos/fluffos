file(GLOB PACKAGE_CORE_SOURCE "*.cc")
file(GLOB PACKAGE_CORE_HEADER "*.h")

add_library(package_core STATIC
        ${PACKAGE_CORE_SOURCE}
        ${PACKAGE_CORE_HEADER}
        )

# CRYPT sometimes comes with libcrypt, somteimes comes with libc, so we only link when needed.
find_package(CRYPT)
if(CRYPT_FOUND)
  target_include_directories(package_core SYSTEM PUBLIC ${CRYPT_INCLUDE_DIR})
  target_link_libraries(package_core PRIVATE ${CRYPT_LIBRARIES})
endif()
