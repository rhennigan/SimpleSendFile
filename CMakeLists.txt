cmake_minimum_required(VERSION 3.3)
project(SimpleSendFile)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(UTIL_FILES sock_util.c sock_util.h)

set(SOURCE_FILES ${UTIL_FILES} sock_srv.c sock_cli.c)
add_executable(sock_cli sock_cli.c ${UTIL_FILES})
add_executable(sock_srv sock_srv.c ${UTIL_FILES})