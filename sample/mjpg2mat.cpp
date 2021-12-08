#include "capture.hpp"

int main(int argc, char *argv[])
{
    Capture cap("/dev/video0");
    cap.initCamera();
    // cap.setCamera(MJPG_FORMAT, 1920, 1080, 30);
    cap.printCameraformats();
    cv::Mat image;

    clock_t startTime, endTime;
    startTime = clock(); //计时开始
    endTime = clock();   //计时开始
    std::vector<cv::Mat> images(100);
    for (size_t i = 0; i < 100; i++)
    {
        startTime = clock(); //计时开始

        cap.captureMat(image);
        endTime = clock(); //计时结束
        std::cout << "read image time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
        images[i] = image;
        // cv::imwrite("./temp/" + std::to_string(i) + ".jpg", image);
    }

    for (size_t i = 0; i < 100; i++)
    {
        startTime = clock(); //计时开始
        cv::imwrite("./temp/" + std::to_string(i) + ".jpg", images[i]);

        endTime = clock(); //计时结束
        std::cout << "write image time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
    }
    return 0;
}
