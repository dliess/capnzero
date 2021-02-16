include(${PROJECT_SOURCE_DIR}/cmake/capnzero_generate_cpp.cmake)
capnzero_generate_cpp(CAPNZERO_GEN_SRCS CAPNZERO__GEN_HDRS "CalculatorRpcOnly.toml")

add_library(${TargetBaseName}-schema STATIC ${CAPNZERO_GEN_SRCS})
target_include_directories(${TargetBaseName}-schema PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${TargetBaseName}-schema
    PRIVATE cppzmq
    PRIVATE CapnProto::capnp-rpc
    PUBLIC capnzero
)