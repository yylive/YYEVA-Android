//
// Created by zengjiale on 2023/12/13.
//

#ifndef YYEVA_FRAMEQUEUE_H
#define YYEVA_FRAMEQUEUE_H

#include <queue/Frame.h>
#include <common/safequeue.h>
#include <util/ELog.h>

#define TAG_FRAME_QUEUE "FrameQueue"
#define FRAME_QUEUE_SIZE 16 //默认帧缓存数量
#define VIDEO_FRAME_QUEUE_SIZE 3 //视频帧缓存数量，不适宜过大，解码后的帧缓存一般都是比较大的
#define AUDIO_FRAME_QUEUE_SIZE 9 //音频帧缓存数量

extern "C" {
    #include <libavformat/avformat.h>
};

namespace yyeva {
    class FrameQueue {
    public:
        FrameQueue(int maxSize, bool isKeepLast);

        ~FrameQueue();

        Frame *current(); //获取当前的Frame，不出队列

        Frame *last(); //不是队尾，是最后显示过的一帧

        Frame *next(); //下一帧

        Frame *peekWritableLocked();

        Frame *peekReadableLocked();

        int pop(); //出队，更新读索引，和peekReadable配合使用

        int push(Frame *frame); //入队，更新写索引，和peekWritable配合使用

        int isRemainingNum(); //是否还未处理过的帧

        int getRemainingDuration(); //为处理帧的总时长

        int getFramesMemSize(); //获取队列所有帧的内存大小

        void signal();

        void clear(); //释放所有frame

        void release();

        void reset();

    private:
        void initUnused();

        void initFrame(Frame *frame); //复位frame所有的参数

        void copyLastFrameFrom(Frame *frame);

    private:
        int maxSize; //队列最大长度
        int abort;
        bool isKeepLast; //队列保持最后一帧，只有clear的时候，才会被清除
        SafeQueue<Frame *> queue;
        SafeQueue<Frame *> unused;
        Frame *lastFrame; //最后一帧
        std::mutex mutex;
        std::condition_variable cond;
    };
}

#endif //YYEVA_FRAMEQUEUE_H
