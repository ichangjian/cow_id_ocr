cmake_minimum_required (VERSION 2.8.9)
project (CowID)

string(TIMESTAMP COMPILE_TIME %Y-%m-%d_%H:%M:%S)
set(BUILD_TIME    ${COMPILE_TIME})
message(STATUS "Build time ${BUILD_TIME}")

set(GIT_HASH "")
set(GIT_BRANCH "")
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%h
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR})

    execute_process(
        COMMAND ${GIT_EXECUTABLE} symbolic-ref --short -q HEAD
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR})
    
    message(STATUS "GIT FOUND ${GIT_BRANCH} ${GIT_HASH}")
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/build_version.h.in 
    ${CMAKE_BINARY_DIR}/build_version.h)
include_directories(${CMAKE_BINARY_DIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON )
set(CMAKE_BUILD_TYPE "Release")
message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    MESSAGE(STATUS "current platform: Linux ")
    add_definitions(-DLINUX_PC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_CXX_STANDARD 11)

ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    MESSAGE(STATUS "current platform: Windows")
    set(CMAKE_CXX_FLAGS "-std=gnu++11 -fpermissive")
    add_definitions("-DWINDOWS")
    set(WIN_SCT ws2_32)

ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Android")
	MESSAGE(STATUS "current platform: Android ${ANDROID_ABI}")
    add_definitions("-D_ANDROID_")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fopenmp -mfpu=neon ")
    set(CMAKE_CXX_STANDARD 11)
    set(Android_log  log)

ELSE ()
	MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -fPIC -Wall -s -pthread")  
find_package(OpenCV)
if(OpenCV_FOUND)
    include_directories(OCR)
    include_directories(log)
    include_directories(UVC)
    include_directories(SDK)
    include_directories(net)
    
    include_directories(${OpenCV_INCLUDE_DIRS})
    
    add_executable(runner sample/sample.cpp OCR/cow_id.cpp )
    target_link_libraries(runner ${OpenCV_LIBRARIES})

    IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
        add_executable(rec_cow_id sample/demo.cpp OCR/cow_id.cpp  net/net.cpp SDK/cow_sdk.cpp )
        target_link_libraries(rec_cow_id ${OpenCV_LIBRARIES} ${Android_log} ${WIN_SCT})
    ELSE ()
        add_executable(rec_cow_id sample/demo.cpp OCR/cow_id.cpp UVC/capture.cpp net/net.cpp SDK/cow_sdk.cpp)
        target_link_libraries(rec_cow_id ${OpenCV_LIBRARIES} ${Android_log})

        add_executable(cap sample/mjpg2mat.cpp UVC/capture.cpp)
        target_link_libraries(cap ${OpenCV_LIBRARIES})
    ENDIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    
endif()

add_subdirectory(win)


