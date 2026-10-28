//
// Created by zengjiale on 2022/4/18.
//
#pragma once
#include "src/main/cpp/bean/pointrect.h"
#include <memory>

class TexCoordsUtil {
public:

    static float* create(float width, float height, std::shared_ptr<PointRect> rect, float* array);

    static float* rotate90(float* array);

};
