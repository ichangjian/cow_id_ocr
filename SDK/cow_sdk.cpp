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
    m_last_send_id = "";

    m_window_size = 3;


    // m_measurement_heartbeat = std::thread(&COW::sendHeartbeat, this);
    m_measurement_cowid = std::thread(&COW::sendCowID, this);
    m_measurement_imagebuff = std::thread(&COW::processIdBuff, this);
    m_measurement_idbuff = std::thread(&COW::processImageBuff, this);
}

COW::~COW()
{
    LOGD("end");
    m_release_flag = true;
    // if (m_measurement_heartbeat.joinable())
    // {
    //     LOGD("closing,wait %ds", m_heartbeat_slp);
    //     m_measurement_heartbeat.join();
    // }
    if (m_measurement_imagebuff.joinable())
    {
        m_measurement_imagebuff.join();
    }
    if (m_measurement_idbuff.joinable())
    {
        m_measurement_idbuff.join();
    }
    if (m_measurement_cowid.joinable())
    {
        m_measurement_cowid.join();
    }

    // #ifdef _ANDROID_
    delete m_cap_uvc;
    // #endif
}

void COW::release()
{
    LOGD("release");
    m_release_flag = true;
}
bool COW::processImageBuff()
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
                    LOGD("image empty");
                    continue;
                }
                else
                {
                    m_mtx_image_buff.lock();
                    m_image_buff.push(image);
                    LOGD("push image buff size %d", m_image_buff.size());
                    if (m_image_buff.size() > 10)
                    {
                        m_image_buff.pop();
                    }

                    m_mtx_image_buff.unlock();
                }
            }
            else
            {
                LOGD("image buff sleep for init");
                threadSleep(1, 0);
            }
        } //while
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        LOGE("image buff crash");
        return false;
    }

    return true;
}

