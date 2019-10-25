find_package(Doxygen)

if (DOXYGEN_FOUND)
  add_subdirectory(man)

  configure_file(Doxyfile.in Doxyfile)
  add_custom_target(doc_doxygen ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
    SOURCES Doxyfile.in
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM)
endif (DOXYGEN_FOUND)
