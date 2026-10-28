//
// Created by zengjiale on 2022/4/19.
//

#include "evasrcmap.h"

EvaSrcMap::EvaSrcMap() {

}

EvaSrcMap::EvaSrcMap(list<Effect> effects) {
    for (Effect effect: effects) {
        EvaSrc *src = new EvaSrc(&effect);
        if (src->srcType != EvaSrc::UNKNOWN) {
            map[src->srcId] = *src;
        } else {
            src = nullptr;
        }
    }
//    list<Effect>::iterator it;
//    for (it = effects.begin(); it != effects.end(); ++it) {
//        EvaSrc *src = new EvaSrc(it);
//        if (src->srcType != EvaSrc::UNKNOWN) {
//            map[src->srcId] = *src;
//        } else {
//            src = nullptr;
//        }
//    }
}

EvaSrcMap::~EvaSrcMap() {
    map.clear();
}