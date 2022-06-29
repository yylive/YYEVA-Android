//
// Created by asus on 2022/4/15.
//

#include <GLES2/gl2.h>
#include "src/main/cpp/bean/evaanimeconfig.h"

#ifndef YYEVA_IRENDER_H
#define YYEVA_IRENDER_H

class IRender {
public:
    virtual void initRender() = 0;
    virtual void renderFrame() = 0;
    virtual void clearFrame() = 0;
    virtual void destroyRender() = 0;
    virtual void setAnimeConfig(EvaAnimeConfig* config) = 0;
    virtual void updateViewPort(int width, int height){};
    virtual GLuint getExternalTexture() = 0;
    virtual void releaseTexture() = 0;
    virtual void swapBuffers() = 0;
    void setYUVData(int width, int height, char *y, char *u, char *v){};
};

#endif
