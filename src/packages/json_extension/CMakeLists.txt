if(PACKAGE_JSON_EXTENSION)
    add_library(package_json_extension STATIC pkg_json.cc yyjson.c)
    target_include_directories(package_json_extension PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../)
endif()