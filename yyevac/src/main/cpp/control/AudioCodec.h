//
// Created by 张保磊 on 2022/8/8.
//

#ifndef AUDIO_NATIVE_AUDIOCODEC_H
#define AUDIO_NATIVE_AUDIOCODEC_H

#include "media/NdkMediaExtractor.h"
#include "media/NdkMediaCodec.h"
#include <android/asset_manager.h>
#include <android/log.h>
#include <memory>
#include "DataVector.h"

#define LOG_TAG "AUDIO_CODEC"
#define LOG_E(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOG_V(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)


class AudioCodec {
public:

    AudioCodec() {
    };

    ~AudioCodec() = default;

    int32_t decode(DataVector<int16_t> &decodeData);

    int32_t setDataSource(AAsset *asset);

    int32_t setDataSource(const char *path);

    bool openMediaCodec(int32_t &sampleRate,
                        int32_t &channelCount);
    void stop();

private:
    AMediaCodec *mediaCodec = nullptr;
    AMediaExtractor *extractor = nullptr;
    std::atomic_bool isDecoding{false};

    int32_t decodeAudioData(DataVector<int16_t> &decodeData, bool &isDecoding);

    void extractAudioData(bool &isExtracting);

    AMediaCodec *openMediaCodec(AMediaFormat *format, size_t trackIndex, const char *mine);
};


#endif //AUDIO_NATIVE_AUDIOCODEC_H
