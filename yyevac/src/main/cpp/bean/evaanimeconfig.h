//
// Created by zengjiale on 2022/4/15.
//
#include "datas.h"
#include "descript.h"
#include "effect.h"
#include <list>
#include <util/parson.h>

#ifndef YYEVA_EVAANIMECONFIG_H
#define YYEVA_EVAANIMECONFIG_H

using namespace std;
class EvaAnimeConfig {
public:
    EvaAnimeConfig() {};
    ~EvaAnimeConfig() {
        descript = nullptr;
        effects.clear();
        datas.clear();
    };
    int width = 0;
    int height = 0;
    int videoWidth = 0;
    int videoHeight = 0;
    Descript *descript;
    list<Effect> effects;
    list<Datas> datas;

    PointRect *alphaPointRect;
    PointRect *rgbPointRect;
    bool isMix = false;

    static EvaAnimeConfig* parse(const char* json);
    static EvaAnimeConfig* defaultConfig(int _videoWidth, int _videoHeight, int defaultVideoMode);
};





#endif //YYEVA_EVAANIMECONFIG_H
