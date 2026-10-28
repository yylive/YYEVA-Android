//
// Created by zengjiale on 2022/4/18.
//
#include "src/main/cpp/util/vertexutil.h"


#define LOG_TAG "VertexUtil"
#define ELOGE(...) yyeva::ELog::get()->e(LOG_TAG, __VA_ARGS__)

float* VertexUtil::create(float width, float height, std::shared_ptr<PointRect> rect, float *array) {
    if (array == nullptr) {
        ELOGE("array is null");
        return array;
    }
    //x0
    array[0] = switchX(rect->x / width);
    //y0
    array[1] = switchY(rect->y / height);
    //x1
    array[2] = switchX(rect->x / width);
    //y1
    array[3] = switchY((rect->y + rect->h) / height);

    //x2
    array[4] = switchX((rect->x + rect->w) / width);
    //y2
    array[5] = switchY( rect->y / height);
    //x3
    array[6] = switchX((rect->x + rect->w) / width);
    //y3
    array[7] = switchY((rect->y + rect->h) / height);

    return array;
}

//顶点区域为-1~1
float VertexUtil::switchX(float x) {
    return x * 2 - 1;
}

//顶点区域-1~1
float VertexUtil::switchY(float y) {
    return ((y*2 - 2) * -1) - 1;
}