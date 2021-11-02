#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>

#ifndef __V4L2_COMMON__
#define CAMERA_OLD_VIDEODEV2_H
#endif

typedef struct
{
  uint8_t *start;
  size_t length;
} camera_buffer_t;

typedef struct
{
  int fd;
  bool initialized;
  uint32_t width;
  uint32_t height;
  size_t buffer_count;
  camera_buffer_t *buffers;
  camera_buffer_t head;
} camera_t;

typedef struct
{
  uint32_t format;
  uint32_t width;
  uint32_t height;
  struct
  {
    uint32_t numerator;
    uint32_t denominator;
  } interval;
} camera_format_t;

typedef struct
{
  size_t length;
  camera_format_t *head;
} camera_formats_t;

enum UVC_FORMAT
{
  MJPG_FORMAT,
  YUYV_FORMAT
};
class Capture
{
private:
  camera_t *camera_open(const char *device);
  bool camera_start(camera_t *camera);
  bool camera_stop(camera_t *camera);
  bool camera_close(camera_t *camera);

  bool camera_capture(camera_t *camera);
  uint8_t *yuyv2rgb(const uint8_t *yuyv, uint32_t width, uint32_t height);
  /* convert 4 char name and id. e.g. "YUYV" */
  uint32_t camera_format_id(const char *name);
  void camera_format_name(uint32_t format_id, char *name);

  camera_formats_t *camera_formats_new(const camera_t *camera);
  void camera_formats_delete(camera_formats_t *formats);
  bool camera_config_get(camera_t *camera, camera_format_t *format);
  bool camera_config_set(camera_t *camera, const camera_format_t *format);
  bool camera_frame(camera_t *camera, struct timeval timeout);

  camera_t *m_camera;
  camera_formats_t *m_format;
  std::string m_dev_vedio;

public:
  Capture(const std::string &_dev_vedio)
  {
    m_camera = NULL;
    m_format = NULL;
    m_dev_vedio = _dev_vedio;
  };
  ~Capture()
  {
    if (m_camera != NULL)
    {
      camera_stop(m_camera);
      camera_close(m_camera);
    }
    if (m_format != NULL)
    {
      camera_formats_delete(m_format);
    }
  };

  bool initCamera();
  bool setCamera(UVC_FORMAT _format, unsigned int _width, unsigned int _height, unsigned int _fps);
  int printCameraformats();

  int captureMat(cv::Mat &_image);
};
