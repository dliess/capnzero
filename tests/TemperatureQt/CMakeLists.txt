get_filename_component(TargetBaseName "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
add_subdirectory(schema)
add_subdirectory(utils)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)   
find_package(Qt5 5.11.3
   COMPONENTS Core Quick REQUIRED)


add_executable(${TargetBaseName}-client "Client.cpp" "qml.qrc")
add_executable(${TargetBaseName}-server "Server.cpp")

target_link_libraries(${TargetBaseName}-client
    PRIVATE ${TargetBaseName}-schema
    PRIVATE Qt5::Core
    PRIVATE Qt5::Quick
)

target_link_libraries(${TargetBaseName}-server
    PRIVATE ${TargetBaseName}-schema
    PRIVATE ${TargetBaseName}-utils
)
