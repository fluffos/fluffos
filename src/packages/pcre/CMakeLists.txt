if(${PACKAGE_PCRE})
    add_library(package_pcre STATIC
            "pcre.cc"
            "pcre.h"
            )
    find_package (PCRE REQUIRED)
    if(STATIC)
      target_compile_definitions(package_pcre PUBLIC "PCRE_STATIC")
    endif()
    target_link_libraries(package_pcre ${PCRE_LIBRARIES})
    target_include_directories(package_pcre PUBLIC ${PCRE_INCLUDE_DIR})
endif()
