file(GLOB PACKAGE_CORE_SOURCE "*.cc")
file(GLOB PACKAGE_CORE_HEADER "*.h")

# The DNS resolver has per-target implementations (dns.cc holds the shared
# address cache): evdns natively, a clean-failure stub on WASM.
if (EMSCRIPTEN)
  list(REMOVE_ITEM PACKAGE_CORE_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/dns_libevent.cc")
else ()
  list(REMOVE_ITEM PACKAGE_CORE_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/dns_stub.cc")
endif ()

add_library(package_core STATIC
        ${PACKAGE_CORE_SOURCE}
        ${PACKAGE_CORE_HEADER}
        )
