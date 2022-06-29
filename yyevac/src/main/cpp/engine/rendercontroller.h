//
// Created by zengjiale on 2022/4/22.
//

#ifndef YYEVA_RENDERCONTROLLER_H
#define YYEVA_RENDERCONTROLLER_H


#include "irender.h"
#include <android/native_window_jni.h>
#include <mix/evamixrender.h>
#include "bean/evaframeall.h"
#include "list"
#include "render.h"
#include "yuvrender.h"

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
    void setSrcBitmap(const char* srcId, unsigned char* bitmap, AndroidBitmapInfo* bitmapInfo, string addr);
    void setSrcTxt(const char* srcId, const char* txt);

private:
    IRender* render;
    EvaMixRender* mixRender;
    EvaAnimeConfig* config;
    int curFrameIndex;
    EvaFrameAll* frameAll;
    EvaSrcMap* srcMap;
};


#endif //YYEVA_RENDERCONTROLLER_H
