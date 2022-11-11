//
// Created by 张保磊 on 2022/8/8.
//
#ifndef AUDIO_NATIVE_AUDIOPLAYER_H
#define AUDIO_NATIVE_AUDIOPLAYER_H

#include "AudioCodec.h"
#include "oboe/Oboe.h"
#include "DataVector.h"

using namespace oboe;


class AudioPlayer : public AudioStreamErrorCallback, AudioStreamDataCallback {

public:
    AudioPlayer(int32_t sampleRate,
                int32_t channelCount) : sampleRate_(sampleRate), channelCount_(channelCount) {};

    AudioPlayer() {};

    ~AudioPlayer() { dataVec.clear(); };

    DataCallbackResult
    onAudioReady(AudioStream *audioStream, void *audioData, int32_t numFrames) override;

    void onErrorAfterClose(AudioStream *, Result) override;


    void setAudioData(const int16_t *data, const int32_t size) {
        dataVec.set(data, size);
    }

    DataVector<int16_t> &dataVector() {
        return dataVec;
    };

    void setSampleRate(int32_t sampleRate) {
        sampleRate_ = sampleRate;
    }

    void setChannelCount(int32_t channelCount) {
        channelCount_ = channelCount;
    }

    void setLoopCount(int32_t loop) {
        mLoopCount = loop;
    }

    void stop();

    bool start();

private:
    std::shared_ptr<AudioStream> mAudioStream;
    DataVector<int16_t> dataVec;
    std::atomic_int64_t readIndex{0};
    int32_t sampleRate_ = 48000;
    int32_t channelCount_ = 2;
    std::atomic_int32_t mLoopCount{1};

    bool openAudioStream();
};

#endif //AUDIO_NATIVE_AUDIOPLAYER_H
