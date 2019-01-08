option(PACKAGE_MATH "math package" ON)

if(${PACKAGE_MATH})
    add_library(package_math STATIC
            "math.cc"
            )
endif()
