cmake_minimum_required(VERSION 3.17)
project(Test)

set(CMAKE_CXX_STANDARD 20)

#file( GLOB SOURCES src/strace2.c)
file( GLOB SOURCES src/main.cpp)
add_executable(Test ${SOURCES})

target_link_libraries(Test pthread)
target_link_libraries(Test libexplain.so)
