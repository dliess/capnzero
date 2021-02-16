function(capnzero_generate_cpp SOURCES HEADERS PROTOCOL_DESCRIPTION_FILE)
  set(_GEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})

  set(${SOURCES})
  set(${HEADERS})

  get_filename_component(FIL_WLE ${PROTOCOL_DESCRIPTION_FILE} NAME_WLE) # File name without directory and last extension

  set(GEN_CAPNP_FILE "${_GEN_OUTPUT_DIR}/${FIL_WLE}.capnp")

  add_custom_command(
    OUTPUT  "${GEN_CAPNP_FILE}"
            "${_GEN_OUTPUT_DIR}/${FIL_WLE}_Client.h"
            "${_GEN_OUTPUT_DIR}/${FIL_WLE}_Client.cpp"
            "${_GEN_OUTPUT_DIR}/${FIL_WLE}_Server.h"
            "${_GEN_OUTPUT_DIR}/${FIL_WLE}_Server.cpp"
    COMMAND python3 ${GENERATOR_SCRIPT_PATH}
    ARGS  --outdir=${_GEN_OUTPUT_DIR}
          --descrfile=${PROTOCOL_DESCRIPTION_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${GENERATOR_SCRIPT_PATH}  ${PROTOCOL_DESCRIPTION_FILE}
    COMMENT "Running capnzeroc generator script on ${PROTOCOL_DESCRIPTION_FILE}"
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

  add_custom_command(
    OUTPUT "${GEN_CAPNP_FILE}.c++" "${GEN_CAPNP_FILE}.h"
    COMMAND ${CAPNP_EXECUTABLE}
    ARGS compile
        -o ${CAPNPC_CXX_EXECUTABLE}
        --src-prefix ${_GEN_OUTPUT_DIR}
        ${GEN_CAPNP_FILE}
    DEPENDS "${GEN_CAPNP_FILE}"
    COMMENT "Compiling Cap'n Proto schema ${GEN_CAPNP_FILE}"
    VERBATIM
  )

  list(APPEND ${SOURCES} "${GEN_CAPNP_FILE}.c++")
  list(APPEND ${SOURCES} "${_GEN_OUTPUT_DIR}/${FIL_WLE}_Client.cpp")
  list(APPEND ${SOURCES} "${_GEN_OUTPUT_DIR}/${FIL_WLE}_Server.cpp")
  list(APPEND ${HEADERS} "${GEN_CAPNP_FILE}.h")
  list(APPEND ${HEADERS} "${_GEN_OUTPUT_DIR}/${FIL_WLE}_Client.h")
  list(APPEND ${HEADERS} "${_GEN_OUTPUT_DIR}/${FIL_WLE}_Server.h")

  set(${SOURCES} ${${SOURCES}} PARENT_SCOPE)
  set(${HEADERS} ${${HEADERS}} PARENT_SCOPE)
endfunction()
