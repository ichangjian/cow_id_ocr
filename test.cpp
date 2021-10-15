#include <opencv2/opencv.hpp>
#include "rec_num.h"
using namespace std;
using namespace cv;

void LOGE(string s)
{
    cout << s << "\n";
}
void LOGI(string s)
{
    cout << s << "\n";
}

void openCm()
{
    VideoCapture cap;
    for (int i = 0; i < 30; i++)
    {
        cap.open("/dev/video"+to_string(i)); //打开视频,以上两句等价于VideoCapture cap("E://2.avi");

        //cap.open("http://www.laganiere.name/bike.avi");//也可以直接从网页中获取图片，前提是网页有视频，以及网速够快
        if (!cap.isOpened()) //如果视频不能正常打开则返回
        {
            cout << i<<"cant open\n";
        }
        else
        {
            cout << i << "ok\n";
            // break;
        }
    }
    if (!cap.isOpened()) //如果视频不能正常打开则返回
    {
        cout << "cant open\n";
    }
    Mat frame;
    for (size_t i = 0; i < 30; i++)
    {

        cap >> frame;
    }

    imwrite("cm.png", frame);
    cap.release();
    return;
}
int main(int argc, char **argv)
{
    openCm();

    return 0;
    clock_t startTime, endTime;
    startTime = clock(); //计时开始
    Mat image;
    std::string file("../corner1.yaml");
    if (argc > 1)
    {
        file = argv[2];
        image = imread(argv[1]);
        startTime = clock(); //计时开始
        for (size_t i = 0; i < 1000; i++)
        {

            getCowID(image, file); /* code */
        }

        // getCowID(image, file);
        endTime = clock(); //计时结束
        std::cout << "OCR run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
    }
    else
        image = imread("../IMG20210928132619.jpg");
    // resize(image, image, cv::Size(), 0.5, 0.5);
    endTime = clock(); //计时结束
    std::cout << "read image time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
    startTime = clock(); //计时开始

    getCowID(image, file);
    endTime = clock(); //计时结束
    std::cout << "OCR run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
}

int maina(int argc, char **argv)
{
    Mat image;
    if (argc > 1)
    {

        image = imread(argv[1]);
    }
    else
        image = imread("../dst.png");

    if (image.empty())
    {
        LOGE("image error");
        return false;
    }
    LOGI("cvt data");

    Mat hsv;
    // GaussianBlur(image,image,Size(11,11),0,0);
    cv::cvtColor(image, hsv, COLOR_BGR2HSV);
    Mat HSV[3];
    cv::split(image, HSV);
    Mat G = HSV[1];
    cv::dilate(G, G, cv::Mat(11, 11, CV_8UC1, Scalar(1)));
    Mat W = G > 220;
    imwrite("G.jpg", W);
    vector<vector<Point>> contours;
    vector<Vec4i> hierarcy;
    findContours(W, contours, hierarcy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    std::vector<cv::Rect> rect_num;
    for (int i = 0; i < contours.size(); i++)
    {
        Rect rect = boundingRect(contours[i]);
        int center_y = rect.y + rect.height / 2;
        int center_x = rect.x + rect.width / 2;
        if (center_x > image.cols / 2 && center_y < image.rows / 2 + image.rows / 10 && center_y > image.rows / 2 - image.rows / 10)
        {
            rect_num.push_back(rect);
            rectangle(image, rect, Scalar(255, 0, 0), 5, 8);
        }

        drawContours(image, contours, i, Scalar(255, 255, 255), 5, 8);
    }
    myNumberSort(rect_num);
    for (size_t i = 0; i < rect_num.size(); i++)
    {
        Rect rect = rect_num[i];
        // Mat W = HSV[1] > 220;
        int num = myDiscern(W(rect));
        std::cout << num << "\n";
        putText(image, to_string(num), Point(rect.x, rect.y), 1, 11, Scalar(0, 0, 255), 5);
        imshow("a", image);
        waitKey(50);
    }

    waitKey(0);
    imwrite("ocr.png", image);
    return 0;
    cv::split(hsv, HSV);
    LOGI("splt data");

    Mat tmpH1, tmpH2, tmpH3;
    //blue
    inRange(HSV[0], Scalar(85, 0.0, 0, 0), Scalar(155, 0.0, 0, 0), tmpH1);
    inRange(HSV[1], Scalar(43.0, 0.0, 0, 0), Scalar(255, 0.0, 0, 0), tmpH2);
    inRange(HSV[2], Scalar(46, 0.0, 0, 0), Scalar(255.0, 0.0, 0, 0), tmpH3);

    bitwise_and(tmpH3, tmpH2, tmpH2);
    bitwise_and(tmpH1, tmpH2, tmpH1);
    Mat blue = tmpH1.clone();
    LOGI("blue color");

    //black
    inRange(HSV[0], Scalar(0, 0.0, 0, 0), Scalar(180, 0.0, 0, 0), tmpH1);
    inRange(HSV[1], Scalar(0.0, 0.0, 0, 0), Scalar(255, 0.0, 0, 0), tmpH2);
    inRange(HSV[2], Scalar(0, 0.0, 0, 0), Scalar(46.0, 0.0, 0, 0), tmpH3);

    bitwise_and(tmpH3, tmpH2, tmpH2);
    bitwise_and(tmpH1, tmpH2, tmpH1);
    Mat black = tmpH1.clone();
    LOGI("black color");

    //white
    inRange(HSV[0], Scalar(0, 0.0, 0, 0), Scalar(180, 0.0, 0, 0), tmpH1);
    inRange(HSV[1], Scalar(0.0, 0.0, 0, 0), Scalar(70, 0.0, 0, 0), tmpH2);
    inRange(HSV[2], Scalar(88, 0.0, 0, 0), Scalar(255, 0.0, 0, 0), tmpH3);

    bitwise_and(tmpH3, tmpH2, tmpH2);
    bitwise_and(tmpH1, tmpH2, tmpH1);
    Mat white = tmpH1.clone();
    LOGI("white color");

    Mat bw = black + blue + white;
    bw /= 255;

    if (true)
    {
        HSV[0] = bw;
        HSV[1] = bw;
        HSV[2] = bw;
        Mat BW;
        cv::merge(HSV, 3, BW);
        imwrite("red_yellow_green.jpg", image - image.mul(BW));
    }

    namedWindow("a", WINDOW_NORMAL);
    imshow("a", black);
    waitKey();
    std::cout << "hi\n";
    return 0;
}