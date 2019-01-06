option(PACKAGE_PCRE "pcre package" ON)

if(${PACKAGE_PCRE})
    add_library(package_pcre STATIC
            "pcre.cc"
            "pcre.h"
            )
    find_package (PCRE REQUIRED)
    target_link_libraries(package_pcre ${PCRE_LIBRARIES})
endif()