#include <opencv2/opencv.hpp>
#include <ctime>
bool comp(const cv::Rect &a, const cv::Rect &b)
{
    return a.x < b.x;
}

void myNumberSort(std::vector<cv::Rect> &g_rect)
{
    sort(g_rect.begin(), g_rect.end(), comp);
}
int myDiscern(cv::Mat n)
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
        printf("[error_%d_%d_%d_%d_%d_%d_%d]", a, b, c, d, e, f, g);
    }
    return num;
}

cv::Mat getDisplayROI(const cv::Mat &_image, const std::string &_file_yaml)
{
    cv::FileStorage fs(_file_yaml, cv::FileStorage::READ);
    std::vector<cv::Point2f> conersSrcPosition, conersDstPosition;
    conersSrcPosition.push_back(cv::Point2f(fs["LUX"], fs["LUY"]));
    conersSrcPosition.push_back(cv::Point2f(fs["LDX"], fs["LDY"]));
    conersSrcPosition.push_back(cv::Point2f(fs["RDX"], fs["RDY"]));
    conersSrcPosition.push_back(cv::Point2f(fs["RUX"], fs["RUY"]));

    cv::Mat dst;
    if (conersSrcPosition.size() == 4)
    {
        cv::Rect bRect = cv::boundingRect(conersSrcPosition);
        bRect.width /= 2;
        bRect.height /= 2;

        conersDstPosition.clear();
        conersDstPosition.push_back(cv::Point2f(0, 0));
        conersDstPosition.push_back(cv::Point2f(0, bRect.height));
        conersDstPosition.push_back(cv::Point2f(bRect.width, bRect.height));
        conersDstPosition.push_back(cv::Point2f(bRect.width, 0));

        cv::Mat pose3 = cv::getPerspectiveTransform(conersSrcPosition, conersDstPosition);
        cv::warpPerspective(_image, dst, pose3, bRect.size());
    }
    return dst;
}

int getCowID(const cv::Mat &_image, const std::string &_file_yaml)
{

    clock_t astartTime, aendTime;
    clock_t startTime, endTime;
    startTime = clock();  //计时开始
    astartTime = clock(); //计时开始
    cv::Mat image = getDisplayROI(_image, _file_yaml);
    // image=image(cv::Rect(image.cols/2,0,image.cols/2,image.rows));
    endTime = clock(); //计时结束
    std::cout << "a The run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
    startTime = clock(); //计时开始

    if (image.empty())
    {
        return -1;
    }

    cv::Mat RGB[3];
    cv::split(image, RGB);
    cv::Mat G;
    cv::dilate(RGB[1], G, cv::Mat(11, 11, CV_8UC1, cv::Scalar(1)));
    cv::Mat W = G > 220;
    cv::imwrite("w1.png", W);
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarcy;
    cv::findContours(W, contours, hierarcy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    std::vector<cv::Rect> rect_num;
    for (int i = 0; i < contours.size(); i++)
    {
        cv::Rect rect = cv::boundingRect(contours[i]);
        int center_y = rect.y + rect.height / 2;
        int center_x = rect.x + rect.width / 2;
        if (center_y < image.rows / 2 + image.rows / 10 && center_y > image.rows / 2 - image.rows / 10)
        {
            rect_num.push_back(rect);
            // cv::rectangle(image, rect, cv::Scalar(255, 0, 0), 5, 8);
        }

        // cv::drawContours(image, contours, i, cv::Scalar(255, 255, 255), 5, 8);
    }
    endTime = clock(); //计时结束
    std::cout << "a The run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
    startTime = clock(); //计时开始
    myNumberSort(rect_num);
    endTime = clock(); //计时结束
    std::cout << "a The run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
    startTime = clock(); //计时开始
    for (size_t i = 0; i < rect_num.size(); i++)
    {
        cv::Rect rect = rect_num[i];
        // Mat W = RGB[1] > 220;
        int num = myDiscern(W(rect));
        std::cout << num << "\n";
        // cv::putText(image, std::to_string(num), cv::Point(rect.x, rect.y), 1, 7, cv::Scalar(0, 0, 255), 3);
        // cv::imshow("a", image);
        // cv::waitKey(50);
    }
    endTime = clock(); //计时结束
    std::cout << "a The run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
    std::cout << "all The run time is: " << (double)(endTime - astartTime) / CLOCKS_PER_SEC << "s" << std::endl;
    startTime = clock(); //计时开始

    // waitKey(0);
    // cv::imwrite("ocr.png", image);
    return 0;
}