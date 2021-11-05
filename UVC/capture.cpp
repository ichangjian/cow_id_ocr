#include "capture.hpp"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <asm/types.h> /* for videodev2.h */
#include <memory>
#include <linux/videodev2.h>
#include "../log/log.h"
// static void log_stderr(camera_log_t type, const char *msg, void *pointer)
// {
//   (void)pointer;
//   switch (type)
//   {
//   case CAMERA_ERROR:
//     fprintf(stderr, "ERROR [%s] %d: %s\n", msg, errno, strerror(errno));
//     return;
//   case CAMERA_FAIL:
//     fprintf(stderr, "FAIL [%s]\n", msg);
//     return;
//   case CAMERA_INFO:
//     fprintf(stderr, "INFO [%s]\n", msg);
//     return;
//   }
// }

static bool error(camera_t *camera, const char *msg)
{
  // camera->context.log(CAMERA_ERROR, msg, camera->context.pointer);
  return false;
}
static bool failure(camera_t *camera, const char *msg)
{
  // camera->context.log(CAMERA_FAIL, msg, camera->context.pointer);
  return false;
}

static int xioctl(int fd, unsigned long int request, void *arg)
{
  for (int i = 0; i < 100; i++)
  {
    int r = ioctl(fd, request, arg);
    if (r != -1 || errno != EINTR)
      return r;
  }
  return -1;
}

static void free_buffers(camera_t *camera, size_t count)
{
  for (size_t i = 0; i < count; i++)
  {
    munmap(camera->buffers[i].start, camera->buffers[i].length);
  }
  free(camera->buffers);
  camera->buffers = NULL;
  camera->buffer_count = 0;
}

static bool camera_buffer_prepare(camera_t *camera)
{
  struct v4l2_requestbuffers req;
  memset(&req, 0, sizeof req);
  req.count = 4;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if (xioctl(camera->fd, VIDIOC_REQBUFS, &req) == -1)
    return error(camera, "VIDIOC_REQBUFS");
  camera->buffer_count = req.count;
  camera->buffers = (camera_buffer_t *)calloc(req.count, sizeof(camera_buffer_t));

  size_t buf_max = 0;
  for (size_t i = 0; i < camera->buffer_count; i++)
  {
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;
    if (xioctl(camera->fd, VIDIOC_QUERYBUF, &buf) == -1)
    {
      free_buffers(camera, i);
      return error(camera, "VIDIOC_QUERYBUF");
    }
    if (buf.length > buf_max)
      buf_max = buf.length;
    camera->buffers[i].length = buf.length;
    camera->buffers[i].start = (uint8_t *)mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED,
                                               camera->fd, buf.m.offset);
    if (camera->buffers[i].start == MAP_FAILED)
    {
      free_buffers(camera, i);
      return error(camera, "mmap");
    }
  }
  camera->head.start = (uint8_t *)calloc(buf_max, sizeof(uint8_t));
  return true;
}

static void camera_buffer_finish(camera_t *camera)
{
  free_buffers(camera, camera->buffer_count);
  free(camera->head.start);
  camera->head.length = 0;
  camera->head.start = NULL;
}

static bool camera_load_settings(camera_t *camera)
{
  struct v4l2_format format;
  memset(&format, 0, sizeof format);
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (xioctl(camera->fd, VIDIOC_G_FMT, &format) == -1)
    return error(camera, "VIDIOC_G_FMT");
  camera->width = format.fmt.pix.width;
  camera->height = format.fmt.pix.height;
  return true;
}

static bool camera_init(camera_t *camera)
{
  struct v4l2_capability cap;
  if (xioctl(camera->fd, VIDIOC_QUERYCAP, &cap) == -1)
    return error(camera, "VIDIOC_QUERYCAP");
  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    return failure(camera, "no capture");
  if (!(cap.capabilities & V4L2_CAP_STREAMING))
    return failure(camera, "no streaming");

  struct v4l2_cropcap cropcap;
  memset(&cropcap, 0, sizeof cropcap);
  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (xioctl(camera->fd, VIDIOC_CROPCAP, &cropcap) == 0)
  {
    struct v4l2_crop crop;
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect;
    if (xioctl(camera->fd, VIDIOC_S_CROP, &crop) == -1)
    {
      // cropping not supported
    }
  }
  camera->initialized = true;
  return true;
}
static bool camera_load(camera_t *camera)
{
  if (!camera->initialized)
  {
    if (!camera_init(camera))
      return false;
  }
  if (camera->buffer_count == 0)
  {
    if (!camera_load_settings(camera))
      return false;
    if (!camera_buffer_prepare(camera))
      return false;
  }
  return true;
}

