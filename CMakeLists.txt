cmake_minimum_required(VERSION 3.10)
project(BlueareRedisClone)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_compile_options(-Wall -Wextra -pedantic)

add_executable(server
    server.cpp
    hashtable.cpp
    avl.cpp
    zset.cpp
)

add_executable(client
    client.cpp
)

