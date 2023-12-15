//
// Created by zengjiale on 2023/12/13.
//

#include "BufferQueueContext.h"

yyeva::BufferQueueContext::BufferQueueContext(yyeva::PacketQueue *packetQueue,
                                              yyeva::FrameQueue *frameQueue):
                                              packetQueue(unique_ptr<PacketQueue>(packetQueue)),
                                              frameQueue(unique_ptr<FrameQueue>(frameQueue)),
                                              bufferConsumerListener(){

}

yyeva::BufferQueueContext::~BufferQueueContext() {
    ELOGD(TAG_BUFFERQUEUECONTEXT, "free BufferQueueContext");
    packetQueue.release();
    frameQueue.release();
    packetQueue = nullptr;
    frameQueue = nullptr;
    bufferConsumerListener = nullptr;
}

void yyeva::BufferQueueContext::setConsumerListener(yyeva::IBufferConsumerListener *listener) {
    bufferConsumerListener = listener;
}

void yyeva::BufferQueueContext::reset() {
    ELOGD(TAG_BUFFERQUEUECONTEXT, "reset BufferQueueContext");
    if (packetQueue
        && packetQueue.get()) {
        packetQueue->reset();
    }
    if (frameQueue
        && frameQueue.get()) {
        frameQueue->reset();
    }
}

void yyeva::BufferQueueContext::clear() {
    ELOGD(TAG_BUFFERQUEUECONTEXT, "clear BufferQueueContext");
    if (packetQueue
        && packetQueue.get()) {
        packetQueue->clear();
    }
    if (frameQueue
        && frameQueue.get()) {
        frameQueue->clear();
    }
}
