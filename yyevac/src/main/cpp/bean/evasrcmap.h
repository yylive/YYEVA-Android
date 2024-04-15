//
// Created by zengjiale on 2022/4/19.
//
#pragma once
#include "map"
#include "string"
#include "list"
#include "evasrc.h"
#include "effect.h"

using namespace std;
class EvaSrcMap {
public:
    map<string, shared_ptr<EvaSrc>> map;
    EvaSrcMap();
    EvaSrcMap(list<shared_ptr<Effect>> effects);

    ~EvaSrcMap();
};
