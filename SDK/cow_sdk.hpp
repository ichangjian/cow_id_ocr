#include "../log/log.h"
#include "../NET/net.hpp"
#include "../UVC/capture.hpp"
#include "../OCR/cow_id.hpp"
#include <string>
#include <opencv2/opencv.hpp>
#include <thread>
#include <atomic>
class COW
{
private:
    int m_change_num;
    std::string m_change_id;
    std::string m_corner_file;
    int m_camera_index;
    std::string m_camera_file;
    NetUDP m_client;
    COWID m_ocr_id;
    bool m_test_model;
    bool m_init_flag;
    bool m_heartbeat_flag;
    int m_heartbeat_slp;
    bool m_open_camera_flag;
    int m_window_size;
    std::string m_video_file;
    std::thread m_measurement_heartbeat;
    std::thread m_measurement_cowid;
    std::atomic<bool> m_release_flag;
    cv::VideoCapture m_cap;
#ifdef _ANDROID_
    Capture *m_cap_uvc;
#endif
    std::list<std::string> m_id_que;
    bool m_send_id_flag;
    cv::Mat captureImage();
    std::string recogizeImage(const cv::Mat &_image);

    bool run();
    bool sendHeartbeat();
    bool sendCowID();
    void threadSleep(int sec, int nsec);
    double getCurrentTime();
    void release();

public:
    COW(std::string _file);
    ~COW();
    bool init();
    bool openCamera();
    bool testVideo(std::string _video_file);
    const char *getVersion();
};
