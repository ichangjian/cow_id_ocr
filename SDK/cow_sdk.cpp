#include "cow_sdk.hpp"
#include "build_version.h"

COW::COW(std::string _file)
{
    m_corner_file = _file;
    m_camera_index = 0;
    m_camera_file = "/dev/video";
    m_test_model = false;
    m_init_flag = false;
    m_heartbeat_flag = true;
    m_open_camera_flag = false;
    m_send_id_flag = true;
    m_release_flag = false;
    m_heartbeat_slp = 60;
    m_change_num = 0;
    m_change_id = "";
#ifdef _ANDROID_
    m_window_size = 3;
#else
    m_window_size = 4;
#endif

    m_measurement_heartbeat = std::thread(&COW::sendHeartbeat, this);
    m_measurement_cowid = std::thread(&COW::sendCowID, this);
}

COW::~COW()
{
    LOGD("end");
    m_release_flag = true;
    if (m_measurement_heartbeat.joinable())
    {
        LOGD("closing,wait %ds", m_heartbeat_slp);
        m_measurement_heartbeat.join();
    }
    if (m_measurement_cowid.joinable())
    {
        m_measurement_cowid.join();
    }
#ifdef _ANDROID_
    delete m_cap_uvc;
#endif
}

void COW::release()
{
    LOGD("release");
    m_release_flag = true;
}

bool COW::sendHeartbeat()
{
    try
    {
        double cur_tm = 0;
        long spend_tm = 0;

        while (true)
        {
            if (m_init_flag)
            {
                if (m_release_flag)
                    break;
                if (m_heartbeat_flag)
                {
                    m_heartbeat_flag = false;
                    LOGD("heartbeat send");
                    cur_tm = getCurrentTime();
                    if (!m_test_model)
                    {
                        m_client.sendHeartbeat();
                    }
                }
                else
                {
                    LOGD("heartbeat sleep %ds", m_heartbeat_slp);
                    threadSleep(m_heartbeat_slp, 0);
                    spend_tm = getCurrentTime() - cur_tm;
                    LOGD("heartbeat spend %ds", spend_tm);
                    if (spend_tm > 10 * 60)
                    {
                        m_heartbeat_flag = true;
                        LOGD("heartbeat wake up");
                    }
                }
            }
            else
            {
                LOGD("heartbeat sleep for init");
                threadSleep(0, 0.1E9);
                LOGD("heartbeat sleep for init2");
            }
        } //while
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        LOGE("heartbeat crash");
        return false;
    }

    return true;
}

