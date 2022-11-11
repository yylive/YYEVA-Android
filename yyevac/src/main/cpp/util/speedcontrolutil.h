//
// Created by zengjiale on 2022/6/21.
//

#ifndef YYEVA_SPEEDCONTROLUTIL_H
#define YYEVA_SPEEDCONTROLUTIL_H

#include <android/log.h>
#include <string>

#define LOG_TAG "SpeedControlUtil"
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ELOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

using namespace std;
class SpeedControlUtil {
private:
    long ONE_MILLION = 1000000;
    long prevPresentUsec = 0;
    long prevMonoUsec = 0;
    long fixedFrameDurationUsec = 0;
    bool loopReset = true;
    long getCurrentTime();
public:
    void setFixedPlaybackRate(int fps);
    void preRender(long presentationTimeUsec);
    void reset();
};


#endif //YYEVA_SPEEDCONTROLUTIL_H
