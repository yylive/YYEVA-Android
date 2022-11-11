//
// Created by zengjiale on 2022/8/25.
//

#ifndef YYEVA_RENDERMANGER_H
#define YYEVA_RENDERMANGER_H

#include <util/threadpool.h>
#include <android/log.h>
#include <engine/rendercontroller.h>
#include <control/iframerendercontroller.h>

#define LOG_TAG "RenderManager"
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ELOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

using namespace std;
class RenderManager: public IFrameRenderController {
public:
    ThreadPool* renderThread;
    RenderController* controller;
    OptionParams* optionParams;
    int decodeIndex = 0;
    int decodeEndIndex = 0;
    int frameIndex = 0;
    int frameDiffTimes = 0;
    const int DIFF_TIMES = 4;
//    std::function<void()> releaseCallback;

    RenderManager();
    ~RenderManager();
    GLuint initRender(ANativeWindow *window, bool isNeedYUV);
    void destroyRender();
    void updateViewPoint(int width, int height);
    int getExternalTexture();
    void videoSizeChange(int newWidth, int newHeight);
    void setRenderConfig(EvaAnimeConfig* config);
    void renderFrame();
    void renderSwapBuffers();
    void renderClearFrame();
    void releaseTexture();

    void onDecoding(int decodeIndex);
    void onDecodeEnd(int decodeIndex);
    void onLoopStart();
    void mixConfigCreate(EvaAnimeConfig* config);
    void setMixSrc(vector<EvaSrc*> v);
    void mixRenderCreate();
    void mixRendering();
    void mixRenderDestroy();

    void setParams(OptionParams* params);
    bool setSrcBitmap(string name, unsigned char* bitmap, AndroidBitmapInfo* bitmapInfo, string scaleMode);
    void setSrcTxt(const char* srcId, const char* txt);
    void touchPoint(int x, int y);
    ThreadPool* getRenderThread();
    void renderData();
    void setReleaseFunc(std::function<void()> releaseCallback);

    bool createThread(ThreadPool *t, string name);
    void quitSafely(ThreadPool *t);

};


#endif //YYEVA_RENDERMANGER_H
