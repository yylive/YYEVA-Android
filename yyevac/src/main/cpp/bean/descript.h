//
// Created by zengjiale on 2022/4/15.
//

#include "pointrect.h"

#ifndef YYEVA_DESCRIPT_H
#define YYEVA_DESCRIPT_H

class Descript {
public:
    Descript(){};
    ~Descript(){
        rgbFrame = nullptr;
        alphaFrame = nullptr;
    };
    int width;
    int height;
    bool isEffect;
    int version;
    PointRect* rgbFrame;
    PointRect* alphaFrame;
    int fps = 30;
    bool hasAudio;
    bool hasBg;
};


#endif //YYEVA_DESCRIPT_H
