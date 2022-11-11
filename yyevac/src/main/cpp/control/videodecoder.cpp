//
// Created by zengjiale on 2022/8/16.
//

#include "videodecoder.h"

VideoDecoder::VideoDecoder():
        speedControlUtil(new SpeedControlUtil()),
        renderManager(nullptr),
        extractor(nullptr),
        decoder(nullptr),
        window(nullptr),
        decodeWindow(nullptr),
        container(nullptr),
        listener(nullptr),
        configManager(new EvaConfig()),
        decodeThread(nullptr),
        optionParams(nullptr) {
    createThread(decodeThread, "video_decode");
    if (speedControlUtil != nullptr) {
        speedControlUtil->setFixedPlaybackRate(30);
    }
}

VideoDecoder::~VideoDecoder() {
    if (speedControlUtil != nullptr) {
        speedControlUtil->reset();
        speedControlUtil = nullptr;
    }
    if (renderManager != nullptr) {
        renderManager->releaseTexture();
        renderManager->destroyRender();
        renderManager = nullptr;
    }
    extractor = nullptr;
    decoder = nullptr;
    window = nullptr;
    decodeWindow = nullptr;
    playLoop = 0;
    if (decodeThread != nullptr) {
//        decodeThread->shutdown();
        decodeThread = nullptr;
    }
    listener = nullptr;
    optionParams = nullptr;
}

void VideoDecoder::setSurface(ANativeWindow *window) {
    this->window = window;
    if (this->renderManager != nullptr) {
        this->renderManager->surface = window;
        if (renderManager->getExternalTexture() == -1) {

            //初始化render，默认不适用yuv
            int r = renderManager->initRender(window, false);
//        renderManager->mixRenderCreate();
            if (r < 0) {
                ELOGE("setSurface initRender error");
            }
        }
    }
}

void VideoDecoder::updateViewPoint(int width, int height) {
    if (renderManager != nullptr) {
        renderManager->updateViewPoint(width, height);
    }
}

void VideoDecoder::renderFrame() {
    if (isStopReq) {
        ELOGV("isStop");
        return;
    }
    if (renderManager != nullptr) {
//        ELOGV("renderFrame");
        renderManager->renderFrame();
    }
}

void VideoDecoder::setDecodeSurface(ANativeWindow* window) {
    decodeWindow = window;
}

void VideoDecoder::setParams(OptionParams *params) {
    optionParams = params;
    if (renderManager == nullptr
        && optionParams != nullptr
        && optionParams->isNeedRender()) {
        renderManager = new RenderManager();
    }

    if (renderManager != nullptr && params != nullptr) {
        renderManager->setParams(params);
        playLoop = params->playCount;
    }
}

void VideoDecoder::setAnimListener(EvaListenerManager* listener) {
    this->listener = listener;
}

void VideoDecoder::startPlayMainProcess(IEvaFileContainer* evaFileContainer) {
    ELOGV("startPlayMainProcess");
    isStopReq = false;
    try {
        if (configManager != nullptr) {
            ELOGV("start parseConfig");
            bool result = configManager->parseConfig(evaFileContainer,
                                                     EvaConstant::getInstance()->VIDEO_MODE_SPLIT_HORIZONTAL,
                                                     defaultFps);
            if (result != EvaConstant::getInstance()->OK) {
                if (listener != nullptr) {
                    listener->onFailed(result,
                                       EvaConstant::getInstance()->getErrorMsg(result,
                                                                               ""));
//                    listener->onVideoComplete();
                    release();
                    return;
                }
            }
            if (renderManager != nullptr) {
                configManager->config->listener = listener;
                renderManager->mixConfigCreate(configManager->config);
                renderManager->setMixSrc(evaVector);
            }
            if (speedControlUtil != nullptr) {
                speedControlUtil->setFixedPlaybackRate(configManager->config->fps);
            }
        }
        config = configManager->config;
        if (config != nullptr) {
            ELOGV("startFrameDecode");
            startFrameDecode(evaFileContainer);
        } else {
            ELOGV("config = null");
        }
    } catch (exception e) {
        ELOGE("startPlay error: %s", e.what());
    }
}

void VideoDecoder::startPlayDecodeProcess(IEvaFileContainer* evaFileContainer) {
    ELOGV("startPlayDecodeProcess");
    startFrameDecode(evaFileContainer);
}

