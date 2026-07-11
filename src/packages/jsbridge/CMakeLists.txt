# The JS bridge only exists on the Emscripten/WASM target: it calls into
# the hosting page's JavaScript. Native builds simply don't get the efuns
# (the package target is never created, so the spec is never included).
if (EMSCRIPTEN AND ${PACKAGE_JSBRIDGE})
    add_library(package_jsbridge STATIC
            "jsbridge.cc"
            )
endif ()
