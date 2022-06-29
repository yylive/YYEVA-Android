
#include "evaframeall.h"

EvaFrameAll::EvaFrameAll(){

}

EvaFrameAll::EvaFrameAll(std::list<Datas> datas) {
//        list<Datas>::iterator it;
//        for (it = datas.begin(); it != datas.end(); ++it) {
//            EvaFrameSet *frameSet = new EvaFrameSet(*it);
//            map.insert(make_pair(frameSet->index, *frameSet));
//        }

    for (Datas d: datas) {
        EvaFrameSet *frameSet = new EvaFrameSet(d);
        map[frameSet->index] = *frameSet;
    }
}

EvaFrameAll::~EvaFrameAll() {
    map.clear();
}