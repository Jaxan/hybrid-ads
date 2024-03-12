cmake_minimum_required(VERSION 3.5)
project(hybrid-ads)

set(CMAKE_CXX_STANDARD 14)

find_package (Threads)
set(libs ${libs} ${CMAKE_THREAD_LIBS_INIT})

add_subdirectory("lib")
add_subdirectory("src")
