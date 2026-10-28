//
// Created by zengjiale on 2022/4/19.
//
#pragma once
#include "string"
#include "pointrect.h"
#include "data.h"

using namespace std;
class EvaFrame {
public:
    string srcId = "";
    int z = 0;
    shared_ptr<PointRect> frame;  //渲染frame
    shared_ptr<PointRect> mFrame;  //输出帧
    int mt = 0; // 遮罩旋转角度v2 版本只支持 0 与 90度

    EvaFrame();

    EvaFrame(int index, shared_ptr<Data> d);

    ~EvaFrame();
};
