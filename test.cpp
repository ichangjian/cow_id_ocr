#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;

void LOGE(string s)
{
    cout<<s<<"\n";
}
void LOGI(string s)
{
    cout << s << "\n";
    
}
int main()
{
    Mat image = imread("../IMG20210928132544.jpg");


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
    std::cout<<"hi\n";
    return 0;

}