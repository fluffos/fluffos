option(PACKAGE_MATRIX "matrix package" ON)

if(${PACKAGE_MATRIX})
    add_library(package_matrix STATIC
            "matrix.cc"
            "matrix.h"
            )
endif()
