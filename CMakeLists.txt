cmake_minimum_required(VERSION 3.1)
project(opencv_example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_GXX_FLAGS "-Wall -Wextra -Wconversion -pedantic -lstdc++fs")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

file( GLOB_RECURSE headers "${CMAKE_SOURCE_DIR}/*.h*")
file( GLOB_RECURSE sources "${CMAKE_SOURCE_DIR}/*.cpp")

file( GLOB_RECURSE REMOVE_CMAKE "${CMAKE_SOURCE_DIR}/build/*.cpp" )
list( REMOVE_ITEM sources "${REMOVE_CMAKE}" )
file( GLOB_RECURSE REMOVE_CMAKE "${CMAKE_SOURCE_DIR}/build/*.cpp" )
list( REMOVE_ITEM sources "${REMOVE_CMAKE}" )

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(out ${sources} ${headers})
target_link_libraries(out PRIVATE ${OpenCV_LIBS})
