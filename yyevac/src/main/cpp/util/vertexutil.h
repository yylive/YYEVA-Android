//
// Created by zengjiale on 2022/4/18.
//
#include "src/main/cpp/bean/pointrect.h"


#ifndef YYEVA_VETEXUTIL_HPP
#define YYEVA_VETEXUTIL_HPP

class VertexUtil {
public:
    static float* create(int width, int height, PointRect* rect, float* array);
    //fbo翻转渲染
    static float* createFlip(int width, int height, PointRect* rect, float* array);
    static float* create(int width, int height, PointRect* rect, float *array,float ratio, float sizeRatio);
    static float switchX(float x);
    //加入ratio参数，计算图像正常比例
    static float* create(int width, int height, PointRect* rect, float* array, float ratio);
    //加入ratio参数，计算图像正常比例
    static float switchX(float x, float ratio);
    static float switchY(float y);
};


#endif //YYEVA_VETEXUTIL_HPP
