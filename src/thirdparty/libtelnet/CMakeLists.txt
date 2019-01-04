PROJECT(libtelnet C)

set(SOURCES
        "libtelnet.c"
        "libtelnet.h")

add_library(libtelnet STATIC ${SOURCES})