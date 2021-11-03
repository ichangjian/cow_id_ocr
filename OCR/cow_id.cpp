#include "cow_id.hpp"

COWID::COWID(/* args */)
{
    m_init_flag = false;
    m_green_threshold = 220;
    // initROI("../corner1.yaml");
}

COWID::~COWID()
{
}

bool COWID::initROI(const std::string &_file_yaml)
{
    cv::FileStorage fs(_file_yaml, cv::FileStorage::READ);

    m_corner_src.clear();
    m_corner_src.push_back(cv::Point2f(fs["MUX"], fs["MUY"]));
    m_corner_src.push_back(cv::Point2f(fs["MDX"], fs["MDY"]));
    m_corner_src.push_back(cv::Point2f(fs["RDX"], fs["RDY"]));
    m_corner_src.push_back(cv::Point2f(fs["RUX"], fs["RUY"]));

    cv::Rect bRect = cv::boundingRect(m_corner_src);
    m_display_roi = bRect;
    for (size_t i = 0; i < m_corner_src.size(); i++)
    {
        m_corner_src[i] -= cv::Point2f(m_display_roi.x, m_display_roi.y);
    }

    // bRect.width /= 2;
    // bRect.height /= 2;
    std::vector<cv::Point2f> conersDstPosition;
    conersDstPosition.push_back(cv::Point2f(0, 0));
    conersDstPosition.push_back(cv::Point2f(0, bRect.height));
    conersDstPosition.push_back(cv::Point2f(bRect.width, bRect.height));
    conersDstPosition.push_back(cv::Point2f(bRect.width, 0));

    m_roi_size = bRect.size();
    m_pose = cv::getPerspectiveTransform(m_corner_src, conersDstPosition);

    m_init_flag = true;
    m_font_height_d = m_roi_size.height / 3 - m_roi_size.height / 10;
    m_font_height_u = m_roi_size.height / 3 + m_roi_size.height / 10;
    m_pen_width = int(m_roi_size.height / 60) * 2 + 1;
    // std::cout << m_font_height_d << " " << m_font_height_u << " " << m_pen_width << "\n";
    return true;
}
cv::Mat COWID::getDisplayRegion(const cv::Mat &_image)
{
    if (m_init_flag)
    {
        cv::Mat dst;
        cv::warpPerspective(_image, dst, m_pose, m_roi_size);
        return dst;
    }
    else
    {
        return cv::Mat();
    }
}

bool COWID::compare(const cv::Rect &_a, const cv::Rect &_b)
{
    return _a.x < _b.x;
}

void COWID::sortPosition(std::vector<cv::Rect> &_rect)
{
    sort(_rect.begin(), _rect.end(), compare);
}

