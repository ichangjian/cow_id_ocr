cmake_minimum_required (VERSION 2.8.9)


IF (CMAKE_SYSTEM_NAME MATCHES "Android")
    include_directories(../OCR)
    include_directories(../log)
    include_directories(../UVC)
    include_directories(../SDK)
    include_directories(../NET)

    include_directories(${OpenCV_INCLUDE_DIRS})

    add_library(cowid SHARED  com_zzas_openapplication_sdk_run.cpp ../OCR/cow_id.cpp ../UVC/capture.cpp ../NET/net.cpp ../SDK/cow_sdk.cpp)
    target_link_libraries(cowid ${OpenCV_LIBRARIES} ${Android_log})

ENDIF (CMAKE_SYSTEM_NAME MATCHES "Android")

