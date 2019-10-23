if(${PACKAGE_OPS})
    add_library(package_ops STATIC
            "ops.cc"
            "ops.h"
            "parse.cc"
            "parse.h"
            )
endif()
