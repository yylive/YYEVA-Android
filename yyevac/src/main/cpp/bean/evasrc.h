//
// Created by zengjiale on 2022/4/19.
//
#pragma once
#include <string>
#include <android/bitmap.h>
#include <GLES3/gl3.h>
#include "src/main/cpp/bean/effect.h"

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
        FIX_XY, // 按原始大小填充纹理  scaleFill
        CENTER_FULL,  // 以纹理中心点放置  aspectFill
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
    LoadType loadType = _UNKNOWN;
    unsigned char* bitmap = nullptr;
    string saveAddress = "";
    AndroidBitmapInfo* bitmapInfo = nullptr;
    int bitmapWidth;
    int bitmapHeight;
    int bitmapFormat;
    string srcTag = "";
    string txt = "";
    Style style = DEFAULT;
    string fontColor = "";
    FitType fitType = FIX_XY;
    GLuint srcTextureId = 0;
    int fontSize = 0;

    EvaSrc();

    EvaSrc(shared_ptr<Effect> effect);

    ~EvaSrc();
};
