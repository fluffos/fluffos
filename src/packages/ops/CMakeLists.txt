# Package OPS is always on
set(PACKAGE_OPS ON)

if(${PACKAGE_OPS})
    add_library(package_ops STATIC
            "ops.cc"
            "ops.h"
            "parse.cc"
            "parse.h"
            )
endif()
