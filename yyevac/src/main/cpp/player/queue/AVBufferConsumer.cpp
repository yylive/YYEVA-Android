//
// Created by zengjiale on 2023/12/13.
//

#include "AVBufferConsumer.h"

yyeva::AVBufferConsumer::AVBufferConsumer(yyeva::BufferQueueContext *context):
    context(context),
    frameQueue(context->frameQueue.get()){

}

bool yyeva::AVBufferConsumer::hasRemainingFrames() {
    if (frameQueue) {
        return frameQueue->isRemainingNum() > 0;
    }
    return false;
}

yyeva::Frame *yyeva::AVBufferConsumer::getLastFrame() {
    if (!frameQueue) {
        return nullptr;
    }
    Frame *ret = frameQueue->last();
    if (ret == nullptr) { //如果没有保留最后一帧，那就返回当前的
        ret = frameQueue->current();
    }

    return ret;
}

yyeva::Frame *yyeva::AVBufferConsumer::getCurrentFrame() {
    if (!frameQueue) {
        return nullptr;
    }
    return frameQueue->current();
}

yyeva::Frame *yyeva::AVBufferConsumer::getNextFrame() {
    if (!frameQueue) {
        return nullptr;
    }
    return frameQueue->next();
}

yyeva::Frame *yyeva::AVBufferConsumer::peekReadableLocked() {
    if (!frameQueue) {
        return NULL;
    }
    return frameQueue->peekReadableLocked();
}

void yyeva::AVBufferConsumer::popNext() {
    if (frameQueue) {
        frameQueue->pop();
    }
}
