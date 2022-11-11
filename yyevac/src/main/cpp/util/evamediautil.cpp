//
// Created by asus on 2022/6/25.
//

#include "evamediautil.h"

EvaMediaUtil* EvaMediaUtil::instance = nullptr;

EvaMediaUtil::~EvaMediaUtil() {
    if (EvaMediaUtil::instance) {
        delete EvaMediaUtil::instance;
        EvaMediaUtil::instance = nullptr;
    }
}

AMediaExtractor *EvaMediaUtil::getExtractor(IEvaFileContainer *evaFile) {
    AMediaExtractor* extractor = AMediaExtractor_new();
    evaFile->setDataSource(extractor);
    return extractor;
}

bool EvaMediaUtil::checkIsHevc(AMediaFormat *videoFormat) {
    const char* mime;
    AMediaFormat_getString(videoFormat, AMEDIAFORMAT_KEY_MIME, &mime);
    return false;
}

int EvaMediaUtil::selectVideoTrack(AMediaExtractor *extractor) {
    int numTracks = AMediaExtractor_getTrackCount(extractor);
    string head("video/");
    for (int i = 0; i < numTracks; ++i) {
        AMediaFormat* format = AMediaExtractor_getTrackFormat(extractor, i);
        const char *mime;

        if(!AMediaFormat_getString(format,AMEDIAFORMAT_KEY_MIME, &mime)) {
            ELOGE("selectVideoTrack getMime error");
        }
        if (mime != nullptr) {
            string m (mime);
            if (startWith(m, head)) {
                ELOGV("Extractor selected track %i, %s", i, m.c_str());
                return i;
            }
        }
    }
    return -1;
}

bool EvaMediaUtil::startWith(string src, string start) {
    if (src == "" || start == "" || src.size() < start.size()) {
        return false;
    }

    return src.compare(0, start.size(), start) == 0;
}

int EvaMediaUtil::selectAudioTrack(AMediaExtractor *extractor) {
    int numTracks = AMediaExtractor_getTrackCount(extractor);
    string head("audio/");
    for (int i = 0; i < numTracks; ++i) {
        AMediaFormat* format = AMediaExtractor_getTrackFormat(extractor, i);
        const char*  mime;
        if(!AMediaFormat_getString(format, AMEDIAFORMAT_KEY_MIME, &mime)) {
            ELOGE("selectAudioTrack getMime error %i",i);
            continue;
        }
        if (mime != nullptr) {
            string m (mime);
            if (startWith(m, head)) {
                ELOGV("Extractor selected track %i, %s", i, m.c_str());
                return i;
            }
        }
    }
    return -1;
}

bool EvaMediaUtil::checkSupportCodec(string mimeType) {
    if (!isTypeMapInit) {
        isTypeMapInit = true;
        getSupportType();
    }
    //判断key在map中出现的次数
    return supportTypeMap.count(mimeType) != 0;
}

void EvaMediaUtil::getSupportType() {
    try {

    } catch (exception e) {
      ELOGE("getSupportType error %s",e.what());
    }
}

//EvaMediaUtil *EvaMediaUtil::getInstance() {
//    if (instance == nullptr) {
//        instance = new EvaMediaUtil();
//    }
//    return instance;
//}
