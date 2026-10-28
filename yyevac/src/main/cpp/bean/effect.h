//
// Created by zengjiale on 2022/4/15.
//
#include "string"

#ifndef YYEVA_EFFECT_H
#define YYEVA_EFFECT_H

class Effect {
public:
    int effectWidth;
    int effectHeight;
    int effectId;
    std::string effectTag;
    std::string effectType;
    std::string fontColor;
    std::string textAlign;
    std::string scaleMode; //scaleFill ,aspectFit, aspectFill
    int fontSize;

    Effect();
    ~Effect();
};

#endif
