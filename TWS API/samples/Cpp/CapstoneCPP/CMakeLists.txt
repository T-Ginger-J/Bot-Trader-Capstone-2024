file(GLOB SOURCES "*.cpp")
add_executable(CapstoneCppClient ${SOURCES})
set_target_properties(CapstoneCppClient
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
target_link_libraries(CapstoneCppClient PRIVATE twsapi)
target_link_libraries(CapstoneCppClient PUBLIC Threads::Threads)
target_include_directories(CapstoneCppClient PRIVATE "${PROJECT_SOURCE_DIR}/source/cppclient/" "${PROJECT_SOURCE_DIR}/source/cppclient/client/")

if( MSVC )
    target_compile_options( CapstoneCppClient PRIVATE /W3 )
else()
    target_compile_options( CapstoneCppClient PRIVATE -Wall -Wextra -pedantic -Wno-unused-parameter )
endif()

install(TARGETS CapstoneCppClient DESTINATION bin)

FILE(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION "include/tws-api")