//
// Created by zengjiale on 2022/4/15.
//
#include "effect.h"

Effect::Effect(){
    effectTag = "";
    effectType = "";
    fontColor = "";
    scaleMode = "";
}
Effect::~Effect(){
    effectTag.clear();
    effectType.clear();
    fontColor.clear();
    textAlign.clear();
    scaleMode.clear();
}