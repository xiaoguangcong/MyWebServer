aux_source_directory(./src SRC_FILES)

include_directories(include)

 #设置库文件的输出目录
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)    

add_library(server ${SRC_FILES} ${headers})
target_link_libraries(server pthread rt)


