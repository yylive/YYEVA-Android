//
// Created by zengjiale on 2023/12/15.
//

#ifndef YYEVA_BASEDECODER_H
#define YYEVA_BASEDECODER_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
};

#include <src/main/cpp/player/queue/AVBufferProducer.h>
#include <src/main/cpp/player/EvaPlayerContext.h>
#include <src/main/cpp/util/ELog.h>

#define TAG_BASEDEOCDER "BaseDecoder"

namespace yyeva {
    class BaseDecoder {
    public:
        BaseDecoder(EvaPlayerContext *context,
                    AVCodecContext *codecContext,
                    AVBufferProducer *producer, int streamIndex);

        virtual ~BaseDecoder() = default;

        virtual void start() = 0;

        virtual void stop() = 0;

        virtual void release();

        virtual bool hasEnoughBuffer() { //是否有足够的buff
            if (producer) {
                return producer->hasEnoughPackets();
            }
            return false;
        }

        virtual bool hasFramesRemaining() { //是否还有没有上屏的frame
            return producer->hasFrameRemaining();
        }

        virtual bool isFinished() { //是否解码完成了
            return isDecodeFinished.load();
        }

        int enqueuePacket(AVPacket *packet, bool isAllowDrop) {
            if (producer
                && streamIndex >= 0
                && streamIndex == packet->stream_index) {
                return producer->enqueuePacket(packet, isAllowDrop);
            }
            return 0;
        }

        void enqueueNullPacket() {
            if (producer
                && streamIndex >= 0) {
                ELOGD(TAG_BASEDEOCDER, "enqueueNullPacket, streamIndex:%d", streamIndex);
                producer->enqueueNullPacket(streamIndex);
            }
        }

    protected:
        virtual int decodeFrame(AVFrame *frame);

    protected:
        int abort; //中断信号
        int streamIndex;
        int frameCount;
        bool isPacketResend; //是否需要重新发送给解码器
        std::atomic_bool isDecodeFinished; //是否解码结束
        AVCodecContext *codecContext; //解码器上下文
        EvaPlayerContext *context;
        AVBufferProducer *producer;
        AVPacket tempPacket;
        AVPacket pendingPacket;
    };
}


#endif //YYEVA_BASEDECODER_H
