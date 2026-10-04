if(${PACKAGE_EXTERNAL})
  add_library(package_external STATIC
          "external.cc"
          "external.h"
          "socketpair.cc"
          )
  # external.cc hands its child-process pipes to the socket table in
  # sockets/socket_efuns.cc (find_new_socket, socket_close, ...). When the
  # sockets package is off that file is not built for it, so build the table
  # here; its efun halves are guarded by PACKAGE_SOCKETS and drop out.
  if(NOT ${PACKAGE_SOCKETS})
    target_sources(package_external PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/../sockets/socket_efuns.cc"
            "${CMAKE_CURRENT_SOURCE_DIR}/../sockets/socket_efuns.h")
  endif()
endif()
