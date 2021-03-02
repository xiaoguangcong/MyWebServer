aux_source_directory(./src SRC_FILES)

include_directories(include)

add_executable(WebServer ${SRCS})
target_link_libraries(WebServer libserver)
