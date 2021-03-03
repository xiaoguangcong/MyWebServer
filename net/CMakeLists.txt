aux_source_directory(./src SRC_FILES)

set(HEADER
    ${PROJECT_SOURCE_DIR}/base/include
    ${PROJECT_SOURCE_DIR}/net/include
    )

include_directories(${HEADER})

 #设置可执行文件的输出目录
 set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) 

add_executable(WebServer ${SRC_FILES})
target_link_libraries(WebServer ${PROJECT_SOURCE_DIR}/lib/libserver.a)
