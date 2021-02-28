cmake_minimum_required(VERSION 3.5)
project(MyWebServer)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -pthread -fopenmp")

## add include/src file
file(GLOB_RECURSE cpp_srcs ${PROJECT_SOURCE_DIR}/src/*.cpp)
set(include_dirs ${PROJECT_SOURCE_DIR}/include)

include_directories(${include_dirs})

#设置可执行文件的输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)    

#生成可执行文件
add_executable(MyWebServer ${cpp_srcs})

add_subdirectory(base)