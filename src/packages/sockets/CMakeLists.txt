option(PACKAGE_SOCKETS "sockets package" ON)

if(${PACKAGE_SOCKETS})
    add_library(package_sockets STATIC
            "socket_efuns.cc"
            "socket_efuns.h"
            "sockets.cc"
            )
endif()