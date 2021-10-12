cmake_minimum_required (VERSION 2.8.9)
project (TrueFalse)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -fPIC")  
find_package(OpenCV)

if(OpenCV_FOUND)
    include_directories(.)
    include_directories(${OpenCV_INCLUDE_DIRS})
    add_executable(runner test.cpp )
    target_link_libraries(runner ${OpenCV_LIBRARIES})
endif()


