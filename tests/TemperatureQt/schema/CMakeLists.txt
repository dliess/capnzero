set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)   

find_package(Qt5 5.11.3
   COMPONENTS Core REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(${PROJECT_SOURCE_DIR}/cmake/capnzero_generate_cpp.cmake)
set(OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated")
capnzero_generate_cpp(
    GEN_CPP_SOURCES    gen_cpp_sources
    GEN_CPP_HEADERS    gen_cpp_headers
    GEN_QT_CPP_SOURCES gen_qt_cpp_sources
    GEN_QT_WC_CPP_SOURCES gen_qt_wc_cpp_sources
    GEN_QT_CPP_HEADERS gen_qt_cpp_headers
    GEN_OUT_DIR        ${OUT_DIR}
    IDL_FILE           "TemperatureQt.toml"
    CLANG_FORMAT       "ON"
    SKIP_GENERATION    "OFF"
)

set(TARGET_NAME ${TargetBaseName}-schema)

add_library(${TARGET_NAME} STATIC
    ${gen_cpp_sources}
    ${gen_cpp_headers}
    ${gen_qt_cpp_sources}
    ${gen_qt_cpp_headers}
)
target_include_directories(${TARGET_NAME} PUBLIC ${OUT_DIR})

target_link_libraries(${TARGET_NAME}
    PUBLIC capnzero
    PUBLIC Qt5::Core
)

set(TARGET_NAME_WC ${TargetBaseName}-schema-wc)

add_library(${TARGET_NAME_WC} STATIC
    ${gen_cpp_sources}
    ${gen_cpp_headers}
    ${gen_qt_wc_cpp_sources}
    ${gen_qt_cpp_headers}
)
target_include_directories(${TARGET_NAME_WC} PUBLIC ${OUT_DIR})

target_link_libraries(${TARGET_NAME_WC}
    PUBLIC capnzero
    PUBLIC Qt5::Core
)