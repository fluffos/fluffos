if(${PACKAGE_SOCKETS})
    add_library(package_sockets STATIC
            "socket_efuns.cc"
            "socket_efuns.h"
            "sockets.cc"
            )
endif()
