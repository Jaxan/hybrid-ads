project(Yannakakis)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")

find_package (Threads)
set(libs ${libs} ${CMAKE_THREAD_LIBS_INIT})

add_subdirectory("lib")
add_subdirectory("src")