camera_t *Capture::camera_open(const char *device)
{
  int fd = open(device, O_RDWR | O_NONBLOCK, 0);
  if (fd == -1)
    return NULL;

  camera_t *camera = (camera_t *)malloc(sizeof(camera_t));
  camera->fd = fd;
  camera->initialized = false;
  camera->width = 0;
  camera->height = 0;
  camera->buffer_count = 0;
  camera->buffers = NULL;
  camera->head.length = 0;
  camera->head.start = NULL;
  // camera->context.pointer = NULL;
  // camera->context.log = &log_stderr;
  return camera;
}

bool Capture::camera_start(camera_t *camera)
{
  if (!camera_load(camera))
    return false;

  for (size_t i = 0; i < camera->buffer_count; i++)
  {
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;
    if (xioctl(camera->fd, VIDIOC_QBUF, &buf) == -1)
      return error(camera, "VIDIOC_QBUF");
  }

  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (xioctl(camera->fd, VIDIOC_STREAMON, &type) == -1)
    return error(camera, "VIDIOC_STREAMON");
  return true;
}

bool Capture::camera_stop(camera_t *camera)
{
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (xioctl(camera->fd, VIDIOC_STREAMOFF, &type) == -1)
    return error(camera, "VIDIOC_STREAMOFF");
  camera_buffer_finish(camera);

  struct v4l2_requestbuffers req;
  memset(&req, 0, sizeof req);
  req.count = 0;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if (xioctl(camera->fd, VIDIOC_REQBUFS, &req) == -1)
    return error(camera, "VIDIOC_REQBUFS 0");
  return true;
}

bool Capture::camera_close(camera_t *camera)
{
  if (camera->buffer_count > 0)
  {
    camera_stop(camera);
  }
  for (int i = 0; i < 10; i++)
  {
    if (close(camera->fd) != -1)
      break;
  }
  free(camera);
  return true;
}

//[[capturing]
bool Capture::camera_capture(camera_t *camera)
{
  struct v4l2_buffer buf;
  memset(&buf, 0, sizeof buf);
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  if (xioctl(camera->fd, VIDIOC_DQBUF, &buf) == -1)
    return false;
  memcpy(camera->head.start, camera->buffers[buf.index].start, buf.bytesused);
  camera->head.length = buf.bytesused;
  if (xioctl(camera->fd, VIDIOC_QBUF, &buf) == -1)
    return false;
  return true;
}

static inline int minmax(int min, int v, int max)
{
  return (v < min) ? min : (max < v) ? max
                                     : v;
}
static inline uint8_t yuv2r(int y, int u, int v)
{
  (void)u;
  return minmax(0, (y + 359 * v) >> 8, 255);
}
static inline uint8_t yuv2g(int y, int u, int v)
{
  return minmax(0, (y + 88 * v - 183 * u) >> 8, 255);
}
static inline uint8_t yuv2b(int y, int u, int v)
{
  (void)v;
  return minmax(0, (y + 454 * u) >> 8, 255);
}
uint8_t *Capture::yuyv2rgb(const uint8_t *yuyv, uint32_t width, uint32_t height)
{
  uint8_t *rgb = (uint8_t *)calloc(width * height * 3, sizeof(uint8_t));
  for (size_t i = 0; i < height; i++)
  {
    for (size_t j = 0; j < width; j += 2)
    {
      size_t index = i * width + j;
      size_t index2 = index * 2, index3 = index * 3;
      int y0 = yuyv[index2 + 0] << 8;
      int u = yuyv[index2 + 1] - 128;
      int y1 = yuyv[index2 + 2] << 8;
      int v = yuyv[index2 + 3] - 128;
      rgb[index3 + 0] = yuv2r(y0, u, v);
      rgb[index3 + 1] = yuv2g(y0, u, v);
      rgb[index3 + 2] = yuv2b(y0, u, v);
      rgb[index3 + 3] = yuv2r(y1, u, v);
      rgb[index3 + 4] = yuv2g(y1, u, v);
      rgb[index3 + 5] = yuv2b(y1, u, v);
    }
  }
  return rgb;
}

//[formats and config]
uint32_t Capture::camera_format_id(const char *name)
{
  //assert(strlen(name) == 4);
  return (uint32_t)name[0] | ((uint32_t)name[1] << 8) |
         ((uint32_t)name[2] << 16) | ((uint32_t)name[3] << 24);
}
void Capture::camera_format_name(uint32_t format_id, char *name)
{
  name[0] = format_id & 0xff;
  name[1] = (format_id >> 8) & 0xff;
  name[2] = (format_id >> 16) & 0xff;
  name[3] = (format_id >> 24) & 0xff;
  name[4] = '\0';
}

