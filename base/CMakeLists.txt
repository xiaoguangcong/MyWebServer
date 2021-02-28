aux_source_directory(./src SRC_FILES)

set( headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MutexLock.h
    ${CMAKE_CURRENT_SOURCE_DIR}//include/Condition.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/NonCopyable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/CountDownLatch.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Thread.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/CurrentThread.h)

message(${headers})

add_library(libbase ${SRC_FILES} ${headers})

