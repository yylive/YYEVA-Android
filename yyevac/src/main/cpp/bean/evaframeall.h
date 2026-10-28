//
// Created by zengjiale on 2022/4/19.
//
#include <map>
#include "list"
#include "evaframeset.h"

#ifndef YYEVA_EVAFRAMEALL_H
#define YYEVA_EVAFRAMEALL_H

using namespace std;
class EvaFrameAll {
public:
    map<int, EvaFrameSet> map;
    EvaFrameAll();

    EvaFrameAll(std::list<Datas> datas);

    ~EvaFrameAll();
};


#endif //YYEVA_EVAFRAMEALL_H
