//
// Created by zengjiale on 2022/4/19.
//
#pragma once
#include "evaframe.h"
#include "list"
#include "datas.h"


class EvaFrameSet {
public:
    int index = 0;
    std::list<shared_ptr<EvaFrame>> list;

    EvaFrameSet();

    EvaFrameSet(std::shared_ptr<Datas> datas);

    ~EvaFrameSet();
};
