//
// Created by zengjiale on 2022/4/15.
//


#ifndef YYEVA_EVAANIMECONFIG_H
#define YYEVA_EVAANIMECONFIG_H

#include <android/log.h>
#include <bean/datas.h>
#include <bean/descript.h>
#include <bean/effect.h>
#include <list>
#include <util/parson.h>
#include <util/evaconstant.h>
#include <bean/optionparams.h>
#include <bean/evaframeall.h>
#include <bean/evasrcmap.h>
#include <control/evalistenermanager.h>

#define LOG_TAG "EvaAnimeConfig"
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ELOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

using namespace std;
class EvaAnimeConfig {
public:
    EvaAnimeConfig();
    ~EvaAnimeConfig();
    int width = 0;
    int height = 0;
    int videoWidth = 0;
    int videoHeight = 0;
    int surfaceWidth = 0;
    int surfaceHeight = 0;
    Descript *descript;
    list<Effect> effects;
    list<Datas> datas;

    PointRect *alphaPointRect;
    PointRect *rgbPointRect;
    bool isMix = false;
    bool isDefaultConfig = false;
    int defaultVideoMode = EvaConstant::getInstance()->VIDEO_MODE_SPLIT_HORIZONTAL;
    int fps = 30;
    OptionParams* params;
    EvaListenerManager* listener;
    GLuint videoTextureId = -1;
    GLuint bgTextureId = -1;

    EvaFrameAll* frameAll;
    EvaSrcMap* srcMap;

    static EvaAnimeConfig* parse(const char* json);
};





#endif //YYEVA_EVAANIMECONFIG_H
