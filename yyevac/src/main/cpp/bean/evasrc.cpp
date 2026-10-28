//
// Created by zengjiale on 2022/4/19.
//

#include "evasrc.h"

EvaSrc::EvaSrc() {

}

EvaSrc::EvaSrc(Effect* effect) {
    srcId = to_string(effect->effectId);
    w = effect->effectWidth;
    h = effect->effectHeight;
    fontColor = effect->fontColor;
    if (effect->effectType == "img") {
        srcType = SrcType(1);
    } else if (effect->effectType == "txt") {
        srcType = SrcType(2);
    }
    if (effect->scaleMode == "aspectFit") {
        fitType = FitType::CENTER_FIT;
    } else if (effect->scaleMode == "aspectFill") {
        fitType = FitType::CENTER_FULL;
    } else {
        fitType = FitType::FIX_XY;
    }

//    srcId = to_string(effect.effectId);
//    w = effect.effectWidth;
//    h = effect.effectHeight;
//    fontColor = effect.fontColor;
//    if (effect.effectType == "img") {
//        srcType = SrcType(1);
//    } else if (effect.effectType == "txt") {
//        srcType = SrcType(2);
//    }
}

EvaSrc::~EvaSrc() {
    srcTag.clear();
    txt.clear();
    fontColor.clear();
    bitmap = nullptr;
    bitmapInfo = nullptr;
}