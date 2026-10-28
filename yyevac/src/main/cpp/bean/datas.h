//
// Created by zengjiale on 2022/4/15.
//
#pragma once

#include "list"
#include "data.h"

class Datas {
public:
    int frameIndex;
    std::list<std::shared_ptr<Data>> data;
    Datas() {

    }
    ~Datas() {
        data.clear();
    }
};

