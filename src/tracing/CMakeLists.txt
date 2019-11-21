find_program(DTRACE dtrace)

set(TRACING_H "${CMAKE_CURRENT_BINARY_DIR}/tracing.autogen.h")
set(TRACING_O "${CMAKE_CURRENT_BINARY_DIR}/tracing.autogen.o")

add_custom_command(
  OUTPUT "${TRACING_H}"
  COMMAND ${DTRACE} "-h" "-s" "${CMAKE_CURRENT_SOURCE_DIR}/fluffos.d" "-o" "${TRACING_H}"
  DEPENDS "fluffos.d"
)

SET_SOURCE_FILES_PROPERTIES(
  ${TRACING_H}
  PROPERTIES
  GENERATED TRUE
)

add_custom_command(
  OUTPUT "${TRACING_O}"
  COMMAND ${DTRACE} "-G" "-s" "${CMAKE_CURRENT_SOURCE_DIR}/fluffos.d" "-o" "${TRACING_O}"
  DEPENDS "fluffos.d"
)

SET_SOURCE_FILES_PROPERTIES(
  ${TRACING_O}
  PROPERTIES
  EXTERNAL_OBJECT TRUE
  GENERATED TRUE
  )

add_library(tracing ${TRACING_H} ${TRACING_O})
set_target_properties(tracing PROPERTIES LINKER_LANGUAGE CXX)
