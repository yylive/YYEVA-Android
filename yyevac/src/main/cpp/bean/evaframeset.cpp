//
// Created by zengjiale on 2022/4/19.
//
#include "evaframeset.h"

EvaFrameSet::EvaFrameSet() {

}

EvaFrameSet::EvaFrameSet(Datas datas) {
    index = datas.frameIndex;
    int i = 0;
    std::list<Data>::iterator it;
    for (it = datas.data.begin(); it != datas.data.end(); ++it) {
        auto *frame = new EvaFrame(i, *it);
        list.push_back(*frame);
        i++;
    }
}

EvaFrameSet::~EvaFrameSet() {
    list.clear();
}