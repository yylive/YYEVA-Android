//
// Created by zengjiale on 2022/8/15.
//

#ifndef YYEVA_NATIVEPLAYER_H
#define YYEVA_NATIVEPLAYER_H

#include <android/log.h>
#include <control/iframerendercontroller.h>
#include <control/iframedecodecontroller.h>
#include <control/iaudiodecodecontroller.h>
#include <control/iaudioplaycontroller.h>
#include <control/inativeplayer.h>
#include <control/evalistenermanager.h>
#include "AudioEngine.h"

#define LOG_TAG "NativePlayer"
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ELOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

using namespace std;

class NativePlayer : public INativePlayer {
public:
    NativePlayer();

    ~NativePlayer();

    void setSurface(ANativeWindow *window);

    void startPlay(string fileAddress);

    void startFrameDecode(IEvaFileContainer *fileContainer);

    void startAudioDecode(string path);

    void play();

    void stop();

    void pause();

    bool isPlaying();

    void setParams(OptionParams *params);

    void setSrcBitmap(string name, unsigned char *bitmap, AndroidBitmapInfo *bitmapInfo, string scaleMode);

    void renderFrame();

    void swapBuffers();

    void release();

    void destroy();

    void updateViewPoint(int width, int height);

    void touchRect();

    void setAnimListener(JavaVM *javaVm, jobject &listener);

    void setDecodeSurface(ANativeWindow *window);

    int getTextureId();

    void touchPoint(int x, int y);

private:
    ANativeWindow *window;
    ANativeWindow *decodeWindow;
    IAudioDecodeController *audioDecoder;
    IAudioPlayController *audioPlayer;
    IFrameDecodeController *frameDecoder;
    std::shared_ptr<AudioEngine> audioEngine_;
//    IFrameRenderController* renderManager;
    IEvaFileContainer *fileContainer;
    EvaListenerManager *listenerManager = nullptr;
    OptionParams* optionParams;
    string fileAddress;
};


#endif //YYEVA_NATIVEPLAYER_H
