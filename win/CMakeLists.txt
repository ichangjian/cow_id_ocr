if(OpenCV_FOUND)
    include_directories(../ROI)
    include_directories(${OpenCV_INCLUDE_DIRS})
    
    add_executable(set_display setDisplay.cpp )
    target_link_libraries(set_display ${OpenCV_LIBRARIES})
endif()


