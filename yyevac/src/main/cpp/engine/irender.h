//
// Created by asus on 2022/4/15.
//

#include <GLES3/gl3.h>
#include "src/main/cpp/bean/evaanimeconfig.h"

#ifndef YYEVA_IRENDER_H
#define YYEVA_IRENDER_H

class IRender {
public:
    virtual void initRender() = 0;
    virtual void renderFrame(int frameIndex) = 0;
    virtual void clearFrame() = 0;
    virtual void destroyRender() = 0;
    virtual void setAnimeConfig(EvaAnimeConfig* config) = 0;
    virtual void updateViewPort(int width, int height){};
    virtual GLuint getExternalTexture() = 0;
    virtual void releaseTexture() = 0;
    virtual void swapBuffers() = 0;
    void setYUVData(int width, int height, char *y, char *u, char *v){};

    //检测错误
    void checkGLError(std::string op) {
        GLint error = glGetError();
        if(error != GL_NO_ERROR){
            char err = (char)error;
            ELOGE("%s :glError 0x%d",op.c_str(),err);
        }
    };
};

#endif
