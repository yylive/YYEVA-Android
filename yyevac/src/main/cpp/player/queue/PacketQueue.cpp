//
// Created by zengjiale on 2023/12/14.
//

#include "PacketQueue.h"

yyeva::PacketQueue::PacketQueue(int maxNum):
    memSize(0),
    duration(0),
    abort(0),
    maxNum(maxNum),
    cond() {

}

yyeva::PacketQueue::~PacketQueue() {
    clear();
}

int yyeva::PacketQueue::push(AVPacket *pkt) {
    return push(pkt, false);
}

int yyeva::PacketQueue::pushLocked(AVPacket *pkt) {
    std::unique_lock<std::mutex> lock(mutex);
    if (!pkt) {
        return 0;
    }
    while (!abort
           && queue.size() > maxNum) {
        cond.wait(lock); //等待有可用的空间
    }
    if (!abort) {
        queue.push(pkt);
        duration += pkt->duration; //记录总时长
        memSize += pkt->size; //记录总大小
        cond.notify_all();
        return 1;
    }
    return 0;
}

int yyeva::PacketQueue::pushNullPacket(int streamIndex) {
    AVPacket *pkt = av_packet_alloc();
    av_init_packet(pkt);
    pkt->data = nullptr;
    pkt->size = 0;
    pkt->pts = -1;
    pkt->dts = 01;
    pkt->duration = 0;
    pkt->stream_index = streamIndex;
    return push(pkt, true);
}

int yyeva::PacketQueue::popLocked(AVPacket *pkt) {
    std::unique_lock<std::mutex> lock(mutex);
    AVPacket *find = nullptr;
    int ret = 0;
    while (!abort) {
        if (queue.pop(find)) { //如果找到了
            memSize -= find->size;
            duration -= find->duration;
            *pkt = * find;
            ret = 1;
            cond.notify_all();
            break;
        } else { //找不到就继续等待
            cond.wait(lock);
        }
    }
    return ret;
}

void yyeva::PacketQueue::signal() {
    std::unique_lock<std::mutex> lock(mutex);
    abort = 1;
    cond.notify_all();
}

void yyeva::PacketQueue::reset() {
    std::unique_lock<std::mutex> lock(mutex);
    abort = 0;
    cond.notify_all();
}

int yyeva::PacketQueue::getSize() {
    std::unique_lock<std::mutex> lock(mutex);
    return memSize;
}

int yyeva::PacketQueue::getNumOfPackets() {
    std::unique_lock<std::mutex> lock(mutex);
    return queue.size();
}

int yyeva::PacketQueue::getMaxNum() {
    std::unique_lock<std::mutex> lock(mutex);
    return maxNum;
}

int64_t yyeva::PacketQueue::getDuration() {
    std::unique_lock<std::mutex> lock(mutex);
    return duration;
}

void yyeva::PacketQueue::clear() {
    AVPacket *packet;
    std::unique_lock<std::mutex> lock(mutex);
    abort = 1;
    while (queue.size() > 0) {
        if (queue.pop(packet)) {
            av_packet_free(&packet);
        }
    }
    duration = 0;
    memSize = 0;
    cond.notify_all();
}

int yyeva::PacketQueue::push(AVPacket *pkt, bool isNullPacket) {
    std::unique_lock<std::mutex> lock(mutex);
    if (!pkt) {
        return 0;
    }
    if (!isNullPacket && queue.size() > maxNum) {
        return -1; //丢弃处理
    }
    queue.push(pkt);
    duration += pkt->duration; //记录总时长
    memSize += pkt->size; //记录总大小
    cond.notify_all();
    return 0;
}
