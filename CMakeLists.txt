cmake_minimum_required(VERSION 3.15)
project(Sockets)

set(CMAKE_CXX_STANDARD 14)

add_executable(Sockets main.cpp)
target_link_libraries(Sockets ws2_32)
target_compile_options(Sockets PRIVATE /W4 /WX)
