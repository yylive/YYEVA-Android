//
// Created by zengjiale on 2022/4/19.
//
#pragma once
#include <map>
#include "list"
#include "evaframeset.h"

using namespace std;
class EvaFrameAll {
public:
    map<int, std::shared_ptr<EvaFrameSet>> map;
    EvaFrameAll();

    EvaFrameAll(std::list<std::shared_ptr<Datas>> datas);

    ~EvaFrameAll();
};