#include <opencv2/opencv.hpp>
#include "cow_id.hpp"
using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cout << "input error\n";
    }

    cv::VideoCapture cap(argv[1]);

    if (!cap.isOpened())
    {
        std::cout << "cant open video\n";
        return -1;
    }
    if (true)
    {

        for (size_t i = 0; i < 1000; i++)
        {

            cv::Mat img;
            cap >> img;
            if (img.empty())
            {
                return 0;
            }

            cv::imwrite("./temp/" + std::to_string(i) + ".png", img);
        }

        return 0;
    }
    cap.release();

    return 0;
}
