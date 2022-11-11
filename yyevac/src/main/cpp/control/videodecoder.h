//
// Created by zengjiale on 2022/8/16.
//

#ifndef YYEVA_VIDEODECODER_H
#define YYEVA_VIDEODECODER_H

#include <util/evamediautil.h>
#include <util/evaconstant.h>
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaExtractor.h>
#include <media/NdkMediaFormat.h>
#include <util/threadpool.h>
#include <util/speedcontrolutil.h>
#include <control/iframedecodecontroller.h>
#include <control/evalistenermanager.h>
#include <control/evaconfig.h>
#include <control/rendermanger.h>
#include <control/iframerendercontroller.h>
#include <functional>

#define LOG_TAG "VideoDecoder"
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ELOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

using namespace std;
class VideoDecoder: public IFrameDecodeController {
public:
    ANativeWindow *window;
    ANativeWindow *decodeWindow;
    ThreadPool* decodeThread;
    AMediaCodec *decoder;
    AMediaExtractor *extractor;
    EvaListenerManager* listener;
    EvaAnimeConfig* config;
    EvaConfig* configManager;
    IEvaFileContainer* container;

    int fps = 0; //帧率
    int playLoop = 0;  //播放循环次数
    bool isRunning = false; //是否正在运行
    bool isStopReq = false; //是否需要停止
    SpeedControlUtil* speedControlUtil;
    IFrameRenderController* renderManager;

    // 动画的原始尺寸
    int videoWidth = 0;
    int videoHeight = 0;

    // 动画对齐后的尺寸
    int alignWidth = 0;
    int alignHeight = 0;

    // 动画是否需要走YUV渲染逻辑的标志位
    bool needYUV = false;

    bool needDestroy = false;

    bool needStartPlay = false;

    VideoDecoder();
    ~VideoDecoder();

    void setSurface(ANativeWindow *window);
    void setDecodeSurface(ANativeWindow* window);
    void setParams(OptionParams* params);
    void startPlay(IEvaFileContainer* evaFileContainer);
    void startPlayMainProcess(IEvaFileContainer* evaFileContainer);
    void startPlayDecodeProcess(IEvaFileContainer* evaFileContainer);
    void startFrameDecode(IEvaFileContainer* evaFileContainer);
    void prepareDecoder(AMediaFormat* format);
    void startDecode();
    void setSrcBitmap(string name, unsigned char* bitmap, AndroidBitmapInfo* bitmapInfo, string scaleMode);
    void updateViewPoint(int width, int height);
    void renderFrame();
    void release();
    int getTextureId();
    bool isPlaying();
    void touchPoint(int x, int y);
    void setAnimListener(EvaListenerManager* listener);

    void preparePlay(int videoWidth, int videoHeight);

    void stop();
    void destroy();
    void destroyInner();
    bool createThread(ThreadPool* t, string name);
    void quitSafely(ThreadPool* t);

    const int32_t COLOR_FormatYUV420Planar = 19;
    int defaultFps = 30;
private:
    vector<EvaSrc*> evaVector;
    OptionParams* optionParams;
};


#endif //YYEVA_VIDEODECODER_H
