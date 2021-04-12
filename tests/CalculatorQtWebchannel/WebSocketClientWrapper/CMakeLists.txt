set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 5.11.3 
   COMPONENTS Core WebSockets WebChannel REQUIRED)

set(SOURCES
   "WebSocketTransport.cpp"
   "WebSocketClientWrapper.cpp"
)
set(HEADER
   "WebSocketTransport.h"
   "WebSocketClientWrapper.h"
)

add_library(QtWebSocketClientWrapper SHARED
   ${SOURCES}
   ${HEADER}
)

target_include_directories(QtWebSocketClientWrapper
   PUBLIC ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(QtWebSocketClientWrapper
   PUBLIC Qt5::Core
   PUBLIC Qt5::WebSockets
   PUBLIC Qt5::WebChannel
)

set_target_properties(QtWebSocketClientWrapper PROPERTIES
   VERSION ${PROJECT_VERSION}
   SOVERSION ${PROJECT_VERSION_MAJOR}
)

install(TARGETS QtWebSocketClientWrapper
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)