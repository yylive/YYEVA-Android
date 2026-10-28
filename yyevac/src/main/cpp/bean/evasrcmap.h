//
// Created by zengjiale on 2022/4/19.
//
#include "map"
#include "string"
#include "list"
#include "evasrc.h"
#include "effect.h"

#ifndef YYEVA_EVASRCMAP_H
#define YYEVA_EVASRCMAP_H

using namespace std;
class EvaSrcMap {
public:
    map<string, EvaSrc> map;
    EvaSrcMap();
    EvaSrcMap(list<Effect> effects);

    ~EvaSrcMap();
};


#endif //YYEVA_EVASRCMAP_H
