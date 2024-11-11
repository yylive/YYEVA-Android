//
// Created by asus on 2022/4/15.
//

#pragma once
#include <GLES3/gl3.h>
#include "src/main/cpp/bean/evaanimeconfig.h"

class IRender {
public:
    virtual void initRender() = 0;
    virtual void renderFrame() = 0;
    virtual void clearFrame() = 0;
    virtual void destroyRender() = 0;
    virtual void setAnimeConfig(shared_ptr<EvaAnimeConfig> config) = 0;
    virtual void updateViewPort(int width, int height){};
    virtual GLuint getExternalTexture() = 0;
    virtual void releaseTexture() = 0;
    virtual void swapBuffers() = 0;
    void setYUVData(int width, int height, char *y, char *u, char *v){};
    //如果有背景需要开启混合
    virtual void setHasBg(bool hasBg)=0;
};
