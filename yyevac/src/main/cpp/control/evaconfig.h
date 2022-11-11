//
// Created by zengjiale on 2022/8/26.
//

#ifndef YYEVA_EVACONFIG_H
#define YYEVA_EVACONFIG_H

#include <bean/evaanimeconfig.h>
#include <util/ievafilecontainer.h>
#include <util/evaconstant.h>
#include <android/log.h>
#include <util/cbase64.h>
#include <util/zutil.h>

#define LOG_TAG "EvaConfig"
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ELOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define DEBUGPRINT(tag, msg) __android_log_print(ANDROID_LOG_DEBUG, tag, "%s", msg)

using namespace std;
class EvaConfig {
public:
    EvaAnimeConfig* config;
    bool isParsingConfig = false;

    EvaConfig();
    ~EvaConfig();
    int parseConfig(IEvaFileContainer* fileContainer, int defaultVideoMode, int defaultFps);
    bool parse(IEvaFileContainer* fileContainer, int defaultVideoMode, int defaultFps);
    void defaultConfig(int videoWidth, int videoHeight);
    void DLONGLOG(const char* tag, const char *msg);
};


#endif //YYEVA_EVACONFIG_H
