//
// Created by 张保磊 on 2022/8/8.
//
#include "AudioPlayer.h"


bool AudioPlayer::start() {
    stop();
    if (!openAudioStream()) {
        return false;
    }
    Result playStartResult = mAudioStream->requestStart();
    return !(playStartResult != oboe::Result::OK);
}

void AudioPlayer::stop() {
    if (mAudioStream) {
        mAudioStream->stop();
        mAudioStream->close();
        mAudioStream.reset();
    }
    readIndex = 0;
}

bool AudioPlayer::openAudioStream() {
    AudioStreamBuilder builder;
    builder.setFormat(AudioFormat::I16);
    builder.setDirection(Direction::Output);
    builder.setSharingMode(SharingMode::Exclusive);
    builder.setSampleRateConversionQuality(SampleRateConversionQuality::Medium);
    builder.setPerformanceMode(PerformanceMode::LowLatency);
    builder.setChannelCount(channelCount_);
    builder.setSampleRate(sampleRate_);
    builder.setDataCallback(this);
    builder.setErrorCallback(this);
    Result result = builder.openStream(mAudioStream);
    if (result != Result::OK) {
        return false;
    }
    mAudioStream->setBufferSizeInFrames(mAudioStream->getFramesPerBurst() * 2);
    return true;
}

DataCallbackResult
AudioPlayer::onAudioReady(AudioStream *audioStream, void *audioData, int32_t numFrames) {
    if (mAudioStream->getState() != StreamState::Started) {
        return DataCallbackResult::Stop;
    }
    auto *targetData = static_cast<int16_t *>(audioData);
    const int64_t data_size = dataVec.size();
    if (data_size == 0) {
        return DataCallbackResult::Continue;
    }
    if (readIndex == data_size) {
        readIndex = 0;
        if (--mLoopCount > 0) {
            return DataCallbackResult::Continue;
        } else {
            return DataCallbackResult::Stop;
        }
    }
    int32_t needDataSize = numFrames * channelCount_;
    if (data_size - readIndex < needDataSize) {
        needDataSize = data_size - readIndex;
    }
    dataVec.copy(targetData, readIndex, needDataSize);
    readIndex += needDataSize;
    return DataCallbackResult::Continue;
}

void AudioPlayer::onErrorAfterClose(AudioStream *stream, Result result) {
    LOG_E("onErrorAfterClose %s", convertToText(result));
    if (result == Result::ErrorDisconnected) {
        mAudioStream.reset();
        readIndex = 0;
        start();
    }
}

