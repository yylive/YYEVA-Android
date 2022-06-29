//
// Created by zengjiale on 2022/4/18.
//

#include "texcoordsutil.h"

float* TexCoordsUtil::create(int width, int height, PointRect* rect, float* array) {
    //x0
    array[0] = (float) rect->x / width;
    //y0
    array[1] = (float) rect->y / height;
    //x1
    array[2] = (float) rect->x / width;
    //y1
    array[3] = ((float)rect->y + rect->h) / height;

    //x2
    array[4] = ((float)rect->x + rect->w) / width;
    //y2
    array[5] = (float) rect->y / height;
    //x3
    array[6] = ((float)rect->x + rect->w) / width;
    //y3
    array[7] = ((float)rect->y + rect->h) / height;

    return array;
}

float* TexCoordsUtil::rotate90(float* array) {
    // 0->2 1->0 3->1 2->3
    float tx = array[0];
    float ty = array[1];
    // 1->0
    array[0] = array[2];
    array[1] = array[3];

    // 3->1
    array[2] = array[6];
    array[3] = array[7];

    // 2->3
    array[6] = array[4];
    array[7] = array[5];

    // 0->2
    array[4] = tx;
    array[5] = ty;

    return array;
}
