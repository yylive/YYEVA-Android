//
// Created by 张保磊 on 2022/9/4.
//

#include "AudioEngine.h"


void AudioEngine::stop() {
    mAudioPlayer->setLoopCount(1);
    mAudioPlayer->stop();
    mAudioCodec->stop();
}

bool AudioEngine::setDecoderDataSource(AAssetManager &manager, const char *fileName) {
    if (!fileName || strlen(fileName) == 0) {
        LOG_E("file is wrong");
        return false;
    }
    auto asset = AAssetManager_open(&manager, fileName, AASSET_MODE_UNKNOWN);
    if (!asset) {
        LOG_E("can not open asset manager");
        return false;
    }
    if (mAudioCodec->setDataSource(asset) != 0) {
        LOG_V("setDataSource failed");
        return false;
    }
    return true;
}

bool AudioEngine::setDecoderDataSource(const char *path) {
    if (!path || strlen(path) == 0) {
        LOG_E("file is wrong");
        return false;
    }
    if (mAudioCodec->setDataSource(path) != 0) {
        LOG_V("setDataSource failed %s", path);
        return false;
    }
    return true;
}

void AudioEngine::requestStart() {
    int32_t sampleRate = 48000, channelCount = 2;
    if (!mAudioCodec->openMediaCodec(sampleRate, channelCount)) {
        LOG_V("codec is null");
        return;
    }
    mAudioPlayer->setSampleRate(sampleRate);
    mAudioPlayer->setChannelCount(channelCount);
    if (!mAudioPlayer->start()) {
        return;
    }
    mAudioCodec->decode(mAudioPlayer->dataVector());
}

void AudioEngine::play() {
    LOG_V("AudioEngine start");
    mLoadingResult = std::async(&AudioEngine::requestStart, this);
}

void AudioEngine::replay() {
    if (mAudioPlayer->dataVector().size() == 0) {
        return;
    }
    mAudioPlayer->start();
}

void AudioEngine::setLoop(int32_t loop) {
    if (loop < 1) {
        loop = 1;
    }
    mAudioPlayer->setLoopCount(loop);
}



