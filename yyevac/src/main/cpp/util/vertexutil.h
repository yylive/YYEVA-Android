//
// Created by zengjiale on 2022/4/18.
//
#include "src/main/cpp/bean/pointrect.h"


#ifndef YYEVA_VETEXUTIL_HPP
#define YYEVA_VETEXUTIL_HPP

class VertexUtil {
public:
    static float* create(int width, int height, PointRect* rect, float* array);
    static float switchX(float x);
    static float switchY(float y);
};


#endif //YYEVA_VETEXUTIL_HPP
