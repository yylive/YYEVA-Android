//
// Created by zengjiale on 2023/12/15.
//

#ifndef YYEVA_AUDIODECODER_H
#define YYEVA_AUDIODECODER_H

#include <decoder/BaseDecoder.h>
#include <src/main/cpp/util/thread.h>

#define TAG_AUDIODECODER "AUDIO_DECODER"

namespace yyeva {
    class AudioDecoder : public BaseDecoder {

    public:
        AudioDecoder(EvaPlayerContext *context,
                     AVCodecContext *codecContext,
                     AVBufferProducer *producer,
                     int streamIndex);

        void start() override;

        void stop() override;

        void release() override;

    private:
        void startDecode();

        void stopDecode();

        int onDecodeBuffer();

        int getAudioFrame(AVFrame *frame);

        int enqueueFrame(AVFrame *frame, double pts, double duration);

    private:
        std::mutex mutex;
        std::condition_variable cond;

        bool isStarted;
        int64_t nextPts;
        AVRational nextPts_tb;
        Thread *thread;
    };
}


#endif //YYEVA_AUDIODECODER_H
