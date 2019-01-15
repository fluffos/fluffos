# PACKAGE CORE is always on
set(PACKAGE_CORE ON)

file(GLOB PACKAGE_CORE_SOURCE "*.cc")
file(GLOB PACKAGE_CORE_HEADER "*.h")

add_library(package_core STATIC
        ${PACKAGE_CORE_SOURCE}
        ${PACKAGE_CORE_HEADER}
        )

# CRYPT sometimes comes with libcrypt, somteimes comes with libc, so we only link when needed.
find_package(CRYPT)
target_link_libraries(package_core ${CRYPT_LIBRARIES})
