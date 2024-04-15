//
// Created by zengjiale on 2022/4/15.
//
#pragma once
#include "pointrect.h"

class Data {
public:
    Data(){};
    ~Data(){
        renderFrame = nullptr;
        outputFrame = nullptr;
    };
    std::shared_ptr<PointRect> renderFrame;
    int effectId;
    std::shared_ptr<PointRect> outputFrame;
};
