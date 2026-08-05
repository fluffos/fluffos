if(${PACKAGE_DEBUGGER})
    add_library(package_debugger STATIC
            "debugger.cc"
            )
endif()
