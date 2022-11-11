//
// Created by zengjiale on 2022/9/22.
//

#ifndef YYEVA_OPTIONPARAMS_H
#define YYEVA_OPTIONPARAMS_H

#include <android/log.h>
#include <util/parson.h>
#include <string>

#define LOG_TAG "OptionParams"
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ELOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

using namespace std;
class OptionParams {
public:
    int fps = 30;
    int playCount = 1;
    bool isEnableH265 = true;
    bool isEnableH264 = true;
    bool isMute = false;
    bool usingSurfaceView = false;
    bool isRemoteService = false;
    bool isMainProcess = true;
    string mp4Address;
    int scaleType = 1;   // 1=>裁剪居中， 2=>全屏拉伸  3=》原资源大小
    bool useFbo = true;
    string filterType; //高清算法

    ~OptionParams();
    OptionParams* parse(const char* json);

    bool isNeedRender();
};


#endif //YYEVA_OPTIONPARAMS_H
