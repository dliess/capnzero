function(capnzero_generate_cpp)

  set(noValues)
  set(singleValues
    GEN_CPP_SOURCES
    GEN_CPP_HEADERS
    GEN_QT_CPP_SOURCES
    GEN_QT_CPP_HEADERS
    GEN_OUT_DIR IDL_FILE
  )
  set(multiValues)
  cmake_parse_arguments(
    ARG
    "${noValues}" "${singleValues}" "${multiValues}"
    ${ARGV}
  )

  if(ARG_GEN_CPP_SOURCES)
    set(${ARG_GEN_CPP_SOURCES})
  endif()
  if(ARG_GEN_CPP_HEADERS)
    set(${ARG_GEN_CPP_HEADERS})
  endif()
  if(ARG_GEN_QT_CPP_SOURCES)
    set(${ARG_GEN_QT_CPP_SOURCES})
  endif()
  if(ARG_GEN_QT_CPP_HEADERS)
    set(${ARG_GEN_QT_CPP_HEADERS})
  endif()

  if(ARG_GEN_OUT_DIR)
    set(_GEN_OUTPUT_DIR ${ARG_GEN_OUT_DIR})
  else()
    set(_GEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  GET_TARGET_PROPERTY(GENERATOR_SCRIPT_PATH CAPNZERO::capnzero GENERATOR_SCRIPT_PATH)
  if(NOT EXISTS ${GENERATOR_SCRIPT_PATH})
    message(SEND_ERROR "property GENERATOR_SCRIPT_PATH is not set")
  endif()

  get_filename_component(FIL_WLE ${ARG_IDL_FILE} NAME_WLE) # File name without directory and last extension
  get_filename_component(GENERATOR_SCRIPT_DIR ${GENERATOR_SCRIPT_PATH} DIRECTORY)

  set(ALL_GENERATOS_SCRIPTS
    "${GENERATOR_SCRIPT_PATH}"
    "${GENERATOR_SCRIPT_DIR}/capnp_file.py"
    "${GENERATOR_SCRIPT_DIR}/capnzeroc.py"
    "${GENERATOR_SCRIPT_DIR}/client_cpp.py"
    "${GENERATOR_SCRIPT_DIR}/client_transport_h.py"
    "${GENERATOR_SCRIPT_DIR}/client_transport_inl.py"
    "${GENERATOR_SCRIPT_DIR}/client_h.py"
    "${GENERATOR_SCRIPT_DIR}/client_inl.py"
    "${GENERATOR_SCRIPT_DIR}/common_client.py"
    "${GENERATOR_SCRIPT_DIR}/common.py"
    "${GENERATOR_SCRIPT_DIR}/global_types.py"
    "${GENERATOR_SCRIPT_DIR}/qobject_client_cpp.py"
    "${GENERATOR_SCRIPT_DIR}/qobject_client_h.py"
    "${GENERATOR_SCRIPT_DIR}/rpc_interface_headers.py"
    "${GENERATOR_SCRIPT_DIR}/server_cpp.py"
    "${GENERATOR_SCRIPT_DIR}/server_h.py"
  )

  set(GEN_CAPNP_FILE "${_GEN_OUTPUT_DIR}/${FIL_WLE}.capnp")
  add_custom_command(
    OUTPUT  "${GEN_CAPNP_FILE}"
            "${_GEN_OUTPUT_DIR}/${FIL_WLE}_ClientTransport.h"
            "${_GEN_OUTPUT_DIR}/${FIL_WLE}_ClientTransport.inl"
            "${_GEN_OUTPUT_DIR}/${FIL_WLE}_Client.h"
            "${_GEN_OUTPUT_DIR}/${FIL_WLE}_Client.inl"
            "${_GEN_OUTPUT_DIR}/${FIL_WLE}_Client.cpp"
            "${_GEN_OUTPUT_DIR}/${FIL_WLE}_Server.h"
            "${_GEN_OUTPUT_DIR}/${FIL_WLE}_Server.cpp"
            "${_GEN_OUTPUT_DIR}/${FIL_WLE}_QObjectClient.h"
            "${_GEN_OUTPUT_DIR}/${FIL_WLE}_QObjectClient.cpp"
    COMMAND python3 ${GENERATOR_SCRIPT_PATH}
    ARGS  --outdir=${_GEN_OUTPUT_DIR}
          --descrfile=${ARG_IDL_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${GENERATOR_SCRIPT_PATH}  ${ARG_IDL_FILE} ${ALL_GENERATOS_SCRIPTS}
    COMMENT "Running capnzeroc generator script on ${${ARG_IDL_FILE}}"
    VERBATIM
  )

  if(NOT TARGET CapnProto::capnp_tool)
    message(SEND_ERROR "No capnp_tool TARGET")
  endif()
  if(NOT TARGET CapnProto::capnpc_cpp)
    message(SEND_ERROR "No capnpc_cpp TARGET")
  endif()


  find_program(CAPNP_EXECUTABLE "capnp")
  GET_TARGET_PROPERTY(CAPNPC_CXX_EXECUTABLE CapnProto::capnpc_cpp CAPNPC_CXX_EXECUTABLE)
  if(NOT EXISTS ${CAPNPC_CXX_EXECUTABLE})
    find_program(CAPNPC_CXX_EXECUTABLE "capnpc-c++")
  endif()

  set(CAPNP_GENERATE_BASE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WLE}.capnp")
  add_custom_command(
    OUTPUT "${CAPNP_GENERATE_BASE_PATH}.c++" "${CAPNP_GENERATE_BASE_PATH}.h"
    COMMAND ${CAPNP_EXECUTABLE}
    ARGS compile
        -o ${CAPNPC_CXX_EXECUTABLE}
        --src-prefix ${_GEN_OUTPUT_DIR}
        ${GEN_CAPNP_FILE}
    DEPENDS "${GEN_CAPNP_FILE}"
    COMMENT "Compiling Cap'n Proto schema ${GEN_CAPNP_FILE}"
    VERBATIM
  )

  if(NOT ${_GEN_OUTPUT_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
    add_custom_command(
      OUTPUT "${GEN_CAPNP_FILE}.c++" "${GEN_CAPNP_FILE}.h"
      COMMAND ${CMAKE_COMMAND} -E copy "${CAPNP_GENERATE_BASE_PATH}.c++" "${CAPNP_GENERATE_BASE_PATH}.h" "${_GEN_OUTPUT_DIR}/."
      DEPENDS "${CAPNP_GENERATE_BASE_PATH}.c++" "${CAPNP_GENERATE_BASE_PATH}.h"
      COMMENT "Copying capnp generated sources to destination folder"
      VERBATIM
    )
  endif()

  if(ARG_GEN_CPP_SOURCES)
      list(APPEND ${ARG_GEN_CPP_SOURCES} "${GEN_CAPNP_FILE}.c++")
      list(APPEND ${ARG_GEN_CPP_SOURCES} "${_GEN_OUTPUT_DIR}/${FIL_WLE}_Client.cpp")
      list(APPEND ${ARG_GEN_CPP_SOURCES} "${_GEN_OUTPUT_DIR}/${FIL_WLE}_Server.cpp")
      set(${ARG_GEN_CPP_SOURCES} ${${ARG_GEN_CPP_SOURCES}} PARENT_SCOPE)
  endif()
  if(ARG_GEN_CPP_HEADERS)
    list(APPEND ${ARG_GEN_CPP_HEADERS} "${GEN_CAPNP_FILE}.h")
    list(APPEND ${ARG_GEN_CPP_HEADERS} "${_GEN_OUTPUT_DIR}/${FIL_WLE}_ClientTransport.h")
    list(APPEND ${ARG_GEN_CPP_HEADERS} "${_GEN_OUTPUT_DIR}/${FIL_WLE}_ClientTransport.inl")
    list(APPEND ${ARG_GEN_CPP_HEADERS} "${_GEN_OUTPUT_DIR}/${FIL_WLE}_Client.h")
    list(APPEND ${ARG_GEN_CPP_HEADERS} "${_GEN_OUTPUT_DIR}/${FIL_WLE}_Client.inl")
    list(APPEND ${ARG_GEN_CPP_HEADERS} "${_GEN_OUTPUT_DIR}/${FIL_WLE}_Server.h")
    set(${ARG_GEN_CPP_HEADERS} ${${ARG_GEN_CPP_HEADERS}} PARENT_SCOPE)
  endif()
  if(ARG_GEN_QT_CPP_SOURCES)
    list(APPEND ${ARG_GEN_QT_CPP_SOURCES} "${_GEN_OUTPUT_DIR}/${FIL_WLE}_QObjectClient.cpp")
    set(${ARG_GEN_QT_CPP_SOURCES} ${${ARG_GEN_QT_CPP_SOURCES}} PARENT_SCOPE)
  endif() 
  if(ARG_GEN_QT_CPP_HEADERS)
    list(APPEND ${ARG_GEN_QT_CPP_HEADERS} "${_GEN_OUTPUT_DIR}/${FIL_WLE}_QObjectClient.h")
    set(${ARG_GEN_QT_CPP_HEADERS} ${${ARG_GEN_QT_CPP_HEADERS}} PARENT_SCOPE)
  endif()

endfunction()
