//
// Created by zengjiale on 2022/9/22.
//

#ifndef YYEVA_OPTIONPARAMS_H
#define YYEVA_OPTIONPARAMS_H

#include <android/log.h>
#include <src/main/cpp/util/parson.h>
#include <string>

using namespace std;

class OptionParams {
public:
    long playerId = 0; //播放器id
    int fps = 30;
    int playCount = 1;
    bool isEnableH265 = true;
    bool isEnableH264 = true;
    bool isMute = false;
    bool disableAudio = true; //禁止音频
    bool usingSurfaceView = false;
    string mp4Address = "";
    int scaleType = 1;   // 1=>裁剪居中， 2=>全屏拉伸  3=》原资源大小

    ~OptionParams();

    OptionParams *parse(const char *json);
};


#endif //YYEVA_OPTIONPARAMS_H
