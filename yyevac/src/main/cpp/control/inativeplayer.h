//
// Created by zengjiale on 2022/8/15.
//

#ifndef YYEVA_INATIVEPLAYER_H
#define YYEVA_INATIVEPLAYER_H
#include <bean/optionparams.h>

class INativePlayer {
public:
    virtual void setSurface(ANativeWindow* window) = 0;
    virtual void startPlay(string fileAddress) = 0;
    virtual void startFrameDecode(IEvaFileContainer* fileContainer) = 0;
    virtual void startAudioDecode(string path) = 0;
    virtual void play() = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual bool isPlaying() = 0;
    virtual void setParams(OptionParams* params) = 0;
    virtual void setSrcBitmap(string name, unsigned char* bitmap, AndroidBitmapInfo* bitmapInfo, string scaleMode) = 0;
    virtual void renderFrame() = 0;
    virtual void swapBuffers() = 0;
    virtual void release() = 0;
    virtual void updateViewPoint(int width, int height) = 0;
    virtual void touchRect() = 0;
    virtual void setAnimListener(JavaVM* javaVm,jobject& listener) = 0;
    virtual void setDecodeSurface(ANativeWindow* window) = 0;
    virtual int getTextureId() = 0;
    virtual void touchPoint(int x, int y) = 0;
};


#endif //YYEVA_INATIVEPLAYER_H