static bool camera_format_set(camera_t *camera, const camera_format_t *format)
{
  if (format->width > 0 && format->height > 0)
  {
    uint32_t pixformat = format->format ? format->format : V4L2_PIX_FMT_YUYV;
    struct v4l2_format vformat;
    memset(&vformat, 0, sizeof vformat);
    vformat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    vformat.fmt.pix.width = format->width;
    vformat.fmt.pix.height = format->height;
    vformat.fmt.pix.pixelformat = pixformat;
    vformat.fmt.pix.field = V4L2_FIELD_NONE;
    if (xioctl(camera->fd, VIDIOC_S_FMT, &vformat) == -1)
      return error(camera, "VIDIOC_S_FMT");
  }
  if (format->interval.numerator != 0 && format->interval.denominator != 0)
  {
    struct v4l2_streamparm parm;
    memset(&parm, 0, sizeof parm);
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parm.parm.capture.timeperframe.numerator = format->interval.numerator;
    parm.parm.capture.timeperframe.denominator = format->interval.denominator;
    if (xioctl(camera->fd, VIDIOC_S_PARM, &parm) == -1)
      return error(camera, "VIDIOC_S_PARM");
  }
  return true;
}

static bool camera_format_get(camera_t *camera, camera_format_t *format)
{
  struct v4l2_format vformat;
  memset(&vformat, 0, sizeof vformat);
  vformat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (xioctl(camera->fd, VIDIOC_G_FMT, &vformat) == -1)
    return error(camera, "VIDIOC_G_FMT");

  format->format = vformat.fmt.pix.pixelformat;
  format->width = vformat.fmt.pix.width;
  format->height = vformat.fmt.pix.height;

  struct v4l2_streamparm parm;
  memset(&parm, 0, sizeof parm);
  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (xioctl(camera->fd, VIDIOC_G_PARM, &parm) == -1)
    return error(camera, "VIDIOC_G_PARM");
  format->interval.numerator = parm.parm.capture.timeperframe.numerator;
  format->interval.denominator = parm.parm.capture.timeperframe.denominator;
  return true;
}

bool Capture::camera_config_get(camera_t *camera, camera_format_t *format)
{
  return camera_format_get(camera, format);
}
bool Capture::camera_config_set(camera_t *camera, const camera_format_t *format)
{
  if (camera->buffer_count > 0)
  {
    if (!camera_stop(camera))
      return false;
  }
  if (!camera->initialized)
  {
    if (!camera_init(camera))
      return false;
  }
  if (!camera_format_set(camera, format))
    return false;
  if (!camera_load_settings(camera))
    return false;
  return camera_buffer_prepare(camera);
}

camera_formats_t *Capture::camera_formats_new(const camera_t *camera)
{
  camera_formats_t *ret = (camera_formats_t *)malloc(sizeof(camera_formats_t));
  ret->length = 0;
  ret->head = NULL;
  for (uint32_t i = 0;; i++)
  {
    struct v4l2_fmtdesc fmt;
    memset(&fmt, 0, sizeof fmt);
    fmt.index = i;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(camera->fd, VIDIOC_ENUM_FMT, &fmt) == -1)
      break;
    //printf("[%s]\n", fmt.description);
    for (uint32_t j = 0;; j++)
    {
      struct v4l2_frmsizeenum frmsize;
      memset(&frmsize, 0, sizeof frmsize);
      frmsize.index = j;
      frmsize.pixel_format = fmt.pixelformat;
      if (ioctl(camera->fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == -1)
        break;
      if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
      {
        //printf("- w: %d, h: %d\n",
        //       frmsize.discrete.width, frmsize.discrete.height);
        for (uint32_t k = 0;; k++)
        {
          struct v4l2_frmivalenum frmival;
          memset(&frmival, 0, sizeof frmival);
          frmival.index = k;
          frmival.pixel_format = fmt.pixelformat;
          frmival.width = frmsize.discrete.width;
          frmival.height = frmsize.discrete.height;
          if (ioctl(camera->fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) == -1)
            break;
          if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE)
          {
            //printf("  - fps: %d/%d\n",
            //       frmival.discrete.denominator,frmival.discrete.numerator);
            ret->head = (camera_format_t *)realloc(ret->head, (ret->length + 1) * sizeof(camera_format_t));
            ret->head[ret->length].format = fmt.pixelformat;
            ret->head[ret->length].width = frmsize.discrete.width;
            ret->head[ret->length].height = frmsize.discrete.height;
            ret->head[ret->length].interval.numerator =
                frmival.discrete.numerator;
            ret->head[ret->length].interval.denominator =
                frmival.discrete.denominator;
            ret->length++;
          }
          else
          {
            //printf("  - fps: %d/%d-%d/%d\n",
            //       frmival.stepwise.min.denominator,
            //       frmival.stepwise.min.numerator,
            //       frmival.stepwise.max.denominator,
            //       frmival.stepwise.max.numerator);
            // TBD: when stepwize
          }
        }
      }
      else
      {
        //printf("- w: %d-%d, h: %d-%d\n",
        //       frmsize.stepwise.min_width, frmsize.stepwise.max_width,
        //      frmsize.stepwise.min_height, frmsize.stepwise.max_height);
        // TBD: when stepwize
      }
    }
  }
  return ret;
}
void Capture::camera_formats_delete(camera_formats_t *formats)
{
  free(formats->head);
  free(formats);
}

