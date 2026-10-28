//
// Created by zengjiale on 2022/4/15.
//
#include "list"
#include "data.h"

#ifndef YYEVA_DATAS_H
#define YYEVA_DATAS_H

class Datas {
public:
    int frameIndex;
    std::list<Data> data;
    Datas() {

    }
    ~Datas() {
        data.clear();
    }
};

#endif