bool COW::sendCowID()
{
    try
    {
        double heart_cur_tm = 0;
        double save_cur_tm = 0;
        long heart_spend_tm = 0;
        long save_spend_tm = 0;
        while (true)
        {
            if (m_init_flag)
            {
                if (m_release_flag)
                    break;

                std::pair<time_t, std::string> time_id;
                m_mtx_id_buff.lock();
                if (m_id_buff.size() > 0)
                {
                    time_id = m_id_buff.front();
                    m_id_buff.pop();
                    LOGD("pop id buff size %d", m_id_buff.size());
                    m_mtx_id_buff.unlock();
                }
                else
                {
                    m_mtx_id_buff.unlock();
                    threadSleep(2, 0);
                }

                if (!m_test_model && time_id.second.size() > 1)
                {
                    std::string id = time_id.second;
                    size_t idex = id.find('#');
                    if (idex != std::string::npos)
                    {
                        std::string r_pen = id.substr(0, idex);
                        std::string g_id = id.substr(idex + 1, id.length() - idex - 1);
                        if (m_last_send_id != g_id)
                        {
                            m_last_send_id = g_id;
                            if (!m_client.sendCowIDPen(time_id.first, g_id, r_pen))
                            {
                                LOGD("send error %s", id.c_str());
                                m_mtx_id_buff.lock();
                                m_id_buff.push(time_id);
                                // LOGD("push id buff size %d", m_id_buff.size());
                                m_mtx_id_buff.unlock();
                                threadSleep(2, 0);
                            }
                            else
                            {
                                LOGD("send %s OK", id.c_str());
                            }
                        }
                    }
                    else
                    {
                        if (m_last_send_id != id)
                        {
                            m_last_send_id = id;
                            if (!m_client.sendCowIDPen(time_id.first, id, ""))
                            {
                                LOGD("send error %s", id.c_str());
                                m_mtx_id_buff.lock();
                                m_id_buff.push(time_id);
                                // LOGD("push id buff size %d", m_id_buff.size());
                                m_mtx_id_buff.unlock();
                            }
                            else
                            {
                                LOGD("send %s OK", id.c_str());
                            }
                        }
                    }
                }

                if (m_heartbeat_flag)
                {
                    m_heartbeat_flag = false;
                    LOGD("heartbeat send");
                    heart_cur_tm = getCurrentTime();
                    save_cur_tm = heart_cur_tm;
                    if (!m_test_model)
                    {
                        m_client.sendHeartbeat();
                        removeLogfile(7);
                    }
                }
                else
                {
                    heart_spend_tm = getCurrentTime() - heart_cur_tm;
                    if (heart_spend_tm > 10 * 60)
                    {
                        m_heartbeat_flag = true;
                        LOGD("heartbeat wake up");
                    }
                    save_spend_tm = getCurrentTime() - save_cur_tm;
                    if (save_spend_tm > 60)
                    {
                        save_cur_tm = getCurrentTime();
                        if (__SAVE_DATA__ > 1)
                        {
                            cv::Mat image = captureImage();
                            if (!image.empty())
                            {
                                cv::imwrite("/sdcard/Download/image/" + getCurrentTimeString() + ".jpg", image);
                            }
                        }
                        else
                        {
                            static int save_image_index = 0;
                            if (save_image_index < 100)
                            {
                                cv::Mat image = captureImage();
                                cv::imwrite("/sdcard/Download/image/" + std::to_string(save_image_index++) + ".jpg", image);
                            }
                        }
                    }
                }
            }
            else
            {
                LOGD("send cowid sleep for init");
                threadSleep(1, 0);
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
                    if (__SAVE_DATA__ > 1)
                    {
                        cv::Mat image = captureImage();
                        if (!image.empty())
                        {
                            cv::imwrite("/sdcard/Download/image/" + getCurrentTimeString() + ".jpg", image);
                        }
                    }
                    else
                    {
                        static int save_image_index = 0;
                        if (save_image_index < 100)
                        {
                            cv::Mat image = captureImage();
                            cv::imwrite("/sdcard/Download/image/" + std::to_string(save_image_index++) + ".jpg", image);
                        }
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

bool COW::processIdBuff()
{
    try
    {
        while (true)
        {
            if (m_init_flag && m_open_camera_flag)
            {
                if (m_release_flag)
                    break;
                cv::Mat image;
                m_mtx_image_buff.lock();
                if (m_image_buff.size() > 0)
                {
                    image = m_image_buff.front().clone();
                    m_image_buff.front().release();
                    m_image_buff.pop();
                    // LOGD("pop image buff size %d", m_image_buff.size());
                    m_mtx_image_buff.unlock();
                }
                else
                {
                    m_mtx_image_buff.unlock();
                    threadSleep(0, 0.3E9);
                }

                if (!image.empty())
                {
                    std::string id = recogizeImage(image);
                    if (id.size() > 0)
                    {
                        m_mtx_id_buff.lock();
                        m_id_buff.push(std::pair<time_t, std::string>(time(0), id));
                        // LOGD("push id buff size %d", m_id_buff.size());
                        m_mtx_id_buff.unlock();
                        if (__SAVE_DATA__ > 1)
                        {
                            cv::imwrite("/sdcard/Download/send/" + getCurrentTimeString() + "_" + id + ".jpg", image);
                        }
                        else
                        {
                            static int save_send_index = 0;
                            if (save_send_index < 100)
                            {
                                cv::putText(image, id, cv::Point(image.cols / 2, image.rows / 2), 1, 5, cv::Scalar(0, 0, 255), 3);
                                cv::imwrite("/sdcard/Download/send/" + std::to_string(save_send_index++) + ".jpg", image);
                            }
                        }
#ifdef RS_D455
                        if (m_depth_flag)
                        {
                            rs2::frameset data = m_pipe.wait_for_frames(); // Wait for next set of frames from the camera
                            rs2::depth_frame df = data.get_depth_frame();

                            rs2::frame depth = df.apply_filter(m_color_map);

                            // Query frame size (width and height)
                            const int w = depth.as<rs2::video_frame>().get_width();
                            const int h = depth.as<rs2::video_frame>().get_height();

                            // Create OpenCV matrix of size (w,h) from the colorized depth data
                            cv::Mat img_depth(cv::Size(w, h), CV_16UC1, (void *)df.get_data(), cv::Mat::AUTO_STEP);
#ifdef SHOW_IMAGE
                            cv::Mat rgb(cv::Size(w, h), CV_8UC3, (void *)depth.get_data(), cv::Mat::AUTO_STEP);
                            cv::imshow("Depth", rgb);
#endif
                            cv::imwrite("/sdcard/Download/depth/" + getCurrentTimeString() + "_" + id + ".tiff", img_depth);
                        }
#endif
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            else
            {
                LOGD("id buff sleep for init");
                threadSleep(0, 0.1E9);
            }
        } //while
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        LOGE("id buff crash");
        return false;
    }

    return true;
}

cv::Mat COW::captureImage()
{
    // LOGD("capture image");
    m_mtx_capture.lock();
    cv::Mat image;
    if (m_test_model)
    {
        m_cap >> image;
        if (image.empty())
        {
            LOGD("finish video %s", m_video_file.c_str());
            release();
            m_mtx_capture.unlock();
            return cv::Mat();
        }
        cv::waitKey(30);
    }
    else
    {
        // #ifdef _ANDROID_
        // LOGD("capture uvc image");
        m_cap_uvc->captureMat(image);
// #else
// m_cap >> image;
#ifdef SHOW_IMAGE
        cv::namedWindow("CowID", cv::WINDOW_GUI_EXPANDED);
        cv::resizeWindow("CowID", 960, 540);
        cv::imshow("CowID", image);
        cv::waitKey(1);
#endif

        // #endif
    }
    m_mtx_capture.unlock();
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
                std::cout << m_change_num << " " << m_change_id << " " << id << "\n";
                if (m_change_id == id)
                    m_change_num++;
                else
                {
                    m_change_id = id;
                    m_change_num = 0;
                }
                if (m_change_num > 1)
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
std::string COW::getCurrentTimeString()
{
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char buf[512];
    strftime(buf, 64, "%Y-%m-%d-%H-%M-%S-", ltm);

    int ms = 0;
    {
        std::chrono::system_clock::time_point now =
            std::chrono::system_clock::now();
        std::chrono::nanoseconds ns = now.time_since_epoch();
        std::chrono::milliseconds millsec =
            std::chrono::duration_cast<std::chrono::milliseconds>(ns);
        std::chrono::seconds sec =
            std::chrono::duration_cast<std::chrono::seconds>(ns);
        ms = millsec.count() - sec.count() * 1000;
    }
    std::string ct(buf);
    return ct + std::to_string(ms);
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
        // if (m_client.CreateSocket() < 0)
        // {
        //     LOGE("Create socket failed");
        //     return false;
        // }
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
            std::string url = fs["URL"];
            int port = fs["port"];
            if (url.size() > 3)
            {
                LOGD("UDP connect %s:%d", url.c_str(), port);
                if (!m_client.setURL(url.c_str(), port))
                {
                    LOGE("UDP connect error");
                    return false;
                }
            }
            else
            {
                LOGD("UDP connect %s:%d", ip.c_str(), port);
                if (!m_client.setIP(ip.c_str(), port))
                {
                    LOGE("UDP connect error");
                    return false;
                }
            }
            std::string device_id = fs["deviceID"];
            std::string gateway_id = fs["gatewayID"];
            if (m_client.setDeviceID(device_id))
            {
                if (!m_client.setGatewayID(gateway_id))
                {
                    LOGD("init error gatewayID:%s.", gateway_id.c_str());
                    return false;
                }
            }
            else
            {
                LOGD("init error deviceID:%s.", device_id.c_str());
                return false;
            }
        }
        m_init_flag = true;
        for (size_t i = 0; i < m_window_size; i++)
        {
            m_id_que.push_back("#");
        }
#ifdef RS_D455
        m_depth_flag = false;
        int cap_dep = fs["capture_depth"];
        if (cap_dep == 1)
        {
            m_depth_flag = true;
        }
#endif
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
            m_cap_uvc = new Capture(m_camera_file);
            if (!m_cap_uvc->initCamera())
            {
                LOGD("open camera failed");
                return false;
            }
            m_cap_uvc->printCameraformats();
        }
#endif
        m_open_camera_flag = true;
        LOGD("open camera finish");
#ifdef RS_D455
        // Start streaming with default recommended configuration
        rs2::pipeline_profile profile = m_pipe.start();
#endif
        return true;
    }
    else
    {
        return false;
    }
}

bool COW::checkFileExist(const std::string _path_file)
{
    std::ifstream file;
    file.open(_path_file);
    bool re = file.is_open();
    file.close();
    return re;
}

bool COW::removeLogfile(int _day)
{
    time_t now = time(0);
    now -= _day * 24 * 60 * 60; //天时分秒
    tm *ltm = localtime(&now);
    char buf[512];

    strftime(buf, 64, "/sdcard/Download/log_%Y-%m-%d.txt", ltm);

    if (checkFileExist(buf))
    {
        LOGD("remove %s", buf);
        remove(buf);
    }

    return true;
}
