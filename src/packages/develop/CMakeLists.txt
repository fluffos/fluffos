if(${PACKAGE_DEVELOP})
    add_library(package_develop STATIC
            "develop.cc"
            "checkmemory.cc"
            "disassembler.cc"
            )
    if(PACKAGE_PCRE)
      target_link_libraries(package_develop PRIVATE package_pcre)
    endif()
endif()
