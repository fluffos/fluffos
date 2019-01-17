function(print_target_properties tgt)
    if(NOT TARGET ${tgt})
        message("There is no target named '${tgt}'")
        return()
    endif()

    # this list of properties can be extended as needed
    set(CMAKE_PROPERTY_LIST SOURCE_DIR BINARY_DIR COMPILE_DEFINITIONS
            COMPILE_OPTIONS INCLUDE_DIRECTORIES LINK_LIBRARIES)

    message("Configuration for target ${tgt}")

    foreach (prop ${CMAKE_PROPERTY_LIST})
        get_property(propval TARGET ${tgt} PROPERTY ${prop} SET)
        if (propval)
            get_target_property(propval ${tgt} ${prop})
            message (STATUS "${prop} = ${propval}")
        endif()
    endforeach(prop)

endfunction(print_target_properties)

function(print_iterface_target_properties tgt)
  if(NOT TARGET ${tgt})
    message("There is no target named '${tgt}'")
    return()
  endif()

  # this list of properties can be extended as needed
  set(CMAKE_PROPERTY_LIST INTERFACE_SOURCE_DIR INTERFACE_BINARY_DIR INTERFACE_COMPILE_DEFINITIONS
    INTERFACE_COMPILE_OPTIONS INTERFACE_INCLUDE_DIRECTORIES INTERFACE_LINK_LIBRARIES)

  message("Configuration for target ${tgt}")

  foreach (prop ${CMAKE_PROPERTY_LIST})
    get_property(propval TARGET ${tgt} PROPERTY ${prop} SET)
    if (propval)
      get_target_property(propval ${tgt} ${prop})
      message (STATUS "${prop} = ${propval}")
    endif()
  endforeach(prop)

endfunction(print_iterface_target_properties)