void VideoDecoder::startPlay(IEvaFileContainer* evaFileContainer) {
    if (optionParams != nullptr && optionParams->isRemoteService) {
        if (optionParams->isMainProcess) {
            startPlayMainProcess(evaFileContainer);
        } else {
            startPlayDecodeProcess(evaFileContainer);
        }
    } else {
        isStopReq = false;
        try {
            if (configManager != nullptr) {
                ELOGV("start parseConfig");
                bool result = configManager->parseConfig(evaFileContainer,
                                                         EvaConstant::getInstance()->VIDEO_MODE_SPLIT_HORIZONTAL,
                                                         defaultFps);
                if (result != EvaConstant::getInstance()->OK) {
                    if (listener != nullptr) {
                        listener->onFailed(result,
                                           EvaConstant::getInstance()->getErrorMsg(result,
                                                                                   ""));
                        release();
                        return;
                    }
                }
                if (renderManager != nullptr) {
                    configManager->config->listener = listener;
                    renderManager->mixConfigCreate(configManager->config);
                    renderManager->setMixSrc(evaVector);
                }
                if (speedControlUtil != nullptr) {
                    speedControlUtil->setFixedPlaybackRate(configManager->config->fps);
                }
            }
            config = configManager->config;
            if (config != nullptr) {
                ELOGV("startFrameDecode");
                startFrameDecode(evaFileContainer);
            } else {
                ELOGV("config = null");
            }
        } catch (exception e) {
            ELOGE("startPlay error: %s", e.what());
        }
    }
}

void VideoDecoder::startFrameDecode(IEvaFileContainer* evaFileContainer) {
    AMediaFormat* format = nullptr;
    int trackIndex = 0;

    try {
        extractor = EvaMediaUtil::getInstance()->getExtractor(evaFileContainer);
        trackIndex = EvaMediaUtil::getInstance()->selectVideoTrack(extractor);
        if (trackIndex < 0) {
            ELOGE("No video track found");
//            throw("No video track found");
            return;
        }
        AMediaExtractor_selectTrack(extractor,trackIndex);
        format = AMediaExtractor_getTrackFormat(extractor, trackIndex);
        if (format == nullptr) {
            ELOGE("format is null");
            return;
        }
        //是否支持h265
        if (EvaMediaUtil::getInstance()->checkIsHevc(format)) {
            if (!EvaMediaUtil::getInstance()->checkSupportCodec(MIME_HEVC)) {
                ELOGE("MediaCodec exception");
                string error = EvaConstant::getInstance()->ERROR_MSG_HEVC_NOT_SUPPORT;
                error.append("support hevc");
                if (EvaMediaUtil::getInstance()->checkSupportCodec(MIME_HEVC)) {
                    error.append("true");
                } else {
                    error.append("false");
                }
                if (listener != nullptr) {
                    listener->onFailed(REPORT_ERROR_TYPE_HEVC_NOT_SUPPORT, error);
                }
                release();
                return;
            }
        }

        if (!AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_WIDTH,
                                   &videoWidth)) {
            ELOGE("get videoWidth error");
        }

        if (!AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_HEIGHT,
                                   &videoHeight)) {
            ELOGE("get videoHeight error");
        }
        // 防止没有INFO_OUTPUT_FORMAT_CHANGED时导致alignWidth和alignHeight不会被赋值一直是0
        alignWidth = videoWidth;
        alignHeight = videoHeight;
        ELOGV("Video size is %d x %d", videoWidth, videoHeight);

        // 由于使用mediacodec解码老版本素材时对宽度1500尺寸的视频进行数据对齐，解码后的宽度变成1504，导致采样点出现偏差播放异常
        // 所以当开启兼容老版本视频模式并且老版本视频的宽度不能被16整除时要走YUV渲染逻辑
        // 但是这样直接判断有风险，后期想办法改
        needYUV = videoWidth % 16 != 0;
        if (window != nullptr && renderManager != nullptr
            && renderManager->getExternalTexture() == -1 && !renderManager->initRender(window, needYUV)) {
//        if (!prepareRender(needYUV)) {
            ELOGE("prepareRender fail");
            const char *error = "render create fail";
            if (listener != nullptr) {
                listener->onFailed(REPORT_ERROR_TYPE_CREATE_RENDER,
                                   EvaConstant::getInstance()->ERROR_MSG_CREATE_RENDER);
            }
            release();
            return;
        }
        if (optionParams != nullptr &&
            (!optionParams->isRemoteService
             || (optionParams->isRemoteService && optionParams->isMainProcess))) {
            preparePlay(videoWidth, videoHeight);
        }

