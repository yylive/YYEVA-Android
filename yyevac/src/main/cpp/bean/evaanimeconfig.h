//
// Created by zengjiale on 2022/4/15.
//
#pragma once
#include "datas.h"
#include "descript.h"
#include "effect.h"
#include <list>
#include <util/parson.h>

using namespace std;
class EvaAnimeConfig {
public:
    EvaAnimeConfig() {};
    ~EvaAnimeConfig() {
        descript = nullptr;
        effects.clear();
        datas.clear();
    };
    float width = 0;
    float height = 0;
    float videoWidth = 0;
    float videoHeight = 0;
    shared_ptr<Descript> descript;
    list<shared_ptr<Effect>> effects;
    list<shared_ptr<Datas>> datas;

    shared_ptr<PointRect> alphaPointRect;
    shared_ptr<PointRect> rgbPointRect;
    bool isMix = false;

    static shared_ptr<EvaAnimeConfig> parse(const char* json);
    static shared_ptr<EvaAnimeConfig> defaultConfig(int _videoWidth, int _videoHeight, int defaultVideoMode);
};