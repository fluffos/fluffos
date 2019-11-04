cmake_minimum_required(VERSION 3.10)
project(libtelnet
    VERSION 0.30.0
    LANGUAGES C
)

set(PROJECT_DESCRIPTION "TELNET protocol handling library")
set(PROJECT_HOMEPAGE_URL https://github.com/seanmiddleditch/libtelne)

if (MSVC)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
    if (LIBTELNET_STRICT)
        add_compile_options(/WX)
    endif ()
else ()
    if (LIBTELNET_STRICT)
        add_compile_options(-Wall -Wextra -pedantic -Werror)
    endif ()
endif ()

add_library(libtelnet libtelnet.c)

target_include_directories(libtelnet
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(ZLIB)
if(${ZLIB_FOUND})
      target_compile_definitions(libtelnet PUBLIC HAVE_ZLIB)
      target_include_directories(libtelnet PRIVATE ${ZLIB_INCLUDE_DIRS})
      target_link_libraries(libtelnet PRIVATE ${ZLIB_LIBRARIES})
endif()

add_subdirectory(util)
add_subdirectory(doc)
add_subdirectory(test)

include(GNUInstallDirs)

configure_file(libtelnet.pc.in libtelnet.pc)

install(TARGETS libtelnet
    EXPORT libtelnet-export
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(FILES libtelnet.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(EXPORT libtelnet-export
    FILE libtelnet.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libtelnet
)

include(CPack)
