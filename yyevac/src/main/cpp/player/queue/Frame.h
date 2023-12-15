//
// Created by zengjiale on 2023/12/13.
//

#ifndef YYEVA_FRAME_H
#define YYEVA_FRAME_H

extern "C" {
    #include <libavformat/avformat.h>
};
namespace yyeva {
    struct Frame {
        AVFrame *frame;
        int index;
        int width;
        int height;
        int uploaded; //帧数据是否已经被使用
        double pts; //
        double duration;
    };
}

#endif //YYEVA_FRAME_H
