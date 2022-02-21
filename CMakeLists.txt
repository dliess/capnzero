cmake_minimum_required(VERSION 3.11)
set (CMAKE_CXX_STANDARD 17)

project(capnzero
    LANGUAGES C CXX
    VERSION 0.0.1
)

include(cmake/CPM.cmake)
####################
### Dependencies ###
####################
CPMFindPackage(
  NAME CapnProto
  GITHUB_REPOSITORY capnproto/capnproto
  VERSION 0.7.0
) 
if (CapnProto_ADDED)
  add_executable(CapnProto::capnpc_cpp ALIAS capnpc_cpp)
  add_executable(CapnProto::capnp_tool ALIAS capnp_tool)
endif ()
  
CPMFindPackage(
  NAME cppzmq
  GITHUB_REPOSITORY zeromq/cppzmq
  VERSION 4.7.0
  OPTIONS
    "CPPZMQ_BUILD_TESTS OFF"
)
####################
    
include(GNUInstallDirs)

set(TARGET_NAME ${PROJECT_NAME})

add_library(${TARGET_NAME} STATIC "dummy.cpp")
add_library(CAPNZERO::${TARGET_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${TARGET_NAME}
  INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>"
)

target_link_libraries(${TARGET_NAME}
  PUBLIC CapnProto::capnp-rpc
  PUBLIC cppzmq
)

set_target_properties(${TARGET_NAME}
PROPERTIES
  GENERATOR_SCRIPT_PATH "${CMAKE_CURRENT_LIST_DIR}/scripts/capnzeroc.py"
)

option(CAPNZERO_BUILD_TESTS "Capnzero build tests" ON)
if(${CAPNZERO_BUILD_TESTS})
  add_subdirectory(tests)
endif()

######################################################
#Install target
######################################################

install(TARGETS ${TARGET_NAME} EXPORT ${TARGET_NAME}-targets
  INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}"
)

install(
  FILES 
    "include/capnzero_NativeCapnpMsgWriter.h"
    "include/capnzero_typedefs.h"
    "include/capnzero_utils.h"
    "include/error_msg.hpp"
    "include/span.h"
  DESTINATION 
    "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}"
)

install(EXPORT ${TARGET_NAME}-targets
    NAMESPACE
        CAPNZERO::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

set(GENERATOR_SCRIPT_PATH_INSTALL ${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME})
configure_file(cmake/${PROJECT_NAME}-config.cmake.in cmake/${PROJECT_NAME}-config.cmake @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}-config.cmake" "cmake/capnzero_generate_cpp.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

install(
  FILES 
    "scripts/capnp_file.py"
    "scripts/capnzeroc.py"
    "scripts/client_cpp.py"
    "scripts/client_h.py"
    "scripts/client_inl.py"
    "scripts/client_transport_h.py"
    "scripts/client_transport_inl.py"
    "scripts/common_client.py"
    "scripts/common.py"
    "scripts/global_types.py"
    "scripts/qobject_client_cpp.py"
    "scripts/qobject_client_h.py"
    "scripts/rpc_interface_headers.py"
    "scripts/server_cpp.py"
    "scripts/server_h.py"
  DESTINATION
    "${GENERATOR_SCRIPT_PATH_INSTALL}"
)