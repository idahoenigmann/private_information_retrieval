cmake_minimum_required(VERSION 3.5)
project(private_information_retrieval)

include(FetchContent)

FetchContent_Declare(
        asio
        GIT_REPOSITORY https://github.com/chriskohlhoff/asio
        GIT_TAG asio-1-14-0
        UPDATE_DISCONNECTED 1
)

FetchContent_Populate(asio)
include_directories(${asio_SOURCE_DIR}/asio/include/)

file(GLOB_RECURSE CLIENT_SOURCES "src/client/*.cpp")
add_executable(client ${CLIENT_SOURCES})
target_include_directories(client PRIVATE include/client)

file(GLOB_RECURSE SERVER_SOURCES "src/server/*.cpp")
add_executable(server ${SERVER_SOURCES})
target_include_directories(server PRIVATE include/server)