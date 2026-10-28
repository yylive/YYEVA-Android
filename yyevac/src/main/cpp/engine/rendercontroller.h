//
// Created by zengjiale on 2022/4/22.
//

#ifndef YYEVA_RENDERCONTROLLER_H
#define YYEVA_RENDERCONTROLLER_H


#include "irender.h"
#include <android/native_window_jni.h>
#include <egl/eglcore.h>
#include <mix/evamixrender.h>
#include <bean/evaframeall.h>
#include <list>
#include <engine/render.h>
#include <engine/yuvrender.h>
#include <engine/bgrender.h>
#include <engine/mp4render.h>

using namespace std;
class RenderController {
public:
    RenderController();
    ~RenderController();
    GLuint initRender(ANativeWindow *window, bool isNeedYUV, bool isNormalMp4);
    void destroyRender();
    void updateViewPoint(int width, int height);
    int getExternalTexture();
    void videoSizeChange(int newWidth, int newHeight);
    void setRenderConfig(EvaAnimeConfig* config);
    void renderFrame();
    void renderSwapBuffers();
    void renderClearFrame();
    void releaseTexture();

    void mixConfigCreate(EvaAnimeConfig* config);
    void mixRenderCreate();
    void mixRendering(int frameIndex);
    void mixRenderRelease(int textureId);
    void mixRenderDestroy();

    void parseFrame(EvaAnimeConfig* config);
    void parseSrc(EvaAnimeConfig* config);
    void setSrcBitmap(const char* srcId, unsigned char* bitmap, AndroidBitmapInfo* bitmapInfo, string scaleMode);
    void setSrcTxt(const char* srcId, const char* txt);
    void setBgImage(unsigned char* bitmap, AndroidBitmapInfo* bitmapInfo);

private:
    BgRender* bgRender;
    IRender* render;
    EvaMixRender* mixRender;
    EvaAnimeConfig* config;
    int curFrameIndex;
    EvaFrameAll* frameAll;
    EvaSrcMap* srcMap;
    EGLCore *eglCore;
};


#endif //YYEVA_RENDERCONTROLLER_H
