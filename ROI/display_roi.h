#pragma once
#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>
#include <ctime>
using namespace std;
using namespace cv;

vector<Point2f> conersSrcPosition;
vector<Point2f> conersDstPosition;
cv::Point2f cornerMidU;
cv::Point2f cornerMidD;
vector<int> horizLine;
int drawHorizLineNum = 3;

bool finishFlag = false;

void onMouse(int event, int x, int y, int flags, void *param)
{

    switch (event)
    {
    case cv::EVENT_LBUTTONDOWN:

        if (conersSrcPosition.size() == 4)
        {
            int minLengthH = 512;
            int minIndexH = 0;
            for (size_t i = 0; i < conersSrcPosition.size(); i++)
            {
                int length = abs(x - conersSrcPosition[i].x) + abs(y - conersSrcPosition[i].y);
                if (length < minLengthH)
                {
                    minLengthH = length;
                    minIndexH = i;
                }
            }
            if (minLengthH != 512)
            {
                conersSrcPosition[minIndexH] = Point2f(x, y);
            }
            break;
        }
        else
        {
            conersSrcPosition.push_back(Point2f(x, y));
        }
        //cout << x << "\t" << y << endl;
        break;
    default:
        break;
    }
}

std::string getCT()
{
    time_t now = time(0);
    tm *ltm = localtime(&now);
    std::string s = "";
    s += std::to_string(1900 + ltm->tm_year) + "-" + std::to_string(1 + ltm->tm_mon) + "-" + std::to_string(ltm->tm_mday);
    s += " " + std::to_string(ltm->tm_hour) + ":" + std::to_string(ltm->tm_min) + ":" + std::to_string(ltm->tm_sec);
    return s;
}