bool Capture::initCamera()
{
  LOGD("init camera %s", m_dev_vedio.c_str());
  m_camera = camera_open(m_dev_vedio.c_str());
  if (!m_camera)
  {
    fprintf(stderr, "[%s] %s\n", m_dev_vedio.c_str(), strerror(errno));
    return false;
  }
  if (!camera_start(m_camera))
    return false;
  LOGD("init camera finish");
  camera_format_t config;
  camera_config_get(m_camera, &config);
  LOGD("get camera width %d", config.width);
  LOGD("get camera height %d", config.height);
  return true;
}
bool Capture::setCamera(UVC_FORMAT _format, unsigned int _width, unsigned int _height, unsigned int _fps)
{
  camera_format_t config;
  switch (_format)
  {
  case MJPG_FORMAT:
    config = camera_format_t{1196444237, _width, _height, {1, _fps}};
    LOGD("set camera MJPG");
    LOGD("set camera width %d", config.width);
    LOGD("set camera height %d", config.height);
    if (!camera_config_set(m_camera, &config))
      return false;
    if (!camera_config_get(m_camera, &config))
      return false;
    break;

  case YUYV_FORMAT:
    config = camera_format_t{1448695129, _width, _height, {1, _fps}};
    if (!camera_config_set(m_camera, &config))
      return false;
    if (!camera_config_get(m_camera, &config))
      return false;
    break;

  default:
    break;
  }
  LOGD("get camera width %d", config.width);
  LOGD("get camera height %d", config.height);
  LOGD("set camera finish");
  return true;
}

bool Capture::camera_frame(camera_t *camera, struct timeval timeout)
{
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(camera->fd, &fds);
  int r = select(camera->fd + 1, &fds, 0, 0, &timeout);
  if (r == -1)
    exit(EXIT_FAILURE);
  if (r == 0)
    return false;
  return camera_capture(camera);
}
int Capture::captureMat(cv::Mat &_image)
{
  if (m_camera == NULL)
  {
    return -1;
  }

  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;

  camera_frame(m_camera, timeout);

  std::vector<uchar> data;
  for (size_t i = 0; i < m_camera->head.length; i++)
  {
    data.push_back(m_camera->head.start[i]);
  }
  if (data.size() < 100 && m_camera->head.length == data.size())
  {
    LOGD("%d %d", m_camera->head.length, data.size());
    return -1;
  }

  _image = cv::imdecode(data, cv::IMREAD_COLOR);

  return 0;
}

int Capture::printCameraformats()
{
  if (m_camera == NULL)
  {
    return -1;
  }

  char name[5];
  camera_format_t format;
  camera_config_get(m_camera, &format);
  camera_format_name(format.format, name);
  puts("[current config]");
  printf("- [%s] w: %d, h: %d, fps: %d/%d\n",
         name, format.width, format.height,
         format.interval.denominator,
         format.interval.numerator);

  puts("[available formats]");
  camera_formats_t *formats = camera_formats_new(m_camera);
  for (size_t i = 0; i < formats->length; i++)
  {
    printf("%d\n", formats->head[i].format);
    camera_format_name(formats->head[i].format, name);
    printf("- [%s] w: %d, h: %d, fps: %d/%d\n",
           name, formats->head[i].width, formats->head[i].height,
           formats->head[i].interval.denominator,
           formats->head[i].interval.numerator);
  }
  camera_formats_delete(formats);

  return EXIT_SUCCESS;
}
