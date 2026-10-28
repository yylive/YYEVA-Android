//
// Created by zengjiale on 2022/4/15.
//
#pragma once
#include "pointrect.h"

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
    std::shared_ptr<PointRect> rgbFrame;
    std::shared_ptr<PointRect> alphaFrame;
};
