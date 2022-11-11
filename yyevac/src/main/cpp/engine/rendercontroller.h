//
// Created by zengjiale on 2022/4/22.
//

#ifndef YYEVA_RENDERCONTROLLER_H
#define YYEVA_RENDERCONTROLLER_H


#include "irender.h"
#include <android/native_window_jni.h>
#include <bean/evaframeall.h>
#include <list>
#include <vector>
#include <engine/render.h>
#include <mix/mixrender.h>
#include <engine/yuvrender.h>
#include <engine/bgrender.h>
#include <engine/framebufferrender.h>

#include <android/log.h>

#define LOG_TAG "RenderController"
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ELOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

#define BG_TAG "bg_effect"

using namespace std;
class RenderController {
public:
    RenderController();
    ~RenderController();
    GLuint initRender(ANativeWindow *window, bool isNeedYUV);
    void destroyRender();
    void updateViewPoint(int width, int height);
    int getExternalTexture();
    void videoSizeChange(int newWidth, int newHeight);
    void setRenderConfig(EvaAnimeConfig* config);
    void renderSwapBuffers();
    void renderClearFrame();
    void releaseTexture();

    void mixConfigCreate(EvaAnimeConfig* config);
    void setMixSrc(vector<EvaSrc*> v);
    void mixRenderCreate();
    void mixRenderDestroy();
    void renderFrame(int frameIndex);

    void parseFrame(EvaAnimeConfig* config);
    void parseSrc(EvaAnimeConfig* config);
    void setSrcBitmap(const char* srcId, unsigned char* bitmap, AndroidBitmapInfo* bitmapInfo, string addr, string scaleMode);
    bool setSrcBitmap(string srcTag, unsigned char *bitmap, AndroidBitmapInfo* bitmapInfo, string scaleMode);
    void setSrcTxt(const char* srcId, const char* txt);
    void setBgImage(EvaSrc* src);
    void setBgImage(unsigned char* bitmap, AndroidBitmapInfo* bitmapInfo);
    void touchPoint(int x, int y);
    bool calClick(int x, int y, PointRect* frame);

    ANativeWindow* surface;
private:
    EGLCore *eglCore;
    list<IRender*> renderList;
    EvaAnimeConfig* config;
    int curFrameIndex;
    EvaFrameAll* frameAll;
    EvaSrcMap* srcMap;
    int surfaceWidth = 0;
    int surfaceHeight = 0;
    GLuint textureId = -1;
    GLuint bgTextureId = -1;
    shared_ptr<FramebufferRender> framebufferRender;
};


#endif //YYEVA_RENDERMANAGER_H
