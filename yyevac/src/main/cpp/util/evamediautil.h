//
// Created by asus on 2022/6/25.
//

#ifndef YYEVA_EVAMEDIAUTIL_H
#define YYEVA_EVAMEDIAUTIL_H

#include <string>
#include <map>
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaExtractor.h>
#include <media/NdkMediaFormat.h>
#include <android/log.h>
#include "ievafilecontainer.h"


#define LOG_TAG "EvaMediaUtil"
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ELOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

using namespace std;
const string MIME_HEVC = "video/hevc";

class EvaMediaUtil {
private:
    static EvaMediaUtil* instance;
    bool isTypeMapInit = false;
    map<string, bool> supportTypeMap;

private:
    EvaMediaUtil() = default;
    ~EvaMediaUtil();
    EvaMediaUtil(const EvaMediaUtil&)=delete;
    EvaMediaUtil& operator=(const EvaMediaUtil&)=delete;
public:
    static EvaMediaUtil* getInstance() {
        if (instance == nullptr)
            instance = new EvaMediaUtil();
        return instance;
    }

    AMediaExtractor* getExtractor(IEvaFileContainer* evaFile);

    /**
    * 是否为h265的视频
    */
    bool checkIsHevc(AMediaFormat* videoFormat);

    int selectVideoTrack(AMediaExtractor* extractor);

    int selectAudioTrack(AMediaExtractor* extractor);

    /**
     * 检查设备解码支持类型
     */
    //Synchronized
    bool checkSupportCodec(string mimeType);

    void getSupportType();

    bool startWith(string src, string start);
};

#endif //YYEVA_EVAMEDIAUTIL_H
