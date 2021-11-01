#include <opencv2/opencv.hpp>
#include "cow_id.hpp"
#include <queue>
using namespace std;
using namespace cv;

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cout << "input error\n";
        return -1;
    }
    std::cout << "corner path=<" << argv[1] << ">\n";
    std::cout << "vedio path=<" << argv[2] << ">\n";
    cv::FileStorage fs(argv[1], cv::FileStorage::READ);
    if (!fs.isOpened())
    {
        std::cout << "cant open corner.yaml \n";
        return -1;
    }
    int cn = fs["camera"];

    fs.release();
    // cv::VideoCapture cap(cn);
    cv::VideoCapture cap(argv[2]);

    if (!cap.isOpened())
    {
        std::cout << "cant open vedio\n";
        return -1;
    }
    int width = cap.get(CAP_PROP_FRAME_WIDTH);   //帧宽度
    int height = cap.get(CAP_PROP_FRAME_HEIGHT); //帧高度

    cout << "image width" << width << endl;
    cout << "image height" << height << endl;

    COWID cow;
    cow.initROI(argv[1]);
    namedWindow("video", cv::WINDOW_GUI_EXPANDED);
    resizeWindow("video", 960, 540);

    Mat frame;
    std::list<std::string> id_que;
    id_que.push_back("#");
    id_que.push_back("#");
    id_que.push_back("#");
    id_que.push_back("#");
    bool send_flag = true;
    while (1)
    {
        cap >> frame; //等价于cap.read(frame);
        if (frame.empty())
            break;
        imshow("video", frame);
        std::string id;
        // frame = imread("./test.jpg");
        if (cow.getCowID(frame, id) && id.size() > 2)
        {
            if (send_flag)
            {
                id_que.pop_front();
                id_que.push_back(id);
            }
            else
            {
                if (id == id_que.back())
                {
                    continue;
                }
                else
                {
                    send_flag = true;
                    std::cout<<id_que.back()<<" "<<id<<"\n";
                    id_que.pop_front();
                    id_que.push_back(id);
                }
            }
        }
        else
        {
            continue;
        }

        if (id_que.size() == 4)
        {
            int idx = 0;
            for (std::list<std::string>::iterator it = id_que.begin(); it != id_que.end(); ++it)
            {
                // cout << idx << " " << *it << " " << id << " " << (*it == id) << "\n";
                if (*it == id)
                {
                    idx++;
                }
            }
            if (idx == 4)
            {
                std::cout << id << "------------------OK\n";
                send_flag = false;
            }
            // ids.clear();
        }

        if (waitKey(10) > 0)
            break;
    }
    cap.release();

    return 0;
}
