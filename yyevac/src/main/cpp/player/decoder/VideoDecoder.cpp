//
// Created by zengjiale on 2023/12/15.
//

#include "VideoDecoder.h"

yyeva::VideoDecoder::VideoDecoder(yyeva::EvaPlayerContext *context, AVCodecContext *codecContext,
                                  yyeva::AVBufferProducer *producer, int streamIndex,
                                  double timeBase, double frameRate):
                                  BaseDecoder(context, codecContext, producer, streamIndex) {
    this->abort = 0;
    this->frameCount = 0;
    this->timeBase = timeBase;
    this->frameRate = FFMAX(1, frameRate); //不允许为0
    this->isDecodeFinished = false;
    this->isStarted = false;
    this->thread = nullptr;
}

yyeva::VideoDecoder::~VideoDecoder() = default;

void yyeva::VideoDecoder::start() {
    std::unique_lock<std::mutex> lock(mutex);
    if (!isStarted && streamIndex >= 0 && codecContext && producer) {
        isStarted = true;
        startDecode();
    }
}

void yyeva::VideoDecoder::stop() {
    std::unique_lock<std::mutex> lock(mutex);
    if (isStarted){
       isStarted = false;
       stopDecode();
    }
}

void yyeva::VideoDecoder::release() {
    stopDecode();
    BaseDecoder::release();
    timeBase = 0.0;
    frameRate = 0.0;
}

bool yyeva::VideoDecoder::hasEnoughBuffer() {
    //packet的数量，packet的缓存大小，packet的总时长，只要有一项满足了，就返回true
    bool ret = BaseDecoder::hasEnoughBuffer();
    if(timeBase > 0) {
        double duration = timeBase * this->producer->getDuration();
        return ret || duration > 1.0;
    }
    return ret;
}

int yyeva::VideoDecoder::decodeFrame(AVFrame *frame) {
    if (!codecContext) {
        return -1;
    }
    return BaseDecoder::decodeFrame(frame);
}

void yyeva::VideoDecoder::startDecode() {
    if (!thread) {
        thread = new Thread("VIDEO_DECODER_THREAD", [this] {
           onDecodeBuffer();
        });
    }
    ELOGD(TAG_VIDEO_DECODER, "start videoDecode");
    thread->start();
}

void yyeva::VideoDecoder::stopDecode() {
    ELOGD(TAG_VIDEO_DECODER, "stop videoDecode");
    abort = 1;
    this->producer->signal();
    free_thread(&thread);
}

int yyeva::VideoDecoder::onDecodeBuffer() {
    int ret;
    double pts, duration;
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        return AVERROR(ENOMEM);
    }

    bool isGotFrame;
    for(;;) {
        if (abort) {
            ret = -1;
            break;
        }
        isGotFrame = (ret = getVideoFrame(frame)) == 1;  //如果没有待解码的packet，会阻塞
        if (ret < 0) {
            break;
        }
        if (ret == 0) {//end of file
            frameCount = 0; //结束了就重置
        }

        if (!isGotFrame) { //如果没有拿到解码的帧，继续尝试获取
            continue;
        }
        pts = (frame->pts == AV_NOPTS_VALUE)? 0: frame->pts * timeBase;
        duration = 1/ frameRate;
        frameCount++; //记录解析多少帧了
        ELOGD(TAG_VIDEO_DECODER, "decode frameCount:%d", frameCount);
        ret = enqueueFrame(frame, frameCount - 1, pts, duration);
        av_frame_unref(frame);
        if (ret < 0) {
            break;
        }
    }
    av_frame_free(&frame);
    ELOGD(TAG_VIDEO_DECODER, "decode video finished!");
    return ret;
}

int yyeva::VideoDecoder::getVideoFrame(AVFrame *frame) {
    int ret = decodeFrame(frame);
    if (ret < 0) {
        return ret;
    }
    bool isEof = ret == 0; //end of file，要向解码器放入nullpacket
    bool isGotFrame = ret == 1;
    if (isGotFrame) {
        frame->pts= frame->best_effort_timestamp;//使用解码器推导出来的时间戳
        isDecodeFinished = false;
    }

    if (isEof) {
        isDecodeFinished = true;
        avcodec_flush_buffers(codecContext);
        ELOGD(TAG_VIDEO_DECODER, "eof, flush buffers");
    }
    ELOGD(TAG_VIDEO_DECODER, "getVideoFrame, ret:%d", ret);
    return ret;
}

int yyeva::VideoDecoder::enqueueFrame(AVFrame *srcFrame, int index, double pts, double duration) {
    ELOGD(TAG_VIDEO_DECODER, "enqueueFrame, requestFrameLocked start, wait writable frame");
    Frame *destFrame = producer->requestFrameLocked(); //找可以写入的帧，如果没有找到的话，会阻塞
    ELOGD(TAG_VIDEO_DECODER, "enqueueFrame, requestFrameLocked end, got free frame, abort:%d", abort);
    int ret = destFrame? 1:0;
    if (abort) {
        ret = -1;
    }
    if (ret <= 0) {
        return ret;
    }
    //拷贝frame数据
    if (destFrame) {
        destFrame->index = index;
        destFrame->uploaded = 0;
        destFrame->width = srcFrame->width;
        destFrame->height = srcFrame->height;
        destFrame->pts = pts;
        destFrame->duration = duration;
        av_frame_move_ref(destFrame->frame, srcFrame);
        producer->enqueueFrame(destFrame); //加入到队列，等待渲染
        ELOGD(TAG_VIDEO_DECODER,
              "enqueueFrame, index:%d, pts:%f, duration:%f, width:%d, height:%d",
              index,
              destFrame->pts,
              destFrame->duration,
              destFrame->width,
              destFrame->height);
    }
    return ret;
}
