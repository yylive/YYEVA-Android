# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/jniLibs/${ANDROID_ABI})

# 定义 so 库和头文件所在目录，方便后面使用
set(ffmpeg_lib_dir ${CMAKE_CURRENT_SOURCE_DIR}/../jni/${ANDROID_ABI})
set(ffmpeg_head_dir ${CMAKE_CURRENT_SOURCE_DIR})

# 2. 添加头文件目录
include_directories(${ffmpeg_head_dir}/include)
include_directories(${ffmpeg_head_dir}/yyevav2)

link_directories(
        ${PROJECT_SOURCE_DIR}/jni/${ANDROID_ABI})
include_directories(${CMAKE_SOURCE_DIR} src/main/cpp)
add_library( # Sets the name of the library.
        yyeva

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        src/main/cpp/yyevajni.cpp
        src/main/cpp/bean/effect.cpp
        src/main/cpp/bean/evaanimeconfig.cpp
        src/main/cpp/bean/evasrc.cpp
        src/main/cpp/bean/evasrcmap.cpp
        src/main/cpp/bean/evaframe.cpp
        src/main/cpp/bean/evaframeall.cpp
        src/main/cpp/bean/evaframeset.cpp
        src/main/cpp/egl/eglcore.cpp
        src/main/cpp/engine/yuvrender.cpp
        src/main/cpp/engine/render.cpp
        src/main/cpp/engine/mp4render.cpp
        src/main/cpp/engine/bgrender.cpp
        src/main/cpp/engine/rendercontroller.cpp
        src/main/cpp/engine/evaengine.cpp
        src/main/cpp/util/shaderutil.cpp
        src/main/cpp/util/glfloatarray.cpp
        src/main/cpp/util/vertexutil.cpp
        src/main/cpp/util/texcoordsutil.cpp
        src/main/cpp/util/textureloadutil.cpp
        src/main/cpp/mix/evamixrender.cpp
        src/main/cpp/mix/mixshader.cpp
        src/main/cpp/vulkan_wrapper/vulkan_wrapper.cpp
        )

# 添加FFmpeg相关的so库
add_library(avutil
        SHARED
        IMPORTED)
set_target_properties(avutil
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_lib_dir}/libavutil.so)

add_library(avcodec
        SHARED
        IMPORTED)
set_target_properties(avcodec
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_lib_dir}/libavcodec.so)

add_library(avformat
        SHARED
        IMPORTED)
set_target_properties(avformat
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_lib_dir}/libavformat.so)

add_library(swscale
        SHARED
        IMPORTED)
set_target_properties(swscale
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_lib_dir}/libswscale.so)

add_library(swresample
        SHARED
        IMPORTED)
set_target_properties(swresample
        PROPERTIES IMPORTED_LOCATION
        ${ffmpeg_lib_dir}/libswresample.so)


add_library(parson
        STATIC
        src/main/cpp/util/parson.c)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.


target_link_libraries( # Specifies the target library.
        yyeva
        android
        #                       GLESv3
        #连接 FFmpeg 相关的库
        avcodec
        avutil
        avformat
        swscale
        swresample
        oboe
        GLESv2
        EGL
        jnigraphics
        parson

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib} )