set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)   

find_package(Qt5 5.11.3
   COMPONENTS Core REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


include(${PROJECT_SOURCE_DIR}/cmake/capnzero_generate_cpp.cmake)
capnzero_generate_cpp(CAPNZERO_GEN_SRCS CAPNZERO_GEN_HDRS "CalculatorQt.toml" "WITH_QT_CLIENT")

add_library(${TargetBaseName}-schema STATIC ${CAPNZERO_GEN_SRCS} ${CAPNZERO_GEN_HDRS})
target_include_directories(${TargetBaseName}-schema PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${TargetBaseName}-schema
    PRIVATE cppzmq
    PRIVATE CapnProto::capnp-rpc
    PUBLIC capnzero
    PUBLIC Qt5::Core
)