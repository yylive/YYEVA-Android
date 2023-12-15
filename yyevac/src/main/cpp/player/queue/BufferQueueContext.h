//
// Created by zengjiale on 2023/12/13.
//

#ifndef YYEVA_BUFFERQUEUECONTEXT_H
#define YYEVA_BUFFERQUEUECONTEXT_H

#include <queue/IBufferConsumerListener.h>
#include <queue/PacketQueue.h>

#define TAG_BUFFERQUEUECONTEXT "BufferQueueContext"

namespace yyeva {
    class BufferQueueContext {
    public:
        friend class AVBufferProducer;
        friend class AVBufferConsumer;

        BufferQueueContext(PacketQueue *packetQueue, FrameQueue *frameQueue);

        virtual ~BufferQueueContext();

        void setConsumerListener(IBufferConsumerListener *listener);

        void reset();

        void clear();

    private:
        std::unique_ptr<PacketQueue> packetQueue;
        std::unique_ptr<FrameQueue> frameQueue;
        IBufferConsumerListener *bufferConsumerListener;
    };
}

#endif //YYEVA_BUFFERQUEUECONTEXT_H
