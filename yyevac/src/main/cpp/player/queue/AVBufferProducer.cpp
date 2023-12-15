//
// Created by zengjiale on 2023/12/13.
//

#include "AVBufferProducer.h"

yyeva::AVBufferProducer::AVBufferProducer(yyeva::BufferQueueContext *context):
    context(context),
    packetQueue(context->packetQueue.get()),
    frameQueue(context->frameQueue.get()){
    context->bufferConsumerListener = nullptr;
}

int yyeva::AVBufferProducer::enqueuePacket(AVPacket *packet, bool isAllowDrop) {
    AVPacket *target = av_packet_clone(packet);
    int ret = 0;
    if (target && packetQueue) {
        if (!isAllowDrop) { //不允许丢弃
            ret = packetQueue->pushLocked(target);
        } else {
            ret = packetQueue->push(target);
        }
    }
    if (ret <= 0 && target) {//入队失败就是释放，避免内存泄露
        av_packet_free(&target);
    }
    return ret;
}

void yyeva::AVBufferProducer::enqueueNullPacket(int streamIndex) {
    if (packetQueue) {
        packetQueue->pushNullPacket(streamIndex);
    }
}

int yyeva::AVBufferProducer::requestPacketLocked(AVPacket *packet) {
    if (packetQueue) {
        return packetQueue->popLocked(packet);
    }
    return 0;
}

yyeva::Frame *yyeva::AVBufferProducer::requestFrameLocked() {
    if (frameQueue) {
        return frameQueue->peekWritableLocked();
    }
    return nullptr;
}

void yyeva::AVBufferProducer::enqueueFrame(yyeva::Frame *frame) {
    int ret = 0;
    if(frameQueue) {
        ret = frameQueue->push(frame);
    }

    if (ret == 1 && context->bufferConsumerListener) {
        context->bufferConsumerListener->onFrameAvailable();
    }
}

bool yyeva::AVBufferProducer::hasEnoughPackets() {
    if (packetQueue) {
        ELOGD("AVBufferProducer", "packets:%d, size:%d", packetQueue->getNumOfPackets(),
              packetQueue->getSize());
        return packetQueue->getNumOfPackets() >= packetQueue->getMaxNum()
            || packetQueue->getSize() >= PACKET_QUEUE_SIZE;
    }
    return false;
}

bool yyeva::AVBufferProducer::hasFrameRemaining() {
    if (frameQueue) {
        return frameQueue->isRemainingNum() > 0;
    }
    return false;
}

int64_t yyeva::AVBufferProducer::getDuration() {
    if (packetQueue) {
        return packetQueue->getDuration();
    }
    return 0;
}

void yyeva::AVBufferProducer::signal() {
    if (packetQueue) {
        packetQueue->signal();
    }
    if (frameQueue) {
        frameQueue->signal();
    }
}
