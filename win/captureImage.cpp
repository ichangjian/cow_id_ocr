#include <opencv2/opencv.hpp>
#include "cow_id.hpp"
using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
    int num = 0;
    if (argc == 2)
    {
        if (argv[1][0] == '1')
        {
            num = 1;
        }
    }
    std::cout << "camera " << num << "\n";
    cv::VideoCapture cap(num);

    if (!cap.isOpened())
    {
        std::cout << "cant open camera\n";
        return -1;
    }
    cap.set(CAP_PROP_FRAME_WIDTH, 1920);
    cap.set(CAP_PROP_FRAME_HEIGHT, 1080);
    int width = cap.get(CAP_PROP_FRAME_WIDTH);   //帧宽度
    int height = cap.get(CAP_PROP_FRAME_HEIGHT); //帧高度

    cout << "image width " << width << endl;
    cout << "image height " << height << endl;

    if (true)
    {

        for (size_t i = 0; i < 100; i++)
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
