//
// Created by zengjiale on 2022/4/18.
//
#pragma once
#include <bean/pointrect.h>
#include <memory>

using namespace std;
class VertexUtil {
public:
    static float* create(float width, float height, std::shared_ptr<PointRect> rect, float* array);
    static float switchX(float x);
    static float switchY(float y);
};
