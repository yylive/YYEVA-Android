//
// Created by zengjiale on 2023/12/15.
//

#include "AudioDecoder.h"

yyeva::AudioDecoder::AudioDecoder(yyeva::EvaPlayerContext *context, AVCodecContext *codecContext,
                                  yyeva::AVBufferProducer *producer, int streamIndex) : BaseDecoder(
        context, codecContext, producer, streamIndex),
        cond(),
        mutex(),
        isStarted(false),
        nextPts(AV_NOPTS_VALUE),
        nextPts_tb(),
        thread(nullptr){

}

void yyeva::AudioDecoder::start() {
    std::unique_lock<std::mutex> lock(mutex);
    if (!isStarted
        && streamIndex >= 0
        && codecContext
        && producer) {
        isStarted = true;
        startDecode();
    }
}

void yyeva::AudioDecoder::stop() {
    if (!thread) {
        thread = new Thread("AUDIO_DECODER_THREAD", [this] {
            onDecodeBuffer();
        });
    }
    ELOGD(TAG_AUDIODECODER, "start audioDecoder");
    thread->start();
}

void yyeva::AudioDecoder::release() {
    stopDecode();
    BaseDecoder::release();
}

void yyeva::AudioDecoder::startDecode() {
    if (!thread) {
        thread = new Thread("AUDIO_DECODER_THREAD", [this] {
            onDecodeBuffer();
        });
    }
    ELOGD(TAG_AUDIODECODER, "start audioDecoder");
    thread->start();
}

void yyeva::AudioDecoder::stopDecode() {
    ELOGD(TAG_AUDIODECODER, "stop audioDecode");
    abort = 1;
    this->producer->signal();
    free_thread(&thread);
}

int yyeva::AudioDecoder::onDecodeBuffer() {
    int ret;
    double pts, duration;
    AVRational tb;
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        return AVERROR(ENOMEM);
    }
    bool isGotFrame;
    for (;;) {
        if (abort) {
            ret = -1;
            break;
        }
        isGotFrame = (ret = getAudioFrame(frame)) == 1; //如果没有待解码的packet，会阻塞
        if (ret < 0) {
            break;
        }
        if (!isGotFrame) { //如果没有拿到解码的帧，继续尝试获取
            continue;
        }
        tb = (AVRational) {1, frame->sample_rate}; //采样率
        pts = (frame->pts == AV_NOPTS_VALUE) ? 0: frame->pts * av_q2d(tb);
        duration = av_q2d((AVRational) {frame->nb_samples, frame->sample_rate});
        ret = enqueueFrame(frame, pts, duration);
        av_frame_unref(frame);
        if (ret < 0) {
            break;
        }
    }
    av_frame_free(&frame);
    ELOGD(TAG_AUDIODECODER, "decode audio finished!");
    return ret;
}

int yyeva::AudioDecoder::getAudioFrame(AVFrame *frame) {
    int ret = decodeFrame(frame);
    if (ret < 0) {
        return ret;
    }
    bool isEof = ret == 0;
    bool isGotFrame = ret == 1;

    if (isGotFrame) {
        AVRational tb = (AVRational) {1, frame->sample_rate};
        if (frame->pts != AV_NOPTS_VALUE) {
            frame->pts =  av_rescale_q(frame->pts, codecContext->pkt_timebase, tb);
        } else if (nextPts != AV_NOPTS_VALUE) {
            frame->pts = av_rescale_q(nextPts, nextPts_tb, tb);
        }
        if (frame->pts != AV_NOPTS_VALUE) {
            nextPts = frame->pts + frame->nb_samples;
            nextPts_tb = tb;
        }
        isDecodeFinished = false;
    }
    if (isEof) {
        isDecodeFinished = true;
        avcodec_flush_buffers(codecContext);
    }
    ELOGD(TAG_AUDIODECODER, "getAudioFrame, ret:%d", ret);
    return ret;
}

int yyeva::AudioDecoder::enqueueFrame(AVFrame *srcFrame, double pts, double duration) {
    ELOGD(TAG_AUDIODECODER, "enqueueFrame, requestFrameLocked start, wait writable frame");
    Frame *destFrame = producer->requestFrameLocked(); //找可以写入的帧，如果没有找到的话，会阻塞
    ELOGD(TAG_AUDIODECODER, "enqueueFrame, requestFrameLocked end, got free frame, abort:%d", abort);
    int ret = destFrame ? 1 : 0;
    if (abort) {
        ret = -1;
    }
    if (ret <= 0) {
        return ret;
    }

    //拷贝frame数据
    if (destFrame) {
        destFrame->index = 0;
        destFrame->uploaded = 0;
        destFrame->width = 0;
        destFrame->height = 0;
        destFrame->pts = pts;
        destFrame->duration = duration;
        av_frame_move_ref(destFrame->frame, srcFrame);
        producer->enqueueFrame(destFrame); //加入到队列，等待音频播放器消费
        ELOGD(TAG_AUDIODECODER,
              "enqueueFrame, pts:%f, duration:%f",
              destFrame->pts,
              destFrame->duration);
    }

    return ret;
}
