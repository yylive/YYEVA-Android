//
// Created by zengjiale on 2022/4/18.
//
#include "src/main/cpp/util/vertexutil.h"
#include "android/log.h"

#define LOG_TAG "VertexUtil"
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

float* VertexUtil::create(int width, int height, PointRect* rect, float *array) {
    if (array == nullptr) {
        ELOGE("array is null");
        return array;
    }
    //x0
    array[0] = switchX((float)rect->x / width);
    //y0
    array[1] = switchY((float)rect->y / height);
    //x1
    array[2] = switchX((float)rect->x / width);
    //y1
    array[3] = switchY(((float)rect->y + rect->h) / height);

    //x2
    array[4] = switchX(((float)rect->x + rect->w) / width);
    //y2
    array[5] = switchY( (float)rect->y / height);
    //x3
    array[6] = switchX(((float)rect->x + rect->w) / width);
    //y3
    array[7] = switchY(((float)rect->y + rect->h) / height);

    return array;
}

float* VertexUtil::createFlip(int width, int height, PointRect* rect, float *array) {
    if (array == nullptr) {
        ELOGE("array is null");
        return array;
    }
    //x1
    array[0] = switchX((float)rect->x / width);
    //y1
    array[1] = switchY(((float)rect->y + rect->h) / height);

    //x0
    array[2] = switchX((float)rect->x / width);
    //y0
    array[3] = switchY((float)rect->y / height);

    //x3
    array[4] = switchX(((float)rect->x + rect->w) / width);
    //y3
    array[5] = switchY(((float)rect->y + rect->h) / height);

    //x2
    array[6] = switchX(((float)rect->x + rect->w) / width);
    //y2
    array[7] = switchY( (float)rect->y / height);

    return array;
}

float* VertexUtil::create(int width, int height, PointRect* rect, float *array,float ratio, float sizeRatio) {

    if (array == nullptr) {
        ELOGE("array is null");
        return array;
    }
    //x0
    array[0] = switchX((float)rect->x / width, ratio) * sizeRatio;
    //y0
    array[1] = switchY((float)rect->y / height)* sizeRatio;
    //x1
    array[2] = switchX((float)rect->x / width, ratio)* sizeRatio;
    //y1
    array[3] = switchY(((float)rect->y + rect->h) / height)* sizeRatio;

    //x2
    array[4] = switchX(((float)rect->x + rect->w) / width, ratio)* sizeRatio;
    //y2
    array[5] = switchY( (float)rect->y / height)* sizeRatio;
    //x3
    array[6] = switchX(((float)rect->x + rect->w) / width, ratio)* sizeRatio;
    //y3
    array[7] = switchY(((float)rect->y + rect->h) / height)* sizeRatio;

    return array;
}


float* VertexUtil::create(int width, int height, PointRect* rect, float *array, float ratio) {
    if (array == nullptr) {
        ELOGE("array is null");
        return array;
    }
    //x0
    array[0] = switchX((float)rect->x / width, ratio);
    //y0
    array[1] = switchY((float)rect->y / height);
    //x1
    array[2] = switchX((float)rect->x / width, ratio);
    //y1
    array[3] = switchY(((float)rect->y + rect->h) / height);

    //x2
    array[4] = switchX(((float)rect->x + rect->w) / width, ratio);
    //y2
    array[5] = switchY( (float)rect->y / height);
    //x3
    array[6] = switchX(((float)rect->x + rect->w) / width, ratio);
    //y3
    array[7] = switchY(((float)rect->y + rect->h) / height);

    return array;
}


//顶点区域为-1~1
float VertexUtil::switchX(float x) {
    return (x * 2 - 1);
}

//顶点区域为-1~1
float VertexUtil::switchX(float x, float ratio) {
    return (x * 2 - 1) * ratio;
}

//顶点区域-1~1
float VertexUtil::switchY(float y) {
    return (((y*2 - 2) * -1) - 1);
}