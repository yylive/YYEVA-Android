//
// Created by zengjiale on 2022/8/15.
//

#ifndef YYEVA_IFRAMERENDERCONTROLLER_H
#define YYEVA_IFRAMERENDERCONTROLLER_H

#include <android/native_window_jni.h>
#include <bean/evaanimeconfig.h>
#include <android/bitmap.h>
#include <util/threadpool.h>
#include <GLES3/gl3.h>
#include <bean/optionparams.h>
#include <bean/evasrc.h>

class IFrameRenderController {
public:
    virtual GLuint initRender(ANativeWindow *window, bool isNeedYUV) = 0;
    virtual void destroyRender() = 0;
    virtual void updateViewPoint(int width, int height) = 0;
    virtual int getExternalTexture() = 0;
    virtual void videoSizeChange(int newWidth, int newHeight) = 0;
    virtual void setRenderConfig(EvaAnimeConfig* config) = 0;
    virtual void renderFrame() = 0;
    virtual void renderSwapBuffers() = 0;
    virtual void renderClearFrame() = 0;
    virtual void releaseTexture() = 0;

    virtual void onDecoding(int decodeIndex) = 0;
    virtual void onLoopStart() = 0;
    virtual void mixConfigCreate(EvaAnimeConfig* config) = 0;
    virtual void setMixSrc(vector<EvaSrc*> v) = 0;
    virtual void mixRenderCreate() = 0;
    virtual void mixRendering() = 0;
    virtual void mixRenderDestroy() = 0;

    virtual void setParams(OptionParams* params) = 0;
    virtual bool setSrcBitmap(string name, unsigned char* bitmap, AndroidBitmapInfo* bitmapInfo, string scaleMode) = 0;
    virtual void setSrcTxt(const char* srcId, const char* txt) = 0;
    virtual void touchPoint(int x, int y) = 0;

    virtual ThreadPool* getRenderThread() = 0;
    virtual void renderData() = 0;
    virtual void setReleaseFunc(std::function<void()> releaseCallback) = 0;

    ANativeWindow* surface;
};


#endif //YYEVA_IFRAMERENDERCONTROLLER_H
