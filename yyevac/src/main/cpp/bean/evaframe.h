//
// Created by zengjiale on 2022/4/19.
//
#include "string"
#include "pointrect.h"
#include "data.h"


#ifndef YYEVA_EVAFRAME_H
#define YYEVA_EVAFRAME_H

using namespace std;
class EvaFrame {
public:
    string srcId = "";
    int z = 0;
    PointRect* frame;  //渲染frame
    PointRect* mFrame;  //输出帧
    int mt = 0; // 遮罩旋转角度v2 版本只支持 0 与 90度

    EvaFrame();

    EvaFrame(int index, Data d);

    ~EvaFrame();
};


#endif //YYEVA_EVAFRAME_H
