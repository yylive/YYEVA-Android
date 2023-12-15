//
// Created by zengjiale on 2023/12/15.
//

#include "BaseDecoder.h"

yyeva::BaseDecoder::BaseDecoder(yyeva::EvaPlayerContext *context, AVCodecContext *codecContext,
                                yyeva::AVBufferProducer *producer, int streamIndex):
                                abort(0),
                                frameCount(0),
                                context(context),
                                producer(producer),
                                isPacketResend(false),
                                isDecodeFinished(false),
                                tempPacket(),
                                pendingPacket(),
                                streamIndex(streamIndex) {

}

void yyeva::BaseDecoder::release() {
    ELOGD(TAG_BASEDEOCDER, "release");
    context = NULL;
    producer = NULL;
    frameCount = 0;
    streamIndex = -1;
    isPacketResend = false;
    isDecodeFinished = false;
    av_packet_unref(&pendingPacket);
    av_packet_unref(&tempPacket);
    if (codecContext) {
        avcodec_close(codecContext);
        avcodec_free_context(&codecContext);
    }
}

int yyeva::BaseDecoder::decodeFrame(AVFrame *frame) {
    int ret = AVERROR(EAGAIN);
    AVPacket *packet = &tempPacket;
    for (;;) {
        if (abort) { //中断解码
            ELOGD(TAG_BASEDEOCDER, "decode abort");
            return -1;
        }

        do {
            ret = avcodec_receive_frame(codecContext, frame);
            ELOGD(TAG_BASEDEOCDER, "avcodec_receive_frame, ret:%d", ret);
            if (ret == 0) { //如果立即可以拿到Frame，那就直接结束返回
            } else if (ret == AVERROR_EOF) { //如果已经没有buffer读取了，也直接返回
                ELOGD(TAG_BASEDEOCDER, "end of decode");
                return 0;
            }
            if (ret >= 0) {
                return 1;
            }
        } while (ret != AVERROR(EAGAIN));

        bool isGotPacket;
        ELOGD(TAG_BASEDEOCDER, "start get sending packet, isPacketResend:%s",
              isPacketResend ? "true" : "false");
        if (isPacketResend) {  //先看下是否有待处理的packet
            isPacketResend = false;
            isGotPacket = true;
            av_packet_move_ref(packet, &pendingPacket);
        } else {
            isGotPacket = producer->requestPacketLocked(packet) == 1; //去buffer队列获取待解码的packet，会阻塞，等待可用的packet
        }

        ELOGD(TAG_BASEDEOCDER,
              "get sending packet end，isGotPacket:%s, abort:%d, pos:%ld, size:%ld, "
              "streamIndex:%d, duration:%ld",
              isGotPacket ? "true" : "false", abort, packet->pos, packet->size,
              packet->stream_index, packet->duration);

        if (!abort && isGotPacket) { //获得有效的packet
            ret = avcodec_send_packet(codecContext, packet); //放入解码器解码
            ELOGD(TAG_BASEDEOCDER,
                  "avcodec_send_packet, ret:%d, stream_index:%d, pos:%ld, pts:%ld, dts:%ld, size:%d",
                  ret, packet->stream_index, packet->pos, packet->pts, packet->dts,
                  packet->size);
            if (ret == AVERROR(EAGAIN)) {
                isPacketResend = true;
                av_packet_move_ref(&pendingPacket, packet);
            }
        }
        av_packet_unref(packet);
    }
}
