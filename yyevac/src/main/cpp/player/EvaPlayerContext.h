//
// Created by zengjiale on 2023/12/15.
//

#ifndef YYEVA_EVAPLAYERCONTEXT_H
#define YYEVA_EVAPLAYERCONTEXT_H

#include <src/main/cpp/bean/evaanimeconfig.h>
#include <src/main/cpp/bean/optionparams.h>

namespace yyeva {
    class EvaPlayerContext {
    public:
        EvaPlayerContext(): config(nullptr), params() {

        }

        ~EvaPlayerContext() {
            config = nullptr;
        }

        long id = 0; //播放器id
        int64_t duration = 0; //动画时长
        int64_t totalFrames = 0; //动画帧数
        EvaAnimeConfig *config; //mp4头部信息
        OptionParams params; //播放器配置信息
    };
}

#endif //YYEVA_EVAPLAYERCONTEXT_H