//        if (renderManager->getExternalTexture() != -1) {
//            //callback java init glTexture;
//        }
        if (renderManager != nullptr) {
            renderManager->renderClearFrame();
        }
    } catch (exception e) {
        ELOGE("startPlay %s",e.what());
    }

    if (optionParams != nullptr &&
        (!optionParams->isRemoteService
         || (optionParams->isRemoteService && !optionParams->isMainProcess))) {
        prepareDecoder(format);
    }
}

void VideoDecoder::prepareDecoder(AMediaFormat* format) {
    try {
        const char* mime;
        if (!AMediaFormat_getString(format, AMEDIAFORMAT_KEY_MIME,
                                    &mime)) {
            ELOGE("get videoHeight error");
        }
        decoder = AMediaCodec_createDecoderByType(mime);
        if (decoder == nullptr) {
            ELOGE("AMediaCodec_createDecoderByType create error");
            return;
        }

        if (needYUV) {
            AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_COLOR_FORMAT, COLOR_FormatYUV420Planar);
            AMediaCodec_configure(decoder, format, nullptr, nullptr, 0);
        } else {
            if (decodeWindow != nullptr) {
                ELOGV("decodeWindow");
                AMediaCodec_configure(decoder, format, decodeWindow, nullptr, 0);
            } else  {
                ELOGE("decode window is null");
            }
        }
        AMediaCodec_start(decoder);

        auto decodeWork = [&]() -> void
        {
            try {
                startDecode();
            } catch (exception e) {
                ELOGE("MediaCodec exception e=%s", e.what());
                string error = EvaConstant::getInstance()->ERROR_MSG_DECODE_EXC;
                error.append(e.what());
                if (listener != nullptr) {
                    listener->onFailed(REPORT_ERROR_TYPE_DECODE_EXC, error);
                }
                release();
            }
        };
        decodeThread->submit(decodeWork);
    } catch (exception e) {
        ELOGE("MediaCodec configure exception e=%s", e.what());
        string error = EvaConstant::getInstance()->ERROR_MSG_DECODE_EXC;
        error.append(e.what());
        if (listener != nullptr) {
            listener->onFailed(REPORT_ERROR_TYPE_DECODE_EXC, error);
        }
        release();
        return;
    }
}

void VideoDecoder::preparePlay(int videoWidth, int videoHeight) {
    if (configManager != nullptr) {
        configManager->defaultConfig(videoWidth, videoHeight);
        if (configManager->config != nullptr && renderManager != nullptr) {
            renderManager->mixRenderCreate();
            renderManager->setRenderConfig(configManager->config);
        }
    }
}

