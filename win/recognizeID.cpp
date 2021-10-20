#include <opencv2/opencv.hpp>
#include "cow_id.hpp"
using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
    cv::FileStorage fs("./corner.yaml", cv::FileStorage::READ);
    if (!fs.isOpened())
    {
        std::cout << "cant open corner.yaml \n";
        return -1;
    }
    int cn = fs["camera"];

    fs.release();
    cv::VideoCapture cap(cn);

    if (!cap.isOpened())
    {
        std::cout << "cant open camera\n";
        return -1;
    }
    cap.set(CAP_PROP_FRAME_WIDTH,1920);
    cap.set(CAP_PROP_FRAME_HEIGHT,1080);
    int width = cap.get(CAP_PROP_FRAME_WIDTH);       //帧宽度
    int height = cap.get(CAP_PROP_FRAME_HEIGHT);     //帧高度

    cout << "image width" << width << endl;
    cout << "image height" << height << endl;

    COWID cow;
    cow.initROI("./corner.yaml");

    Mat frame;
    while (1)
    {
        cap >> frame; //等价于cap.read(frame);
        if (frame.empty())
            break;
        imshow("video", frame);
        std::string id;
        frame=imread("./test.jpg");
        cow.getCowID(frame, id);

        if (waitKey(20) > 0)
            break;
    }
    cap.release();

    return 0;
}
