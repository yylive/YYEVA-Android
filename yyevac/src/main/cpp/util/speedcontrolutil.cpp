//
// Created by zengjiale on 2022/6/21.
//

#include "speedcontrolutil.h"

#include <cstdio>
#include <ctime>
#include <unistd.h>
#include <exception>

//获取纳秒时间
long SpeedControlUtil::getCurrentTime() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
//    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

void SpeedControlUtil::setFixedPlaybackRate(int fps) {
    if (fps <= 0) return;
    fixedFrameDurationUsec = ONE_MILLION / fps;
}

////传入视频pts
//void SpeedControlUtil::preRender(long presentationTimeUsec) {
//    if (prevMonoUsec == 0) { //初始化化时间记录，立刻播放
//        prevMonoUsec = getCurrentTime();
//        prevPresentUsec = presentationTimeUsec;
//    } else {
//        long frameDelta = 0;
//        if (loopReset) {
//            //上一次播放的时间
//            prevPresentUsec = presentationTimeUsec - fixedFrameDurationUsec;
//            loopReset = false;
//        }
//        if (fixedFrameDurationUsec != 0) {
//            frameDelta = fixedFrameDurationUsec;
//        } else {
//            //获取默认的帧数差距时间
//            frameDelta = presentationTimeUsec - prevPresentUsec;
//        }
//
//        if (frameDelta < 0) {
//            frameDelta = 0;
//        } else if (frameDelta > 10 * ONE_MILLION) {
//            frameDelta = 5 * ONE_MILLION;
//        }
//
//        long desiredUsec = prevMonoUsec + frameDelta;  //应该播放的时间
//        long nowUsec = getCurrentTime();
////        ELOGV("sleep now %li, desired %li inter %li", nowUsec, desiredUsec, desiredUsec - nowUsec);
//        while (nowUsec < desiredUsec - 100) {
//            long sleepTimeUsec = desiredUsec - nowUsec;
//            if (sleepTimeUsec > 500000) {
//                sleepTimeUsec = 500000;
//            }
//            try {
//                usleep(sleepTimeUsec);
//            } catch (exception e) {
//                ELOGE("preRender failed: %s", e.what());
//            }
//            nowUsec = getCurrentTime();
//        }
//        prevMonoUsec += frameDelta;
//        prevPresentUsec += frameDelta;
//    }
//}

//传入视频pts,简化视频同步逻辑
void SpeedControlUtil::preRender(long presentationTimeUsec) {
    if (prevPresentUsec == 0) { //初始化化时间记录，立刻播放
        prevMonoUsec = getCurrentTime();
        prevPresentUsec = presentationTimeUsec;
    } else {
        if (presentationTimeUsec <= 0) {  //播放时间读取不出
            presentationTimeUsec = prevMonoUsec + fixedFrameDurationUsec;
        }
        long nowUsec = getCurrentTime() - prevMonoUsec;
//        ELOGV("sleep now %li, desired %li inter %li", nowUsec, presentationTimeUsec, presentationTimeUsec - nowUsec);
        if (nowUsec < presentationTimeUsec - 100) {
            long sleepTimeUsec = presentationTimeUsec - nowUsec;
            try {
                usleep(sleepTimeUsec);
            } catch (exception e) {
                ELOGE("preRender failed: %s", e.what());
            }
        }
        prevPresentUsec = presentationTimeUsec;
    }
}

void SpeedControlUtil::reset() {
    prevPresentUsec = 0;
    prevMonoUsec = 0;
}
