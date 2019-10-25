install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/libtelnet.h.3
        ${CMAKE_CURRENT_BINARY_DIR}/telnet_environ_t.3
        ${CMAKE_CURRENT_BINARY_DIR}/telnet_event_t.3
        ${CMAKE_CURRENT_BINARY_DIR}/telnet_event_t_compress_t.3
        ${CMAKE_CURRENT_BINARY_DIR}/telnet_event_t_data_t.3
        ${CMAKE_CURRENT_BINARY_DIR}/telnet_event_t_environ_t.3
        ${CMAKE_CURRENT_BINARY_DIR}/telnet_event_t_error_t.3
        ${CMAKE_CURRENT_BINARY_DIR}/telnet_event_t_iac_t.3
        ${CMAKE_CURRENT_BINARY_DIR}/telnet_event_t_negotiate_t.3
        ${CMAKE_CURRENT_BINARY_DIR}/telnet_event_t_subnegotiate_t.3
        ${CMAKE_CURRENT_BINARY_DIR}/telnet_event_t_ttype_t.3
        ${CMAKE_CURRENT_BINARY_DIR}/telnet_event_t_zmp_t.3
        ${CMAKE_CURRENT_BINARY_DIR}/telnet_event_t_mssp_t.3
        ${CMAKE_CURRENT_BINARY_DIR}/telnet_telopt_t.3
    DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man3
)