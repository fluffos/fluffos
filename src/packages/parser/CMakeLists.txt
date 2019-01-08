option(PACKAGE_PARSER "parser package" ON)

if(${PACKAGE_PARSER})
    add_library(package_parser STATIC
            "parser.cc"
            "parser.h"
            )
endif()
