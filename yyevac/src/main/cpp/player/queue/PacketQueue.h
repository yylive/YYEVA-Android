//
// Created by zengjiale on 2023/12/14.
//

#ifndef YYEVA_PACKETQUEUE_H
#define YYEVA_PACKETQUEUE_H

#define MAX_PACKET_QUEUE_NUM 32
#define VIDEO_PACKET_QUEUE_NUM 25
#define AUDIO_PACKET_QUEUE_NUM 16
#define PACKET_QUEUE_SIZE 4 * 1024 * 1024

#include <common/safequeue.h>
#include <mutex>
#include <condition_variable>

extern "C" {
#include <libavformat/avformat.h>
}

namespace yyeva {
    class PacketQueue {
    public:
        PacketQueue(int maxNum);

        ~PacketQueue();

        int push(AVPacket *pkt); //入队

        int pushLocked(AVPacket *pkt); //入队，没有空间则阻塞

        int pushNullPacket(int streamIndex); //空包入队

        int popLocked(AVPacket *pkt); //出队

        void signal();

        void reset(); //和signal配对使用

        int getSize();

        int getNumOfPackets();

        int getMaxNum();

        int64_t getDuration();

        void clear();

    private:
        int push(AVPacket *pkt, bool isNullPacket);

    private:
        int maxNum; //最大长度
        int memSize; //所有节点内存大小
        int abort;
        int64_t duration; //所有packet的时长
        SafeQueue<AVPacket *> queue;
        std::mutex mutex;
        std::condition_variable cond;
    };
}


#endif //YYEVA_PACKETQUEUE_H
