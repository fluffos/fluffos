# Make Func
find_package(BISON REQUIRED)
BISON_TARGET(MakeFunc make_func.y ${CMAKE_CURRENT_BINARY_DIR}/make_func.cc)
add_executable(make_func ${BISON_MakeFunc_OUTPUT_SOURCE})
target_link_libraries(make_func common)

#build_applies
add_executable(build_applies "build_applies.cc")
target_link_libraries(build_applies common)

add_executable(preprocessor "preprocessor.cc")
target_link_libraries(preprocessor common)


