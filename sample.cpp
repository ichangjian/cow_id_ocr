#include <opencv2/opencv.hpp>
#include "cow_id.hpp"
using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
    COWID cow;

    clock_t startTime, endTime;
    startTime = clock(); //计时开始
    Mat image;
    std::string file("../corner1.yaml");
    if (argc > 1)
    {
        file = argv[2];
        image = imread(argv[1]);
    }
    else
        image = imread("../IMG20210928132619.jpg");
    // resize(image, image, cv::Size(), 0.5, 0.5);
    endTime = clock(); //计时结束
    std::cout << "read image time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
    startTime = clock(); //计时开始
    std::string id;
    for (size_t i = 0; i < 100; i++)
    {
        cow.getCowID(image, id);
        std::cout << id << "\n";
    }

    endTime = clock(); //计时结束
    std::cout << "OCR run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
}
