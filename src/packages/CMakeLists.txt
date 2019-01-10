# Find and include all packages.
file(GLOB PACKAGES LIST_DIRECTORIES true
        RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
        "*"
        )

set(FLUFFOS_PACKAGES "")

foreach(PACKAGE ${PACKAGES})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${PACKAGE}/CMakeLists.txt")
        add_subdirectory(${PACKAGE})
        if(TARGET "package_${PACKAGE}")
            message(STATUS "PACKAGE: ${PACKAGE} ON")
            list(APPEND FLUFFOS_PACKAGES "package_${PACKAGE}")
        else()
            message(STATUS "PACKAGE: ${PACKAGE} OFF")
        endif()
    endif()
endforeach()

set(PACKAGE_H_SOURCE
        "#ifndef PACKAGES_PACKAGES_H\n"
        "#define PACKAGES_PACKAGES_H\n"
        "\n"
)
foreach(PACKAGE ${FLUFFOS_PACKAGES})
    string(TOUPPER "${PACKAGE}" PACKAGE)
    list(APPEND PACKAGE_H_SOURCE
            "#define ${PACKAGE}\n"
            )
endforeach()

list(APPEND PACKAGE_H_SOURCE
        "\n"
        "#endif /* PACKAGES_PACKAGES_H */ \n"
        )

# Generate packages.h
set(PACKAGE_H "packages.autogen.h")

file(GENERATE OUTPUT "${PACKAGE_H}"
        CONTENT "$<JOIN:${PACKAGE_H_SOURCE},>"
        )

set(FLUFFOS_PACKAGES ${FLUFFOS_PACKAGES} PARENT_SCOPE)