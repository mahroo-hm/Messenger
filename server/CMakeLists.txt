cmake_minimum_required(VERSION 3.1.0)
project(server)

set(CMAKE_CXX_STANDARD 11)

set(SRC_DIRECTORY src)
set(INCLUDE_DIRECTORY headers)

set(SOURCES
    ${SRC_DIRECTORY}/server.cpp
    ${SRC_DIRECTORY}/user_server.cpp
    ${SRC_DIRECTORY}/user.cpp
    ${SRC_DIRECTORY}/helper.cpp
    ${SRC_DIRECTORY}/userDB.cpp
)

include_directories(${INCLUDE_DIRECTORY})
add_executable(${PROJECT_NAME}
    ${SRC_DIRECTORY}/main.cpp
    ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    pthread
    sqlite3 sqlite3)

