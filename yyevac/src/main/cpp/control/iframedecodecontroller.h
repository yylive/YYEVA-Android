//
// Created by zengjiale on 2022/8/15.
//

#ifndef YYEVA_IFRAMEDECODECONTROLLER_H
#define YYEVA_IFRAMEDECODECONTROLLER_H
#include "util/threadpool.h"
#include <util/ievafilecontainer.h>
#include <bean/optionparams.h>
#include <android/bitmap.h>
#include <control/evalistenermanager.h>

class IFrameDecodeController {
public:
    virtual void setSurface(ANativeWindow *window) = 0;
    virtual void setDecodeSurface(ANativeWindow* window) = 0;
    virtual void setParams(OptionParams* params) = 0;
    virtual void startPlay(IEvaFileContainer* evaFileContainer) = 0;
    virtual void startFrameDecode(IEvaFileContainer* evaFileContainer) = 0;
    virtual void startDecode() = 0;
    virtual void setSrcBitmap(string name, unsigned char* bitmap, AndroidBitmapInfo* bitmapInfo, string scaleMode) = 0;
    virtual void updateViewPoint(int width, int height) = 0;
    virtual void renderFrame() = 0;
    virtual void release() = 0;
    virtual int getTextureId() = 0;
    virtual bool isPlaying() = 0;
    virtual void touchPoint(int x, int y) = 0;
    virtual void setAnimListener(EvaListenerManager* listener) = 0;

    virtual void stop() = 0;
    virtual void destroy() = 0;
    virtual void destroyInner() = 0;
    virtual bool createThread(ThreadPool* t, string name) = 0;
    virtual void quitSafely(ThreadPool* t) = 0;

};


#endif //YYEVA_IFRAMEDECODECONTROLLER_H
