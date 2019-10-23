if(${PACKAGE_MATRIX})
    add_library(package_matrix STATIC
            "matrix.cc"
            "matrix.h"
            )
endif()
