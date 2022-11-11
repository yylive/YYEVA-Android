//
// Created by zengjiale on 2022/4/15.
//
#include "string"

#ifndef YYEVA_EFFECT_H
#define YYEVA_EFFECT_H

using namespace std;
class Effect {
public:
    int effectWidth;
    int effectHeight;
    int effectId;
    string effectTag;
    string effectType;
    string fontColor;
    string textAlign;
    string scaleMode; //scaleFill ,aspectFit, aspectFill
    int fontSize;

    Effect();
    ~Effect();
};

#endif
