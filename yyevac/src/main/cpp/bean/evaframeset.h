//
// Created by zengjiale on 2022/4/19.
//
#include "evaframe.h"
#include "list"
#include "datas.h"

#ifndef YYEVA_EVAFRAMESET_H
#define YYEVA_EVAFRAMESET_H


class EvaFrameSet {
public:
    int index = 0;
    std::list<EvaFrame> list;

    EvaFrameSet();

    EvaFrameSet(Datas datas);

    ~EvaFrameSet();
};


#endif //YYEVA_EVAFRAMESET_H