bool COWID::getCowID(const cv::Mat &_image, std::string &_id)
{

    cv::Mat image = getDisplayRegion(_image(m_display_roi));

    if (image.empty() && image.channels() == 3)
    {
        return false;
    }

    cv::Mat RGB[3];
    cv::split(image, RGB);

    cv::Mat G;
    cv::dilate(RGB[1], G, cv::Mat(m_pen_width, m_pen_width, CV_8UC1, cv::Scalar(1)));

    cv::Mat W = G > m_green_threshold;
    // cv::imwrite("w.png", W);
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarcy;
    cv::findContours(W, contours, hierarcy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    static int save_index = 0;
    std::vector<cv::Rect> rect_num;
    for (int i = 0; i < contours.size(); i++)
    {
        cv::Rect rect = cv::boundingRect(contours[i]);
        int center_y = rect.height;
        int center_x = rect.width;
        // std::cout << m_font_height_d << " " << m_font_height_u << " " << center_y << "\n";

        if (center_y < m_font_height_u && center_y > m_font_height_d)
        {
            rect_num.push_back(rect);

            cv::rectangle(image, rect, cv::Scalar(255, 0, 0), 5, 8);
        }

        cv::drawContours(image, contours, i, cv::Scalar(255, 255, 255), 5, 8);
    }

    sortPosition(rect_num);
    _id = "";
    for (size_t i = 0; i < rect_num.size(); i++)
    {
        cv::Rect rect = rect_num[i];
        // Mat W = RGB[1] > 220;
        int num = numberOCR(W(rect));
        _id += std::to_string(num);
        // std::cout << num << "\n";
        cv::putText(image, std::to_string(num), cv::Point(rect.x, rect.y), 1, 5, cv::Scalar(0, 0, 255), 3);
        cv::imshow("Cow ID", image);
        if (num == -2)
        {
            cv::imwrite("./temp/error_" + std::to_string(save_index++) + ".png", image);
            return false;
        }

        // cv::waitKey(30);
    }
    cv::imshow("Cow ID", image);
    cv::waitKey(10);
    // waitKey(0);
    // cv::imwrite("ocr.png", image);
    return true;
}

int COWID::numberOCR(cv::Mat n)
{
    int num = -2;
    if (3 * n.cols < n.rows)
    {
        num = 1;
        return num;
    }
    int x_half = n.cols / 2;
    int y_one_third = n.rows / 3;
    int y_two_third = n.rows * 2 / 3;
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0;

    for (int i = 0; i < n.rows; i++)
    {
        uchar *data = n.ptr<uchar>(i);
        if (i < y_one_third)
        {
            if (data[x_half] == 255)
                a = 1;
        }
        else if (i > y_one_third && i < y_two_third)
        {
            if (data[x_half] == 255)
                g = 1;
        }
        else
        {
            if (data[x_half] == 255)
                d = 1;
        }
    }
    for (int j = 0; j < n.cols; j++)
    {
        uchar *data = n.ptr<uchar>(y_one_third);
        if (j < x_half)
        {
            if (data[j] == 255)
                f = 1;
        }
        else
        {
            if (data[j] == 255)
                b = 1;
        }
    }
    for (int j = 0; j < n.cols; j++)
    {
        uchar *data = n.ptr<uchar>(y_two_third);
        if (j < x_half)
        {
            if (data[j] == 255)
                e = 1;
        }
        else
        {
            if (data[j] == 255)
                c = 1;
        }
    }

    if (a == 1 && b == 1 && c == 1 && d == 1 && e == 1 && f == 1 && g == 0)
    {
        num = 0;
    }
    else if (a == 0 && b == 1 && c == 1 && d == 0 && e == 0 && f == 0 && g == 0)
    {
        num = 1;
    }
    else if (a == 1 && b == 1 && c == 0 && d == 1 && e == 1 && f == 0 && g == 1)
    {
        num = 2;
    }
    else if (a == 1 && b == 1 && c == 1 && d == 1 && e == 0 && f == 0 && g == 1)
    {
        num = 3;
    }
    else if (a == 0 && b == 1 && c == 1 && d == 0 && e == 0 && f == 1 && g == 1)
    {
        num = 4;
    }
    else if (a == 1 && b == 0 && c == 1 && d == 1 && e == 0 && f == 1 && g == 1)
    {
        num = 5;
    }
    else if (a == 1 && b == 0 && c == 1 && d == 1 && e == 1 && f == 1 && g == 1)
    {
        num = 6;
    }
    else if (a == 1 && b == 1 && c == 1 && d == 0 && e == 0 && f == 0 && g == 0)
    {
        num = 7;
    }
    else if (a == 1 && b == 1 && c == 1 && d == 1 && e == 1 && f == 1 && g == 1)
    {
        num = 8;
    }
    else if (a == 1 && b == 1 && c == 1 && d == 1 && e == 0 && f == 1 && g == 1)
    {
        num = 9;
    }
    else
    {
        ;// printf("[error_%d_%d_%d_%d_%d_%d_%d]", a, b, c, d, e, f, g);
    }
    return num;
}