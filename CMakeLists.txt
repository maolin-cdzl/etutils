CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

project(etutils)

set(CMAKE_C_FLAGS "-std=c99 -fPIC ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "-std=c++11 -fPIC ${CMAKE_CXX_FLAGS}")

add_library(etutils STATIC
	src/time.cxx
	src/zprotobuf.cxx
	src/ev_zmq.c
	src/ev_zk.c
	src/ev_private.h
	src/thread.cxx
	src/threadpool.cxx
	include/etutils/common/time.hpp
	include/etutils/zep/zprotobuf.hpp
	include/etutils/zep/ev_zmq.h
	include/etutils/zep/ev_zk.h
	include/etutils/mt/thread.hpp
	include/etutils/mt/mqthread.hpp
	include/etutils/mt/threadpool.hpp
	include/etutils/mt/mqthreadpool.hpp
)

target_include_directories(etutils
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(etutils PRIVATE _POSIX_C_SOURCE=200809L )

target_link_libraries(etutils PUBLIC
	czmq
	zmq
	protobuf
)

install(DIRECTORY include/ DESTINATION include FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
install(TARGETS etutils DESTINATION lib/)

#add_subdirectory(test)

