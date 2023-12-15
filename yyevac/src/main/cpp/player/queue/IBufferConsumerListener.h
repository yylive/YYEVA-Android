//
// Created by lijinlong03 on 8/24/22.
//

#ifndef YYEVA_FFMPEG_IBUFFERCONSUMERLISTENER_H
#define YYEVA_FFMPEG_IBUFFERCONSUMERLISTENER_H

#include "FrameQueue.h"

namespace yyeva {

    class IBufferConsumerListener {

    public:
        virtual void onFrameAvailable() = 0;
    };
}

#endif //YYEVA_FFMPEG_IBUFFERCONSUMERLISTENER_H
