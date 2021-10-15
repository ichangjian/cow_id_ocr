#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

class COWID
{
private:
    cv::Mat m_image_src;
    cv::Rect m_display_roi;
    int m_font_height_u;
    int m_font_height_d;
    int m_pen_width;
    int m_green_threshold;
    std::vector<cv::Point2f> m_corner_src;
    cv::Size m_roi_size;
    cv::Mat m_pose;
    bool m_init_flag;

    bool initROI(const std::string &_file_yaml);
    cv::Mat getDisplayRegion(const cv::Mat &_image);
    bool static compare(const cv::Rect &_a, const cv::Rect &_b);
    void sortPosition(std::vector<cv::Rect> &_rect);
    int numberOCR(cv::Mat n);

public:
    COWID(/* args */);
    ~COWID();

    bool getCowID(const cv::Mat &_image, std::string &_id);
};
