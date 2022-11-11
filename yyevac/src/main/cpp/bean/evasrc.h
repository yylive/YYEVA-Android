//
// Created by zengjiale on 2022/4/19.
//
#include <string>
#include <android/bitmap.h>
#include <GLES3/gl3.h>
#include <bean/effect.h>
#include <android/log.h>

#ifndef YYEVA_EVASRC_H
#define YYEVA_EVASRC_H

#define LOG_TAG "EvaSrc"
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ELOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

using namespace std;

class EvaSrc {
public:
    enum SrcType {
        UNKNOWN,
        IMG,
        TXT,
    };
    enum LoadType {
        _UNKNOWN,
        NET, // 网络加载的图片
        LOCAL, // 本地加载的图片
    };

    enum FitType {
        FIX_XY, // 按原始大小填充纹理 scaleFill
        CENTER_FULL,  // 以纹理中心点放置 aspectFill
        CENTER_FIT,  // aspectFit
    };
    enum Style {
        DEFAULT,
        BLOD, //文字粗体
    };

    string srcId = "";
    int w = 0;
    int h = 0;
    SrcType srcType = UNKNOWN;
    string srcTypeStr = "unknown";
    LoadType loadType = _UNKNOWN;
    unsigned char* bitmap = nullptr;
    string saveAddress = "";
    AndroidBitmapInfo* bitmapInfo = nullptr;
    int bitmapWidth = 0;
    int bitmapHeight = 0;
    int bitmapFormat = 0;
    string srcTag = "";
    string txt = "";
    Style style = DEFAULT;
    string fontColor = "";
    string textAlign = "center";
    FitType fitType = FIX_XY;
    GLuint srcTextureId = -1;
    int fontSize = 0;
    long size = 0;

    EvaSrc();

    EvaSrc(Effect* effect);

    ~EvaSrc();
};


#endif //YYEVA_EVASRC_H
