set(SOURCES
    "FdSet.cpp"
)

set(HEADERS
    "FdSet.h"
    "error_msg.hpp"
)

add_library(${TargetBaseName}-utils STATIC ${SOURCES} ${HEADERS})
target_include_directories(${TargetBaseName}-utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})