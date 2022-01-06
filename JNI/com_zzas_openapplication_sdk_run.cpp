#include "com_zzas_openapplication_sdk_run.h"
#include "../SDK/cow_sdk.hpp"
#include <android/log.h>

#ifdef _Included_com_zzas_openapplication_sdk_run
extern "C"
{
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
  COW cow("/sdcard/Download/corner.yaml");
  JNIEXPORT jint JNICALL Java_com_zzas_openapplication_sdk_run(JNIEnv *env, jobject)
  {
    try
    {
      LOGI("%s", cow.getVersion());
      while (!cow.init())
      {
        LODI("init fail");
        cow.reset();
        usleep(3e7);
      }
      return 0;
    }
    catch (const std::exception &e)
    {
      cow.release();
      LOGI("Exception: %s", e.what());
      return -1;
    }
  }
}
#endif