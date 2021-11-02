#ifndef _android_jni_utility_H__
#define _android_jni_utility_H__
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <string>

#ifdef __ANDROID__
#include <android/log.h>
#define LOG_TAG "[COWID]"
// #define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG,
// __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
// #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
// __VA_ARGS__)
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL, LOG_TAG, __VA_ARGS__)
#else
#define LOGI(fmt, args...) \
  do                       \
  {                        \
    printf(fmt, ##args);   \
    printf("\n");          \
  } while (0)

#endif

class Reporter
{
public:
  Reporter(std::string caller, std::string file, int line, std::string level)
  {
    m_caller = caller;
    m_file_path = file;
    m_line = line;
    int index = m_file_path.find_last_of("/") + 1;
    if (index > 0)
    {
      m_file_name = m_file_path.substr(index, m_file_path.length() - index);
    }
    else
    {
      m_file_name = "NULL";
    }

    m_log_level = level;

    if (m_log_level.length() > 0)
    {
      m_log_level += " ";
    }
  }

  int operator()(const char *str, ...)
  {
    std::string cnt = getCurrentTime() + " LDAR->" + m_file_name + ":" +
                      std::to_string(m_line) + " " + m_caller + " # " +
                      m_log_level;
    va_list argptr;
    va_start(argptr, str);
    char log[1024];
    vsprintf(log, str, argptr);
    va_end(argptr);
    cnt += log;
    LOGI("%s", cnt.c_str());
    return 0;
  }

private:
  std::string getCurrentTime()
  {
    time_t now = time(0);
    // now += 28800;  // 8h*60min*60s
    tm *ltm = localtime(&now);
    char buf[512];
    strftime(buf, 64, "%Y-%m-%d %H:%M:%S:", ltm);

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

  std::string m_caller;
  std::string m_file_path;
  std::string m_file_name;
  std::string m_log_level;
  int m_line;
};
#define LOGD Reporter(__FUNCTION__, __FILE__, __LINE__, "")
#define LOGE Reporter(__FUNCTION__, __FILE__, __LINE__, "ERROR")
#endif
