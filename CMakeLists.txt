project(Yannakakis)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")

find_package(Boost REQUIRED COMPONENTS program_options filesystem system serialization)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
set(libs ${libs} ${Boost_LIBRARIES})

# add_subdirectory("contrib")
add_subdirectory("lib")
add_subdirectory("src")

# file(GLOB resources "resources/*")
# file(COPY ${resources} DESTINATION ${CMAKE_BINARY_DIR})