int start(std::string file)
{
    finishFlag = false;
    conersSrcPosition.clear();
    conersDstPosition.clear();
    Mat image = imread(file);
    if (image.empty())
    {
        std::cout << file << " 打开失败\n";
        return -1;
    }
RESET:
    namedWindow("getCorner", cv::WINDOW_GUI_EXPANDED);
    resizeWindow("getCorner", 800, 600);
    setMouseCallback("getCorner", onMouse);
    bool whileFlag = true;
    while (whileFlag)
    {
        Mat cornerImage = image.clone();
        for (size_t i = 0; i < conersSrcPosition.size(); i++)
        {
            circle(cornerImage, conersSrcPosition[i], 4, Scalar(0, 255, 0), 4);
            circle(cornerImage, conersSrcPosition[i], 20, Scalar(255, 255, 255), 4);
            circle(cornerImage, conersSrcPosition[i], 4, Scalar(255, 255, 255), 4);
            int size = 10;
            line(cornerImage, cv::Point(conersSrcPosition[i].x - size * 2, conersSrcPosition[i].y), cv::Point(conersSrcPosition[i].x - size * 5, conersSrcPosition[i].y), Scalar(255, 255, 255), 4, 8, 0);
            line(cornerImage, cv::Point(conersSrcPosition[i].x + size * 2, conersSrcPosition[i].y), cv::Point(conersSrcPosition[i].x + size * 5, conersSrcPosition[i].y), Scalar(255, 255, 255), 4, 8, 0);

            line(cornerImage, cv::Point(conersSrcPosition[i].x, conersSrcPosition[i].y - size * 2), cv::Point(conersSrcPosition[i].x, conersSrcPosition[i].y - size * 5), Scalar(255, 255, 255), 4, 8, 0);
            line(cornerImage, cv::Point(conersSrcPosition[i].x, conersSrcPosition[i].y + size * 2), cv::Point(conersSrcPosition[i].x, conersSrcPosition[i].y + size * 5), Scalar(255, 255, 255), 4, 8, 0);
        }
        if (conersSrcPosition.size() > 1)
        {
            for (size_t i = 0; i < conersSrcPosition.size() - 1; i++)
            {
                line(cornerImage, conersSrcPosition[i], conersSrcPosition[i + 1], Scalar(0, 255, 0), 4);
            }
            if (conersSrcPosition.size() == 4)
            {

                line(cornerImage, conersSrcPosition[0], conersSrcPosition[3], Scalar(0, 255, 0), 4);
            }
        }

        imshow("getCorner", cornerImage);
        int pressedKey = waitKey(100);

        switch (pressedKey)
        {
        case 32:
            if (conersSrcPosition.size() == 4)
            {
                whileFlag = false;
            }
            break;
        case 27:
            destroyWindow("getCorner");
            return 0;
        case 'r':
            conersSrcPosition.clear();
            break;
        default:
            break;
        }
    }
    Mat dst;
    if (conersSrcPosition.size() == 4)
    {

        for (size_t i = 0; i < conersSrcPosition.size(); i++)
        {
            cout << conersSrcPosition[i] << endl;
        }
        Rect bRect = boundingRect(conersSrcPosition);
        conersDstPosition.clear();
        conersDstPosition.push_back(Point2f(0, 0));
        conersDstPosition.push_back(Point2f(0, bRect.height));
        conersDstPosition.push_back(Point2f(bRect.width, bRect.height));
        conersDstPosition.push_back(Point2f(bRect.width, 0));

        Mat pose3 = getPerspectiveTransform(conersSrcPosition, conersDstPosition);

        warpPerspective(image, dst, pose3, bRect.size());
        Mat inpose;
        invert(pose3, inpose);
        Mat mid_u(3, 1, pose3.type(), Scalar(1));
        Mat mid_d(3, 1, pose3.type(), Scalar(1));
        mid_u.at<double>(0) = bRect.width / 2;
        mid_u.at<double>(1) = 0;

        mid_d.at<double>(0) = bRect.width / 2;
        mid_d.at<double>(1) = bRect.height;
        mid_u = inpose * mid_u;
        mid_d = inpose * mid_d;

        mid_u /= mid_u.at<double>(2);
        mid_d /= mid_d.at<double>(2);

        cornerMidU = cv::Point2f(mid_u.at<double>(0), mid_u.at<double>(1));
        cornerMidD = cv::Point2f(mid_d.at<double>(0), mid_d.at<double>(1));
    }
    else
    {
        return -1;
    }

    destroyWindow("getCorner");
    namedWindow("dst", WINDOW_NORMAL);
    resizeWindow("dst", 800, 600);
    // setMouseCallback("dst", onMouse);
    horizLine.clear();
    int gapH = dst.rows / drawHorizLineNum;

    for (size_t i = 0; i <= drawHorizLineNum; i++)
    {
        horizLine.push_back(gapH * i);
    }

    cv::FileStorage fs("corner.yaml", cv::FileStorage::WRITE);
    while (true)
    {
        Mat drawImg = dst.clone();
        for (size_t i = 0; i < horizLine.size(); i++)
        {
            line(drawImg, Point(0, horizLine[i]), Point(dst.cols, horizLine[i]), Scalar(255, 0, 0), 4);
        }

        if (finishFlag)
        {
            putText(drawImg, "Processing", Point(dst.cols / 4, dst.rows / 5), 1, 8, Scalar(200, 0, 200), 10);
            putText(drawImg, "is", Point(dst.cols / 4, dst.rows / 3), 1, 8, Scalar(200, 0, 200), 10);
            putText(drawImg, "complete", Point(dst.cols / 4, dst.rows / 2), 1, 8, Scalar(200, 0, 200), 10);
        }
        imshow("dst", drawImg);
        int pressedKey = waitKey(10);
        switch (pressedKey)
        {
        case 27:

            destroyWindow("dst");
            return 0;
            break;

        case 's':
            fs << "version" << 1.0;
            fs << "time" << getCT();
            fs << "camera" << 0;
            fs.writeComment("牛圏号识别 cow_pen 为1启用 为0关闭");
            fs << "cow_pen" << 1;
            fs << "LUX" << conersSrcPosition[0].x;
            fs << "LUY" << conersSrcPosition[0].y;
            fs << "LDX" << conersSrcPosition[1].x;
            fs << "LDY" << conersSrcPosition[1].y;
            fs << "MUX" << cornerMidU.x;
            fs << "MUY" << cornerMidU.y;
            fs << "MDX" << cornerMidD.x;
            fs << "MDY" << cornerMidD.y;
            fs << "RDX" << conersSrcPosition[2].x;
            fs << "RDY" << conersSrcPosition[2].y;
            fs << "RUX" << conersSrcPosition[3].x;
            fs << "RUY" << conersSrcPosition[3].y;
            fs.writeComment("网关通信");
            fs << "IP"
               << "111.113.25.66";
            fs << "port" << 18093;

            fs.writeComment("16位数字");
            fs << "deviceID"
               << "0111111111111111";

            fs.writeComment("8位数字");
            fs << "gatewayID"
               << "00000000";

            fs.release();
            imwrite("dst.png", dst);
            finishFlag = true;

            destroyWindow("dst");
            return 2;
        case 'b':
            destroyWindow("dst");
            goto RESET;

        default:
            break;
        } //switch

    } //while
}