bool COW::sendCowID()
{
    try
    {
        while (true)
        {
            if (m_init_flag && m_open_camera_flag)
            {
                if (m_release_flag)
                    break;
                cv::Mat image = captureImage();
                if (image.empty())
                {
                    continue;
                }
                else
                {
                    std::string id = recogizeImage(image);
                    if (id.size() > 0)
                    {
                        if (!m_test_model)
                        {
                            m_client.sendCowID(id);
                        }
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            else
            {
                LOGD("cowid sleep for init");
                threadSleep(0, 0.1E9);
                LOGD("cowid sleep for init2");
            }
        } //while
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        LOGE("cowid crash");
        return false;
    }

    return true;
}

cv::Mat COW::captureImage()
{
    // LOGD("capture image");
    cv::Mat image;
    if (m_test_model)
    {
        m_cap >> image;
        if (image.empty())
        {
            LOGD("finish video %s", m_video_file.c_str());
            release();
            return cv::Mat();
        }
    }
    else
    {
#ifdef _ANDROID_
        LOGD("capture uvc image");
        m_cap_uvc->captureMat(image);
#else
        m_cap >> image;
#endif
    }
    return image;
}

std::string COW::recogizeImage(const cv::Mat &_image)
{
    std::string id = "";
    if (m_ocr_id.getCowID(_image, id) && id.size() > 1)
    {
        if (m_send_id_flag)
        {
            m_id_que.pop_front();
            m_id_que.push_back(id);
        }
        else
        {
            if (id == m_id_que.back())
            {
                return "";
            }
            else
            {
                std::cout<<m_change_num<<" "<<m_change_id<<" "<<id<<"\n";
                if (m_change_id == id)
                    m_change_num++;
                else
                {
                    m_change_id = id;
                    m_change_num = 0;
                }
                if (m_change_num > 2)
                {
                    m_send_id_flag = true;
                    std::cout << m_id_que.back() << " " << id << "\n";
                    m_id_que.pop_front();
                    m_id_que.push_back(id);
                }
            }
        }
    }
    else
    {
        return "";
    }

    int idx = 0;
    for (std::list<std::string>::iterator it = m_id_que.begin(); it != m_id_que.end(); ++it)
    {
        // cout << idx << " " << *it << " " << id << " " << (*it == id) << "\n";
        if (*it == id)
        {
            idx++;
        }
    }
    if (idx == m_window_size)
    {
        m_change_num = 0;
        LOGD("%s OK", id.c_str());
        m_send_id_flag = false;
        return id;
    }
    else
    {
        return "";
    }
}

void COW::threadSleep(int sec, int nsec)
{
    struct timespec sleepTime;
    struct timespec returnTime;
    sleepTime.tv_sec = sec;
    sleepTime.tv_nsec = nsec;
    nanosleep(&sleepTime, &returnTime);
}

double COW::getCurrentTime()
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::nanoseconds ns = now.time_since_epoch();
    std::chrono::milliseconds millsec =
        std::chrono::duration_cast<std::chrono::milliseconds>(ns);

    double ms = millsec.count() / 1000.0;
    return ms;
}

bool COW::init()
{
    try
    {
        LOGD("load config file %s", m_corner_file.c_str());
        cv::FileStorage fs(m_corner_file, cv::FileStorage::READ);
        if (!fs.isOpened())
        {
            LOGE("cant open %s", m_corner_file.c_str());
            return false;
        }
        if (m_client.CreateSocket() < 0)
        {
            LOGE("Create socket failed");
            return false;
        }
        if (!m_ocr_id.initROI(m_corner_file))
        {
            LOGE("init OCR failed");
            return false;
        }

        m_camera_index = fs["camera"];
        m_camera_file += std::to_string(m_camera_index);
        LOGD("set camera index %d", m_camera_index);
        if (!m_test_model)
        {
            std::string ip = fs["IP"];
            int port = fs["port"];
            LOGD("UDP connect %s:%d", ip.c_str(), port);
            if (!m_client.Connect(ip.c_str(), port))
            {
                LOGE("UDP connect error");
                return false;
            }
        }
        m_init_flag = true;
        for (size_t i = 0; i < m_window_size; i++)
        {
            m_id_que.push_back("#");
        }
        return openCamera();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        LOGE("init crash");
        return false;
    }

    return true;
}

bool COW::testVideo(std::string _video_file)
{
    LOGD("test video model %s", _video_file.c_str());
    m_video_file = _video_file;
    m_test_model = true;
    m_heartbeat_slp = 10;
    return true;
}

const char *COW::getVersion()
{
    static std::string version = "COWID_0.1Beta_";
    version += V_GIT_INFO;
    version += "_";
    version += V_BUILD_TIME;
    return version.c_str();
}

bool COW::openCamera()
{
    if (m_init_flag)
    {

#ifdef _ANDROID_
        LOGD("open camera %s", m_camera_file.c_str());
        m_cap_uvc = new Capture(m_camera_file);
        if (!m_cap_uvc->initCamera())
        {
            LOGD("open camera failed");
            return false;
        }
        m_cap_uvc->printCameraformats();
        // m_cap_uvc->setCamera(MJPG_FORMAT, 1920, 1080, 30);
#else
        if (m_test_model)
        {
            LOGD("open video %s", m_video_file.c_str());
            m_cap.open(m_video_file);
        }
        else
        {
            m_cap.open(m_camera_index);

            m_cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
            m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);

            int width = m_cap.get(cv::CAP_PROP_FRAME_WIDTH);   //帧宽度
            int height = m_cap.get(cv::CAP_PROP_FRAME_HEIGHT); //帧高度
            LOGD("image width %d", width);
            LOGD("image height %d", height);
        }
        if (!m_cap.isOpened())
        {
            LOGD("open camera failed");
            return false;
        }
#endif
        m_open_camera_flag = true;
        LOGD("open camera finish");
        return true;
    }
    else
    {
        return false;
    }
}