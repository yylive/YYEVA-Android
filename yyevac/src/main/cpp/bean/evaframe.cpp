//
// Created by zengjiale on 2022/4/19.
//
#include "evaframe.h"

EvaFrame::EvaFrame(){

}

EvaFrame::EvaFrame(int index, Data d) {
    srcId = to_string(d.effectId);
    z = index;
    frame = d.renderFrame;
    mFrame = d.outputFrame;
}

EvaFrame::~EvaFrame(){
    srcId.clear();
    frame = nullptr;
    mFrame = nullptr;
}