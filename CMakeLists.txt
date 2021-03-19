cmake_minimum_required(VERSION 3.11)
set (CMAKE_CXX_STANDARD 17)

project(capnzero
    LANGUAGES C CXX
    VERSION 0.0.1
)

include(cmake/CPM.cmake)

CPMFindPackage(
    NAME capnproto
    GITHUB_REPOSITORY capnproto/capnproto
    VERSION 0.7.0
)

if (capnproto_ADDED)
  add_executable(CapnProto::capnpc_cpp ALIAS capnpc_cpp)
  add_executable(CapnProto::capnp_tool ALIAS capnp_tool)
endif (capnproto_ADDED)

#CPMFindPackage(
#  NAME libzmq
#  GITHUB_REPOSITORY zeromq/libzmq
#  VERSION 4.3.4
#  OPTIONS
#   "ZMQ_BUILD_TESTS OFF"
#)

CPMFindPackage(
  NAME cppzmq
  GITHUB_REPOSITORY zeromq/cppzmq
  VERSION 4.7.0
  OPTIONS
   "CPPZMQ_BUILD_TESTS OFF"
)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(GENERATOR_SCRIPT_PATH ${CMAKE_CURRENT_LIST_DIR}/scripts/capnzeroc.py)

option(CAPNZERO_BUILD_TESTS "Capnzero build tests" ON)
if(${CAPNZERO_BUILD_TESTS})
  add_subdirectory(tests)
endif()