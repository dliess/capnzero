get_filename_component(capnzero_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(CMakeFindDependencyMacro)

list(APPEND CMAKE_MODULE_PATH "${capnzero_CMAKE_DIR}/cmake")

find_dependency(capnproto REQUIRED)
find_dependency(cppzmq REQUIRED)

if(NOT TARGET CAPNZERO::capnzero)
    include("${capnzero_CMAKE_DIR}/capnzero-targets.cmake")
endif()