//
// Created by zengjiale on 2022/8/15.
//

#include "nativeplayer.h"
#include <control/videodecoder.h>
#include <control/rendermanger.h>
#include <util/evafilecontainer.h>

NativePlayer::NativePlayer() :
        frameDecoder(new VideoDecoder()),
        window(nullptr),
        decodeWindow(nullptr),
        fileContainer(nullptr),
        listenerManager(nullptr),
        optionParams(nullptr) {
    audioEngine_ = std::make_shared<AudioEngine>();
}

NativePlayer::~NativePlayer() {
    if (frameDecoder != nullptr) {
        frameDecoder = nullptr;
    }
    if (window != nullptr) {
        window = nullptr;
    }
    decodeWindow = nullptr;
    fileContainer = nullptr;
    fileAddress.clear();
    optionParams = nullptr;
}

void NativePlayer::setSurface(ANativeWindow *window) {
    ELOGV("setSurface");
    if (window != nullptr) {
        this->window = window;
        frameDecoder->setSurface(window);
        if (optionParams != nullptr
            && (optionParams->isRemoteService && optionParams->isMainProcess)) {  //单进程
            if (!fileAddress.empty()) {
                startPlay(fileAddress);
            }
        }
    } else {
        ELOGV("window is null");
    }
}

void NativePlayer::startPlay(string fileAddress) {
//    isStartRunning = true;
    if (!fileAddress.empty()) {
        if (this->window != nullptr || this->decodeWindow != nullptr) {
            fileContainer = new EvaFileContainer(fileAddress);
            //视频解码
            startFrameDecode(fileContainer);
            //音频解码和播放音频
            if (optionParams != nullptr
                    && (!optionParams->isRemoteService  //单进程
                    || (optionParams->isRemoteService && !optionParams->isMainProcess))) {  //多进程的解码进程
                startAudioDecode(fileAddress);
            }
        } else {  //等待window被初始化后再play
            this->fileAddress = fileAddress;
        }
    } else {
        ELOGE("fileAddress is empty");
    }
}

void NativePlayer::startFrameDecode(IEvaFileContainer *fileContainer) {
    try {
        if (frameDecoder != nullptr) {
            frameDecoder->startPlay(fileContainer);
        }
    } catch (exception e) {
        ELOGE("startFrameDecode %s", e.what());
    }
}

void NativePlayer::startAudioDecode(string path) {
    if (audioEngine_ && !path.empty()) {
        if (audioEngine_->setDecoderDataSource(path.c_str())){
            audioEngine_->play();
        }
    }
}

void NativePlayer::setSrcBitmap(string name, unsigned char *bitmap, AndroidBitmapInfo *bitmapInfo, string scaleMode) {
    if (frameDecoder != nullptr) {
        frameDecoder->setSrcBitmap(name, bitmap, bitmapInfo, scaleMode);
    }
}

void NativePlayer::play() {

}

void NativePlayer::stop() {
    if (frameDecoder != nullptr) {
        frameDecoder->stop();
    }
}

void NativePlayer::pause() {

}

bool NativePlayer::isPlaying() {
    if (frameDecoder != nullptr) {
        return frameDecoder->isPlaying();
    }
    return false;
}

void NativePlayer::setParams(OptionParams *params) {
    optionParams = params;
    if (frameDecoder != nullptr) {
        frameDecoder->setParams(params);
    }
    if (audioEngine_) {
        audioEngine_->setLoop(params->playCount);
    }
}

void NativePlayer::renderFrame() {
    if (frameDecoder != nullptr) {
        frameDecoder->renderFrame();
    }
}

void NativePlayer::swapBuffers() {

}

void NativePlayer::release() {
    destroy();
}

void NativePlayer::destroy() {
    if (frameDecoder != nullptr) {
        frameDecoder->destroy();
        frameDecoder = nullptr;
    }
    if (audioEngine_) {
        audioEngine_->stop();
    }
    if (listenerManager != nullptr) {
        listenerManager = nullptr;
    }
    if (fileContainer != nullptr) {
        fileContainer->close();
        fileContainer = nullptr;
    }
    if (window != nullptr) {
        this->window = nullptr;
    }
    fileAddress.clear();
    decodeWindow = nullptr;
    fileContainer = nullptr;
    optionParams = nullptr;
}

void NativePlayer::updateViewPoint(int width, int height) {
    if (frameDecoder != nullptr) {
        frameDecoder->updateViewPoint(width, height);
    }
}

void NativePlayer::touchRect() {

}

int NativePlayer::getTextureId() {
    if (frameDecoder != nullptr) {
        return frameDecoder->getTextureId();
    }
    return -1;
}

void NativePlayer::setAnimListener(JavaVM *javaVm, jobject &listener) {
    if (listenerManager == nullptr) {
        listenerManager = new EvaListenerManager(javaVm);
    }
    listenerManager->setListener(listener);
    if (frameDecoder != nullptr) {
        frameDecoder->setAnimListener(listenerManager);
    }
}

void NativePlayer::setDecodeSurface(ANativeWindow *window) {
    if (frameDecoder != nullptr) {
        decodeWindow = window;
        frameDecoder->setDecodeSurface(window);
        if (!this->fileAddress.empty()) {
            startPlay(this->fileAddress);
        }
    }
}

void NativePlayer::touchPoint(int x, int y) {
    if (frameDecoder != nullptr) {
        frameDecoder->touchPoint(x, y);
    }
}
