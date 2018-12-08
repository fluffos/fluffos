# Make Func
find_package(BISON REQUIRED)
BISON_TARGET(MakeFunc make_func.y ${CMAKE_CURRENT_BINARY_DIR}/make_func.cc)
add_executable(make_func ${BISON_MakeFunc_OUTPUT_SOURCE})

#build_applies
add_executable(build_applies "build_applies.cc")

add_executable(make_options_defs "make_options_defs.cc" "preprocessor.hpp")
