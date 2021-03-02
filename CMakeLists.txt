cmake_minimum_required(VERSION 3.5)
project(MyWebServer)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2 -pthread -fopenmp")

#设置可执行文件的输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)    

#生成可执行文件
#add_executable(MyWebServer ${cpp_srcs})

add_subdirectory(base)
add_subdirectory(net)