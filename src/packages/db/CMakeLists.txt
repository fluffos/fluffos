option(PACKAGE_DB "db package" ON)

set(PACKAGE_DB_MYSQL "1")
set(PACKAGE_DB_POSTGRESQL OFF)
set(PACKAGE_DB_SQLITE OFF)
set(PACKAGE_DB_DEFAULT_DB ${PACKAGE_DB_MYSQL})

if(${PACKAGE_DB})
    add_library(package_db STATIC
            "db.cc"
            "db.h")
    if(${PACKAGE_DB_MYSQL})
        message(STATUS "PACKAGE_DB USE_MYSQL: ${PACKAGE_DB_MYSQL}")
        target_compile_definitions(package_db PUBLIC "USE_MYSQL=${PACKAGE_DB_MYSQL}")
        target_compile_definitions(package_db PUBLIC "DEFAULT_DB=${PACKAGE_DB_DEFAULT_DB}")

        if(STATIC)
          set(MYSQLCLIENT_STATIC_LINKING TRUE)
        endif()

        find_package(MySQL REQUIRED)
        target_include_directories(package_db PUBLIC ${MYSQL_INCLUDE_DIR})

        target_link_libraries(package_db PUBLIC ${MYSQL_LIBRARIES})
    endif()
    # Adding others
endif()
