if(OpenCV_FOUND)
    include_directories(../ROI)
    include_directories(../OCR)
    include_directories(${OpenCV_INCLUDE_DIRS})
    
    add_executable(set_display setDisplay.cpp )
    target_link_libraries(set_display ${OpenCV_LIBRARIES})

    add_executable(cow_id recognizeID.cpp ../OCR/cow_id.cpp)
    target_link_libraries(cow_id ${OpenCV_LIBRARIES})
endif()


