//
// Created by zengjiale on 2022/4/15.
//
#pragma once
#include "string"

class Effect {
public:
    float effectWidth;
    float effectHeight;
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

