
#include "evaframeall.h"

EvaFrameAll::EvaFrameAll(){

}

EvaFrameAll::EvaFrameAll(std::list<std::shared_ptr<Datas>> datas) {
//        list<Datas>::iterator it;
//        for (it = datas.begin(); it != datas.end(); ++it) {
//            EvaFrameSet *frameSet = new EvaFrameSet(*it);
//            map.insert(make_pair(frameSet->index, *frameSet));
//        }

    for (std::shared_ptr<Datas> d: datas) {
        auto frameSet = make_shared<EvaFrameSet>(d);
        map[frameSet->index] = frameSet;
    }
}

EvaFrameAll::~EvaFrameAll() {
    map.clear();
}