cmake_minimum_required(VERSION 3.10)
project(webServer)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
add_executable(webServer main.cpp
        http/httpConn.cpp http/httpConn.h
        threadpool/threadPool.h threadpool/locker.h)