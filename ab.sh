#!/bin/bash
rm -r android_build
mkdir android_build
cd android_build
NDK_HOME="/home/hi/android-ndk-r22b"
echo "----------------------------cmake release----------------------------"

/home/hi/cmake-3.21.3-linux-x86_64/bin/cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$NDK_HOME/build/cmake/android.toolchain.cmake \
    -DANDROID_NDK=$NDK_HOME \
    -DANDROID_ABI=armeabi-v7a \
    -DANDROID_NATIVE_API_LEVEL=21 \
    -DANDROID_STL=c++_static 
    #-DBUILD_ANDROID_EXAMPLES=OFF 
