include(${PROJECT_SOURCE_DIR}/cmake/capnzero_generate_cpp.cmake)
capnzero_generate_cpp(
    GEN_CPP_SOURCES gen_cpp_sources
    GEN_CPP_HEADERS gen_cpp_headers
    GEN_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
    IDL_FILE "CalculatorRpcOnly.toml"
)

add_library(${TargetBaseName}-schema STATIC ${gen_cpp_sources} ${gen_cpp_headers})
target_include_directories(${TargetBaseName}-schema PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${TargetBaseName}-schema
    PUBLIC capnzero
)