cmake_minimum_required(VERSION 3.10)
project(Messenger VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(SOURCES
    main.cpp
    messenger/messenger.cpp
    server/server.cpp
    client/client.cpp
)
set(HEADERS
    messenger/messenger.hpp
    server/server.hpp
    client/client.hpp
)
add_executable(messenger ${SOURCES} ${HEADERS})
target_include_directories(messenger PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/messenger
    ${CMAKE_CURRENT_SOURCE_DIR}/server
    ${CMAKE_CURRENT_SOURCE_DIR}/client
)
install(TARGETS messenger
    RUNTIME DESTINATION bin
)