set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(SOURCES
        "libtelnet.c"
        "libtelnet.h")

add_library(libtelnet STATIC ${SOURCES})
set_target_properties(libtelnet PROPERTIES LINKER_LANAGUGE C)

find_package(ZLIB REQUIRED)
target_compile_definitions(libtelnet PUBLIC HAVE_ZLIB)
target_include_directories(libtelnet PRIVATE ${ZLIB_INCLUDE_DIRS})
target_link_libraries(libtelnet PUBLIC ${ZLIB_LIBRARIES})
