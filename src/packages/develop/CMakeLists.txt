option(PACKAGE_DEVELOP "compress package" ON)

if(${PACKAGE_DEVELOP})
    add_library(package_develop STATIC
            "develop.cc"
            "checkmemory.cc"
            "disassembler.cc"
            )
endif()
