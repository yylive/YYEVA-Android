//
// Created by zengjiale on 2023/12/13.
//

#ifndef YYEVA_AVBUFFERPRODUCER_H
#define YYEVA_AVBUFFERPRODUCER_H

#include "BufferQueueContext.h"

namespace yyeva {
    class AVBufferProducer {
    public:
        AVBufferProducer(BufferQueueContext *context);

        int enqueuePacket(AVPacket *packet, bool isAllowDrop);

        void enqueueNullPacket(int streamIndex);

        int requestPacketLocked(AVPacket *packet);

        Frame *requestFrameLocked();

        void enqueueFrame(Frame *frame);

        bool hasEnoughPackets(); //是否缓存足够多的packet

        bool hasFrameRemaining();

        int64_t getDuration(); //获取packet的总时长

        void signal(); //唤醒阻塞的队列

    private:
        BufferQueueContext *context;
        PacketQueue *packetQueue;
        FrameQueue *frameQueue;
    };

}

#endif //YYEVA_AVBUFFERPRODUCER_H