void VideoDecoder::startDecode() {
    if (decoder == nullptr || extractor == nullptr) {
        return;
    }
    ELOGV("startDecode");

    int TIMEOUT_USEC = 10000L;
    int inputChunk = 0;
    bool outputDone = false;
    bool inputDone = false;
    int frameIndex = 0;
    bool isLoop = false;
    isRunning = true;
    AMediaCodecBufferInfo bufferInfo;
    AMediaFormat* outputFormat;
    while(!outputDone && isRunning) {
        if (isStopReq) {
            ELOGV("stop decode");
            release();
            return;
        }

        if (!inputDone) {
            int inputBufIndex = AMediaCodec_dequeueInputBuffer(decoder, TIMEOUT_USEC);
            if (inputBufIndex >= 0) {
                size_t bufsize;
                uint8_t *buf = AMediaCodec_getInputBuffer(decoder, inputBufIndex, &bufsize);
                int chunkSize = AMediaExtractor_readSampleData(extractor, buf, bufsize);
                if (chunkSize < 0) {
                    AMediaCodec_queueInputBuffer(decoder,inputBufIndex, 0, 0, 0, AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM);
                    inputDone = true;
                    ELOGV("decode EOS");
                } else {
                    long presentationTimeUs = AMediaExtractor_getSampleTime(extractor);
                    AMediaCodec_queueInputBuffer(decoder, inputBufIndex, 0, chunkSize, presentationTimeUs, 0);
                    ELOGV("submitted frame %i to desc,size=%i", inputChunk, chunkSize);
                    inputChunk++;
                    AMediaExtractor_advance(extractor);
                }
            } else {
                ELOGV("input buffer not available");
            }
        }

        if (!outputDone) {
            ssize_t status = AMediaCodec_dequeueOutputBuffer(decoder, &bufferInfo, 1);
            if(status == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
                ELOGV("no output from decoder available");
            } else if(status == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {
                AMediaFormat_delete(outputFormat);
                outputFormat = nullptr;
                ELOGV("decoder output buffers changed");
            } else if(status == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
                outputFormat = AMediaCodec_getOutputFormat(decoder);
                if (outputFormat != nullptr) {
                    try {
                        string stride = "stride";
                        int strideSize = 0;
                        if (!AMediaFormat_getInt32(outputFormat, stride.c_str(), &strideSize)) {
                            ELOGE("get stride error");
                        }
                        string sliceHeight = "slice-height";
                        int sliceHeightSize = 0;
                        if (!AMediaFormat_getInt32(outputFormat, sliceHeight.c_str(),
                                                   &sliceHeightSize)) {
                            ELOGE("get slice-height error");
                        }
                        if (strideSize > 0 && sliceHeightSize > 0) {
                            alignWidth = strideSize;
                            alignHeight = sliceHeightSize;
                        }
                    } catch (exception e) {
                        ELOGE("output error %s", e.what());
                    }
                }
                ELOGV("decoder output format changed");
            } else if(status < 0) {
                ELOGE("unexpected result from decoder.dequeueOutputBuffer %i", status);
                isRunning = false;
                break;
            } else {
                int loop = 0;
                //到达结束标志位，且非暂停状态
                if (bufferInfo.flags == AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM && !isStopReq) {
                    ELOGV("AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM");
                    loop = --playLoop;
                    //playerEva.playLoop = playLoop
                    outputDone = playLoop <=0;
                    speedControlUtil->reset();
                }
                bool doRender = !outputDone;
                if (doRender) { //控制帧率时间
                    speedControlUtil->preRender(bufferInfo.presentationTimeUs);
                }

                if (needYUV && doRender) {
//                  yuvProcess(decoder, decoderStatus)
                }

                AMediaCodec_releaseOutputBuffer(decoder, status, doRender && !needYUV);
                if (frameIndex == 0 && !isLoop && listener != nullptr) {
                    listener->onVideoStart();
                }

                if (renderManager != nullptr) {
                    renderManager->onDecoding(frameIndex);
                }
                // onVideoRender(frameIndex, player->configManger->config);
                if (listener != nullptr && config != nullptr) {
                    listener->onVideoRender(frameIndex);
                }

                frameIndex++;
                ELOGV("decode frameIndex= %i", frameIndex);
                if (!isStopReq && loop > 0) {
                    ELOGV("Reached, looping loop %i", loop);
                    //playerEva.pluginManager.onLoopStart()
                    if (renderManager != nullptr) {
                        //重置误差计算
                        renderManager->onLoopStart();
                    }
                    AMediaExtractor_seekTo(extractor, 0, AMEDIAEXTRACTOR_SEEK_CLOSEST_SYNC);
                    inputDone = false;
                    AMediaCodec_flush(decoder);
                    speedControlUtil->reset();
                    frameIndex = 0;
                    isLoop = true;
                }

                if (isStopReq || outputDone) {
//                    if (renderManager != nullptr) {
//                        renderManager->onDecodeEnd(frameIndex);
//                        renderManager->setReleaseFunc(std::bind(&VideoDecoder::release, this));
////                    release();
//                    }
                    release();
                }
            }

        }
    }
}

void VideoDecoder::setSrcBitmap(string name, unsigned char *bitmap, AndroidBitmapInfo *bitmapInfo, string scaleMode) {
    if (bitmap == nullptr || bitmapInfo == nullptr) {
        ELOGE("setSrcBitmap bitmap null");
        return;
    }

    auto* src = new EvaSrc();
    src->srcTag = name;
    src->bitmapWidth = bitmapInfo->width;
    src->bitmapHeight = bitmapInfo->height;
    src->bitmapFormat = bitmapInfo->format;
    if (scaleMode == "aspectFit") {
        src->fitType = EvaSrc::CENTER_FIT;
    } else if (scaleMode == "aspectFill") {
        src->fitType = EvaSrc::CENTER_FULL;
    } else {
        src->fitType = EvaSrc::FIX_XY;
    }
    if (bitmapInfo->format == ANDROID_BITMAP_FORMAT_RGB_565) { //RGB三通道，例如jpg格式
        src->size = bitmapInfo->stride * bitmapInfo->height;
        src->bitmap = new unsigned char[src->size];
        memset(src->bitmap, 0, src->size);
        memcpy(src->bitmap, bitmap, src->size);
    } else if (bitmapInfo->format == ANDROID_BITMAP_FORMAT_RGBA_8888) {  //RGBA四通道，例如png格式 android图片是ARGB排列，所以还是要做图片转换的
        src->size = bitmapInfo->stride * bitmapInfo->height;
        src->bitmap = new unsigned char[src->size];
        memset(src->bitmap, 0, src->size);
        memcpy(src->bitmap, bitmap, src->size);
    }

    if (renderManager != nullptr
        && !renderManager->setSrcBitmap(name, bitmap, bitmapInfo, scaleMode)) { //初始化时查看是否json是否能正常解析成功，解析成功则添加
        evaVector.push_back(src);
    }
}

void VideoDecoder::release() {
    ELOGV("release");
//    if (decodeThread != nullptr) {
//        auto releaseWork = [&]() -> void
//        {
//            if (!isRunning) return;
//            isRunning = false;
//            try {
//                if (decoder != nullptr) {
//                    AMediaCodec_stop(decoder);
//                    AMediaCodec_delete(decoder);
//                }
//                if (extractor != nullptr) {
//                    AMediaExtractor_delete(extractor);
//                }
//                if (renderManager != nullptr) {
//                    renderManager->renderClearFrame();
//                    renderManager->releaseTexture();
//                }
//                speedControlUtil->reset();
//            } catch (exception e) {
//                ELOGE("releaseT %s", e.what());
//            }
//            isRunning = false;
//            if (listener != nullptr) {
//                listener->onVideoComplete();
//            }
//            if (needDestroy) {
//                destroyInner();
//            }
//        };
//        decodeThread->submit(releaseWork);
//    }
    if (!isRunning) return;

    try {
        //标志位变为停止
        isRunning = false;
        if (decoder != nullptr) {
            AMediaCodec_flush(decoder);
            AMediaCodec_stop(decoder);
            AMediaCodec_delete(decoder);
            decoder = nullptr;
        }
        if (extractor != nullptr) {
            AMediaExtractor_delete(extractor);
            extractor = nullptr;
        }
        if (renderManager != nullptr) {
            renderManager->renderClearFrame();
            renderManager->releaseTexture();
        }
        speedControlUtil->reset();
    } catch (exception e) {
        ELOGE("releaseT %s", e.what());
    }
    isRunning = false;
    playLoop = 0;
    if (listener != nullptr) {
        listener->onVideoComplete();
    }
    if (needDestroy) {
        destroyInner();
    }
}

void VideoDecoder::stop() {
    ELOGV("stop");
    isStopReq = true;
}

void VideoDecoder::destroy() {
    if (isRunning) {
        needDestroy = true;
//        stop();
        release();
    } else {
        destroyInner();
    }
}

void VideoDecoder::destroyInner() {
    ELOGV("destroyInner");
//    if (renderThread != nullptr) {
//        auto destroyWork = [&]() -> void
//        {
//            if (controller != nullptr) {
//                controller->destroyRender();
//            }
//            onVideoDestroy();
//            destroyThread();
//        };
//        renderThread->submit(destroyWork);
//    }
    if (listener != nullptr) {
        listener->onVideoDestroy();
    }
    if (renderManager != nullptr) {
        renderManager->destroyRender();
        renderManager->mixRenderDestroy();
    }
    if (decodeThread != nullptr) {
//        decodeThread->shutdown();
        decodeThread = nullptr;
    }
    if (listener != nullptr) {
        listener = nullptr;
    }
}

int VideoDecoder::getTextureId(){
    if (renderManager != nullptr) {
        return renderManager->getExternalTexture();
    }
    return -1;
}

bool VideoDecoder::isPlaying() {
    return isRunning;
}

void VideoDecoder::touchPoint(int x, int y) {
    if (renderManager != nullptr) {
        renderManager->touchPoint(x, y);
    }
}

bool VideoDecoder::createThread(ThreadPool *t, string name) {
    try {
        if (t == nullptr) {
            t = new ThreadPool(1);
            t->init();
            decodeThread = t;
        }
        return true;
    } catch (exception e) {
        ELOGE("createThread failed: %s", e.what());
    }
    return false;
}

void VideoDecoder::quitSafely(ThreadPool *t) {
    if (t != nullptr) {
        t->shutdown();
        t = nullptr;
    }
}
