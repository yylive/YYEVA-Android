//
// Created by zengjiale on 2022/4/18.
//

#include "texcoordsutil.h"

float* TexCoordsUtil::create(float width, float height, std::shared_ptr<PointRect> rect, float* array) {
    //x0
    array[0] = rect->x / width;
    //y0
    array[1] = rect->y / height;
    //x1
    array[2] = rect->x / width;
    //y1
    array[3] = (rect->y + rect->h) / height;

    //x2
    array[4] = (rect->x + rect->w) / width;
    //y2
    array[5] = rect->y / height;
    //x3
    array[6] = (rect->x + rect->w) / width;
    //y3
    array[7] = (rect->y + rect->h) / height;

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
