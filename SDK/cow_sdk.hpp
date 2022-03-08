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
    SendData m_client;
    COWID m_ocr_id;
    bool m_test_model;
    bool m_init_flag;
    bool m_heartbeat_flag;
    int m_heartbeat_slp;
    bool m_open_camera_flag;
    int m_window_size;
    std::string m_video_file;
    std::thread m_measurement_heartbeat;
    std::thread m_measurement_imagebuff;
    std::thread m_measurement_idbuff;
    std::thread m_measurement_cowid;
    std::atomic<bool> m_release_flag;
    cv::VideoCapture m_cap;
#ifdef _ANDROID_
    Capture *m_cap_uvc;
#endif
    std::list<std::string> m_id_que;
    bool m_send_id_flag;
    std::string m_last_send_id;
    cv::Mat captureImage();
    std::string recogizeImage(const cv::Mat &_image);
    std::mutex m_mtx_capture;

    std::queue<cv::Mat> m_image_buff;
    std::queue<std::pair<time_t, std::string>> m_id_buff;
    std::mutex m_mtx_image_buff;
    std::mutex m_mtx_id_buff;
    bool processImageBuff();
    bool processIdBuff();

    bool run();
    bool sendHeartbeat();
    bool sendCowID();
    void threadSleep(int sec, int nsec);
    double getCurrentTime();
    std::string getCurrentTimeString();
    bool checkFileExist(const std::string _path_file);
    bool removeLogfile(int _day);

    std::string m_path_jam_file;
    bool m_save_jam_flag;
    bool jam2file(time_t _t, std::string _id);
    bool loadJam();

public:
    COW(std::string _file);
    ~COW();
    void release();
    bool init();
    bool reset();
    bool openCamera();
    bool testVideo(std::string _video_file);
    const char *getVersion();
};
