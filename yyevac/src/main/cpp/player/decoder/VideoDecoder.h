//
// Created by zengjiale on 2023/12/15.
//

#ifndef YYEVA_VIDEODECODER_H
#define YYEVA_VIDEODECODER_H

#include <decoder/BaseDecoder.h>
#include <src/main/cpp/util/thread.h>

#define TAG_VIDEO_DECODER "VIDEO_DECODER"

namespace yyeva {
    class VideoDecoder : public BaseDecoder {

    public:
        VideoDecoder(EvaPlayerContext *context,
                     AVCodecContext *codecContext,
                     AVBufferProducer *producer,
                     int streamIndex, double timeBase, double frameRate);

        ~VideoDecoder();

        void start() override;

        void stop() override;

        void release() override;

        bool hasEnoughBuffer() override;

    protected:
        int decodeFrame(AVFrame *frame) override;

    private:
        void startDecode();

        void stopDecode();

        int onDecodeBuffer();

        int getVideoFrame(AVFrame *frame);

        int enqueueFrame(AVFrame *srcFrame, int index, double pts, double duration);

    private:
        double timeBase;
        double frameRate;
        bool isStarted;
        std::mutex mutex;
        std::condition_variable cond;

        Thread *thread;
    };
}


#endif //YYEVA_VIDEODECODER_H
