if(${PACKAGE_PARSER})
    add_library(package_parser STATIC
            "parser.cc"
            "parser.h"
            )
endif()
