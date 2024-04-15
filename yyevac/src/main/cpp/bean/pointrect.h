//
// Created by zengjiale on 2022/4/15.
//
#pragma once

#include <util/elog.h>

class PointRect {
public:
    int x;
    int y;
    int w;
    int h;
    PointRect(int x, int y, int w, int h) {
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
    }
};