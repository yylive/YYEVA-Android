# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10.2)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/jniLibs/${ANDROID_ABI})
# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

# 去除未使用函数与变量
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections -Os -Wall -flto")
#set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")

#set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -O3 -flto")

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os -Wall -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os -Wall -s")

set(OBOE_DIR src/main/cpp/oboe)
add_subdirectory(${OBOE_DIR} ./build)
include_directories(${OBOE_DIR}/include)

include_directories(${CMAKE_SOURCE_DIR} src/main/cpp)
add_library( # Sets the name of the library.
        yyeva2

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        src/main/cpp/yyeva2jni.cpp
        src/main/cpp/bean/effect.cpp
        src/main/cpp/bean/evaanimeconfig.cpp
        src/main/cpp/bean/evasrc.cpp
        src/main/cpp/bean/evasrcmap.cpp
        src/main/cpp/bean/evaframe.cpp
        src/main/cpp/bean/evaframeall.cpp
        src/main/cpp/bean/evaframeset.cpp
        src/main/cpp/bean/optionparams.cpp
        src/main/cpp/egl/eglcore.cpp
        src/main/cpp/engine/yuvrender.cpp
        src/main/cpp/engine/render.cpp
        src/main/cpp/engine/bgrender.cpp
        src/main/cpp/engine/rendercontroller.cpp
        src/main/cpp/engine/framebufferrender.cpp
        src/main/cpp/util/shaderutil.cpp
        src/main/cpp/util/glfloatarray.cpp
        src/main/cpp/util/vertexutil.cpp
        src/main/cpp/util/texcoordsutil.cpp
        src/main/cpp/util/textureloadutil.cpp
        src/main/cpp/util/speedcontrolutil.cpp
        src/main/cpp/util/evamediautil.cpp
        src/main/cpp/util/evaconstant.cpp
        src/main/cpp/util/evafilecontainer.cpp
        src/main/cpp/mix/mixrender.cpp
        src/main/cpp/mix/mixshader.cpp
        src/main/cpp/vulkan_wrapper/vulkan_wrapper.cpp
        src/main/cpp/control/nativeplayer.cpp
        src/main/cpp/control/videodecoder.cpp
        src/main/cpp/control/rendermanger.cpp
        src/main/cpp/control/evaconfig.cpp
        src/main/cpp/control/evalistenermanager.cpp
        src/main/cpp/control/AudioEngine.cpp
        src/main/cpp/control/AudioPlayer.cpp
        src/main/cpp/control/AudioCodec.cpp
        )

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

# 设置去除未使用代码的链接flag
#SET_TARGET_PROPERTIES(yyeva2 PROPERTIES LINK_FLAGS "-Wl,-s,--gc-sections")

target_link_libraries( # Specifies the target library.
                       yyeva2
                       android
                       GLESv3
                       EGL
                       jnigraphics
                       mediandk
                       oboe
                       parson
                       z

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )

