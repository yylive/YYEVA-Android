//
// Created by zengjiale on 2023/12/13.
//

#include "FrameQueue.h"

yyeva::FrameQueue::FrameQueue(int maxSize, bool isKeepLast):
    maxSize(FFMIN(maxSize, FRAME_QUEUE_SIZE)),
    isKeepLast(isKeepLast),
    lastFrame(nullptr),
    mutex(),
    abort(0),
    cond(){
    initUnused(); //初始化unused队列
}

yyeva::FrameQueue::~FrameQueue() {
    release();
}

yyeva::Frame *yyeva::FrameQueue::current() {
    std::unique_lock<std::mutex> lock(mutex);
    Frame *find = nullptr;
    queue.peek(find);
    return find;
}

yyeva::Frame *yyeva::FrameQueue::last() {
    std::unique_lock<std::mutex> lock(mutex);
    return lastFrame;
}

yyeva::Frame *yyeva::FrameQueue::next() {
    return nullptr;
}

yyeva::Frame *yyeva::FrameQueue::peekWritableLocked() {
    std::unique_lock<std::mutex> lock(mutex);
    ELOGD(TAG_FRAME_QUEUE, "peekWritableLocked, curSize:%d, unusedSize:%d, maxSize:%d",
          queue.size(), unused.size(), maxSize);
    while (!abort && unused.size() == 0) { //如果没有可用的，就等待
        ELOGD(TAG_FRAME_QUEUE, "peekWritableLocked, wait!");
        cond.wait(lock);
    }
    if (!abort && unused.size() > 0) {
        Frame *find = nullptr;
        unused.pop(find);
        return find;
    }
    return nullptr;
}

yyeva::Frame *yyeva::FrameQueue::peekReadableLocked() {
    return nullptr;
}

int yyeva::FrameQueue::pop() {
    std::unique_lock<std::mutex> lock(mutex);
    Frame *find = nullptr;
    if (queue.pop(find)) {
        if (isKeepLast) {
            copyLastFrameFrom(find); //复制，保留最后一帧
        }
        initFrame(find); //重置后放入unused
        unused.push(find);
        cond.notify_all();
        ELOGD(TAG_FRAME_QUEUE, "pop, curSize:%d, unusedSize:%d", queue.size(), unused.size());
        return 1;
    }
    return 0;
}

int yyeva::FrameQueue::push(yyeva::Frame *frame) {
    std::unique_lock<std::mutex> lock(mutex);
    if (queue.size() + unused.size() >= maxSize) { //满了不能写入
        ELOGD(TAG_FRAME_QUEUE, "push more than maxsize");
        return 0;
    }
    queue.push(frame);
    cond.notify_all();
    ELOGD(TAG_FRAME_QUEUE, "push, curSize:%d, unusedSize:%d", queue.size(), unused.size());
    return 1;
}

int yyeva::FrameQueue::isRemainingNum() { //是否还有未处理的帧
    std::unique_lock<std::mutex> lock(mutex);
    return queue.size() > 0;
}

int yyeva::FrameQueue::getRemainingDuration() {
    return 0;
}

int yyeva::FrameQueue::getFramesMemSize() {
    return 0;
}

void yyeva::FrameQueue::signal() {
    ELOGD(TAG_FRAME_QUEUE, "signal");
    unique_lock<std::mutex> lock(mutex);
    abort = 1;
    cond.notify_all();
}

void yyeva::FrameQueue::clear() {
    ELOGD(TAG_FRAME_QUEUE, "clear");
    unique_lock<std::mutex> lock(mutex);
    release();
    initUnused();
}

void yyeva::FrameQueue::release() {
    ELOGD(TAG_FRAME_QUEUE, "release");
    abort = 1;
    if (lastFrame) {
        if(lastFrame->frame) {
            av_frame_free(&lastFrame->frame);
        }
        free(lastFrame);
        lastFrame = nullptr;
    }
    while (queue.size() > 0) {
        Frame *find = nullptr;
        if (queue.pop(find)) {
            av_frame_free(&find->frame);
            free(find);
        }
    }

    while (unused.size() > 0) {
        Frame *find = nullptr;
        if (queue.pop(find)) {
            av_frame_free(&find->frame);
            free(find);
        }
    }
    cond.notify_all();
}

void yyeva::FrameQueue::reset() {
    ELOGD(TAG_FRAME_QUEUE, "reset");
    unique_lock<std::mutex> lock(mutex);
    abort = 0;
    cond.notify_all();
}

void yyeva::FrameQueue::initUnused() {
    for (int i = 0; i < maxSize; ++i) {
        auto *frame = (Frame *) malloc(sizeof (Frame));
        if (frame) {
            memset(frame, 0, sizeof (Frame));
            frame->frame = av_frame_alloc();
            initFrame(frame);
            unused.push(frame);
        } else {
            ELOGD(TAG_FRAME_QUEUE, "malloc frame error, index %d", i);
        }
    }
}

void yyeva::FrameQueue::initFrame(yyeva::Frame *frame) {
    if (frame) {
        if (frame->frame) {
            av_frame_unref(frame->frame);
        }
        frame->index = -1;
        frame->width = 0;
        frame->height = 0;
        frame->uploaded = 0;
        frame->pts = 0;
        frame->duration = 0;
    }
}

void yyeva::FrameQueue::copyLastFrameFrom(yyeva::Frame *frame) {
    if (!lastFrame) {
        lastFrame = (Frame *) malloc(sizeof (Frame));
        if (lastFrame) {
            memset(lastFrame, 0, sizeof (Frame));
            lastFrame->frame = av_frame_alloc();
            initFrame(lastFrame);
        }
    }
    if (lastFrame) {
        lastFrame->index = frame->index;
        lastFrame->width = frame->width;
        lastFrame->height = frame->height;
        lastFrame->uploaded = frame->uploaded;
        lastFrame->pts = frame->pts;
        lastFrame->duration = frame->duration;
        if (lastFrame->frame) {
            av_frame_unref(lastFrame->frame); //移除上次的引用，避免内存泄露
            if (frame->frame) {
                av_frame_ref(lastFrame->frame, frame->frame);
            }
        }
    }
}
