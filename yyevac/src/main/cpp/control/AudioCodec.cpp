//
// Created by 张保磊 on 2022/8/8.
//
#include "AudioCodec.h"
#include "android/asset_manager_jni.h"

AMediaCodec *AudioCodec::openMediaCodec(AMediaFormat *format, size_t trackIndex, const char *mine) {
    AMediaExtractor_selectTrack(extractor, trackIndex);
    auto codec = AMediaCodec_createDecoderByType(mine);
    AMediaCodec_configure(codec, format, nullptr, nullptr, 0);
    media_status_t status = AMediaCodec_start(codec);
    LOG_V("start media codec %d", status);
    AMediaFormat_delete(format);
    return codec;
}

bool AudioCodec::openMediaCodec(int32_t &sampleRate, int32_t &channelCount) {
    size_t trackCount = AMediaExtractor_getTrackCount(extractor);
    LOG_V("openMediaCodec %zu", trackCount);
    for (size_t i = 0; i < trackCount; i++) {
        auto format = AMediaExtractor_getTrackFormat(extractor, i);
        const char *mine = AMediaFormat_toString(format);
        if (!AMediaFormat_getString(format, AMEDIAFORMAT_KEY_MIME, &mine)) {
            AMediaFormat_delete(format);
            continue;
        }
        if (!strncmp(mine, "audio/", 6)) {
            AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_SAMPLE_RATE, &sampleRate);
            AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_CHANNEL_COUNT, &channelCount);
            mediaCodec = openMediaCodec(format, i, mine);
        } else {
            AMediaFormat_delete(format);
        }
    }
    return mediaCodec != nullptr;
}

int32_t AudioCodec::setDataSource(AAsset *asset) {
    off64_t out_start, out_length;
    int fd = AAsset_openFileDescriptor64(asset, &out_start, &out_length);
    if (fd < 0) {
        LOG_V("AAsset_openFileDescriptor64 error %d", fd);
        return -1;
    }
    if (!extractor) {
        AMediaExtractor_delete(extractor);
        extractor = nullptr;
    }
    extractor = AMediaExtractor_new();
    media_status_t status = AMediaExtractor_setDataSourceFd(extractor, fd, out_start, out_length);
    if (status != AMEDIA_OK) {
        LOG_E("set data source error:%d", status);
        AMediaExtractor_delete(extractor);
        extractor = nullptr;
        return -1;
    }
    return 0;
}

int32_t AudioCodec::setDataSource(const char *path) {
    FILE *file = fopen(path, "rb");
    fseek(file, 0, SEEK_END); //定位到文件末
    off64_t dataSize = ftell(file); //文件长度
    if (!extractor) {
        AMediaExtractor_delete(extractor);
        extractor = nullptr;
    }
    extractor = AMediaExtractor_new();
    media_status_t status = AMediaExtractor_setDataSourceFd(extractor, fileno(file), 0,
                                                            dataSize);
    fclose(file);
    if (status != AMEDIA_OK) {
        LOG_E("set data source error:%d", status);
        AMediaExtractor_delete(extractor);
        extractor = nullptr;
        return -1;
    }
    return 0;
}

void AudioCodec::extractAudioData(bool &isExtracting) {
    ssize_t buf_idx = AMediaCodec_dequeueInputBuffer(mediaCodec, 1000);
    if (buf_idx <= 0) {
        LOG_V("buf_idx:%zd", buf_idx);
        return;
    }
    size_t buf_size;
    auto input_buf = AMediaCodec_getInputBuffer(mediaCodec, buf_idx, &buf_size);
    ssize_t sampleDataSize = AMediaExtractor_readSampleData(extractor, input_buf, buf_size);
    if (sampleDataSize < 0) {
        sampleDataSize = 0;
        isExtracting = false;
    }
    auto sampleTime = AMediaExtractor_getSampleTime(extractor);
    const uint32_t flag = !isExtracting ? AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM : 0;
    AMediaCodec_queueInputBuffer(mediaCodec, buf_idx, 0, sampleDataSize, sampleTime, flag);
    AMediaExtractor_advance(extractor);
}

int32_t AudioCodec::decodeAudioData(DataVector<int16_t> &decodeData, bool &isDecoding) {
    AMediaCodecBufferInfo info;
    auto output_index = AMediaCodec_dequeueOutputBuffer(mediaCodec, &info, 1000);
    if (output_index < 0) {
        LOG_E("output_index %zd", output_index);
        AMediaCodec_releaseOutputBuffer(mediaCodec, output_index, false);
        return 0;
    }
    if (info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) {
        isDecoding = false;
    }
    size_t out_size;
    uint8_t *out_buffer = AMediaCodec_getOutputBuffer(mediaCodec, output_index, &out_size);
    size_t data_size = info.size;
    if (out_buffer && data_size != 0) {
        auto len = data_size / sizeof(int16_t);
        const auto *audioData = reinterpret_cast<int16_t *>(out_buffer);
        decodeData.set(audioData, len);
    }
    AMediaCodec_releaseOutputBuffer(mediaCodec, output_index, false);
    return data_size;
}

int32_t AudioCodec::decode(DataVector<int16_t> &decodeData) {
    LOG_V("decode start");
    if (!mediaCodec || !extractor) {
        if (!extractor) {
            AMediaExtractor_delete(extractor);
        }
        if (!mediaCodec) {
            AMediaCodec_delete(mediaCodec);
        }
        return 0;
    }
    decodeData.clear();
    bool isExtracting = true;
    bool isDecoding = true;
    int32_t decodedSize = 0;
    isDecoding = true;
    while (isExtracting || isDecoding) {
        if (isExtracting) {
            extractAudioData(isExtracting);
        }
        if (isDecoding) {
            decodedSize += decodeAudioData(decodeData, isDecoding);
        }
    }
    LOG_V("decode end");
    AMediaExtractor_delete(extractor);
    AMediaCodec_delete(mediaCodec);
    isDecoding = false;
    return decodedSize;
}

void AudioCodec::stop() {
    if (isDecoding && mediaCodec) {
        LOG_V("AudioCodec::stop");
        AMediaCodec_stop(mediaCodec);
        isDecoding = false;
    }
}


