cmake_minimum_required(VERSION 3.17)
project(Test)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_COMPILER clang++)

file( GLOB SOURCES src/main3.cpp)
add_executable(Test ${SOURCES})

target_link_libraries(Test pthread)
target_link_libraries(Test libexplain.so)
