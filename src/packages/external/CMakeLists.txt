if(${PACKAGE_EXTERNAL})
  add_library(package_external STATIC
          "external.cc"
          )
endif()
