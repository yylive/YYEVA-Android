//
// Created by zengjiale on 2022/4/15.
//
#pragma once

#include <util/elog.h>

class PointRect {
public:
    float x;
    float y;
    float w;
    float h;
    PointRect(float x, float y, float w, float h) {
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
    }
};