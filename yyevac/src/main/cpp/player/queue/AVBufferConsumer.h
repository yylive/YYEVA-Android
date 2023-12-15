//
// Created by zengjiale on 2023/12/13.
//

#ifndef YYEVA_AVBUFFERCONSUMER_H
#define YYEVA_AVBUFFERCONSUMER_H

#define TAG_AVBufferConsumer = "AVBufferConsumer"

#include <queue/BufferQueueContext.h>
#include <queue/Frame.h>
#include <queue/FrameQueue.h>

namespace yyeva {
    class AVBufferConsumer {
    public:
        AVBufferConsumer(BufferQueueContext *context);

        bool hasRemainingFrames();

        Frame *getLastFrame();

        Frame *getCurrentFrame();

        Frame *getNextFrame();

        Frame *peekReadableLocked();

        void popNext();

    private:
        BufferQueueContext *context;
        FrameQueue *frameQueue;
    };
}


#endif //YYEVA_AVBUFFERCONSUMER_H
