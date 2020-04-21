if(${PACKAGE_DB})
    add_library(package_db STATIC
            "db.cc"
            "db.h")

    if (NOT "${PACKAGE_DB_DEFAULT_DB}" STREQUAL "${PACKAGE_DB_MYSQL}" AND
            NOT "${PACKAGE_DB_DEFAULT_DB}" STREQUAL "${PACKAGE_DB_SQLITE}" AND
            NOT "${PACKAGE_DB_DEFAULT_DB}" STREQUAL "${PACKAGE_DB_POSTGRESQL}"
            )
      message(FATAL "PACKAGE_DB_DEFAULT_DB is not valid!")
    endif()

    target_compile_definitions(package_db PUBLIC "DEFAULT_DB=${PACKAGE_DB_DEFAULT_DB}")

    if(NOT "${PACKAGE_DB_MYSQL}" STREQUAL "")
        if ("${PACKAGE_DB_MYSQL}" STREQUAL "{PACKAGE_DB_SQLITE}" OR
            "${PACKAGE_DB_MYSQL}" STREQUAL "{PACKAGE_DB_POSTGRESQL}")
          message(FATAL "Conflicting DB type!")
        endif()

        message(STATUS "PACKAGE_DB USE_MYSQL: ${PACKAGE_DB_MYSQL}")
        target_compile_options(package_db PUBLIC "${MYSQL_CXXFLAGS}")
        target_compile_definitions(package_db PUBLIC "USE_MYSQL=${PACKAGE_DB_MYSQL}")

        if(STATIC)
          set(MYSQLCLIENT_STATIC_LINKING TRUE)
        endif()

        find_package(MySQL REQUIRED)
        target_include_directories(package_db SYSTEM PUBLIC ${MYSQL_INCLUDE_DIR})

        target_link_libraries(package_db PRIVATE ${MYSQL_LINK_FLAGS})
        target_link_libraries(package_db PRIVATE -L${MYSQL_LIB_DIR} ${MYSQL_LIBRARIES})
    endif()

    if(NOT "${PACKAGE_DB_SQLITE}" STREQUAL "")
      if ("${PACKAGE_DB_SQLITE}" STREQUAL "{PACKAGE_DB_MYSQL}" OR
              "${PACKAGE_DB_SQLITE}" STREQUAL "{PACKAGE_DB_POSTGRESQL}")
        message(FATAL "Conflicting DB type!")
      endif()

      find_package(SQLite3 REQUIRED)
      message(STATUS "PACKAGE_DB USE_SQLITE3: ${PACKAGE_DB_SQLITE}, Found version ${SQLite3_VERSION}")

      target_compile_definitions(package_db PUBLIC "USE_SQLITE3=${PACKAGE_DB_SQLITE}")
      target_include_directories(package_db SYSTEM PUBLIC ${SQLite3_INCLUDE_DIRS})
      target_link_libraries(package_db PRIVATE ${SQLite3_LIBRARIES})
    endif()

    if(NOT "${PACKAGE_DB_POSTGRESQL}" STREQUAL "")
      if ("${PACKAGE_DB_POSTGRESQL}" STREQUAL "{PACKAGE_DB_MYSQL}" OR
              "${PACKAGE_DB_POSTGRESQL}" STREQUAL "{PACKAGE_DB_SQLITE}")
        message(FATAL "Conflicting DB type!")
      endif()
      message(FATAL "Not supported yet!")
    endif()

endif()
