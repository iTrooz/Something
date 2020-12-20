cmake_minimum_required(VERSION 3.17)
project(Test)

set(CMAKE_CXX_STANDARD 17)

file( GLOB SOURCES src/*.cpp)
add_executable(Test ${SOURCES})