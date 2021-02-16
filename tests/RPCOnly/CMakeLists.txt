get_filename_component(TargetBaseName ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_subdirectory(schema)

add_executable(${TargetBaseName}-client "Client.cpp")
add_executable(${TargetBaseName}-server "Server.cpp")


target_link_libraries(${TargetBaseName}-client
    PRIVATE ${TargetBaseName}-schema
)

target_link_libraries(${TargetBaseName}-server
    PRIVATE ${TargetBaseName}-schema
)
