if(OpenCV_FOUND)
    include_directories(../ROI)
    include_directories(../OCR)
    include_directories(${OpenCV_INCLUDE_DIRS})
    
    add_executable(set_display setDisplay.cpp)
    target_link_libraries(set_display ${OpenCV_LIBRARIES})

    add_executable(video2image video2image.cpp )
    target_link_libraries(video2image ${OpenCV_LIBRARIES})

    add_executable(captureImage captureImage.cpp )
    target_link_libraries(captureImage ${OpenCV_LIBRARIES})

    add_executable(cow_id recognizeID.cpp ../OCR/cow_id.cpp)
    target_link_libraries(cow_id ${OpenCV_LIBRARIES})

    add_executable(video_cow_id recognizeVideo.cpp ../OCR/cow_id.cpp)
    target_link_libraries(video_cow_id ${OpenCV_LIBRARIES})
endif()


