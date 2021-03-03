cmake_minimum_required(VERSION 3.5)
project(MyWebServer)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -pthread -fopenmp")

add_subdirectory(base)
add_subdirectory(net)
add_subdirectory(client)
