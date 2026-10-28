//
// Created by zengjiale on 2022/4/15.
//
#include "pointrect.h"
#ifndef YYEVA_DATA_H
#define YYEVA_DATA_H


class Data {
public:
    Data(){};
    ~Data(){
        renderFrame = nullptr;
        outputFrame = nullptr;
    };
    PointRect* renderFrame;
    int effectId;
    PointRect* outputFrame;
};


#endif //YYEVA_DATA_H
