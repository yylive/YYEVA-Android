//
// Created by 张保磊 on 2022/9/4.
//

#ifndef AUDIO_NATIVE_AUDIOENGINE_H
#define AUDIO_NATIVE_AUDIOENGINE_H

#include "android/asset_manager.h"
#include "AudioCodec.h"
#include "AudioPlayer.h"
#include <future>

class AudioEngine {

public:

    AudioEngine() {
        mAudioCodec = std::make_shared<AudioCodec>();
        mAudioPlayer = std::make_unique<AudioPlayer>();
    }

    bool setDecoderDataSource(AAssetManager &manager, const char *fileName);

    bool setDecoderDataSource(const char *path);

    void play();

    void replay();

    void setLoop(int32_t loop);

    void stop();

private:
    std::shared_ptr<AudioCodec> mAudioCodec;
    std::unique_ptr<AudioPlayer> mAudioPlayer;
    std::future<void> mLoadingResult;

    void requestStart();
};

#endif //AUDIO_NATIVE_AUDIOENGINE_H
