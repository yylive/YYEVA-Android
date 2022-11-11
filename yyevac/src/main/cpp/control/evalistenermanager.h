//
// Created by zengjiale on 2022/8/29.
//

#ifndef YYEVA_EVALISTENERMANAGER_H
#define YYEVA_EVALISTENERMANAGER_H

#include <bean/evasrc.h>
#include <jni.h>
#include <cstdarg>
#include <list>
#include <android/log.h>

#define LOG_TAG "EvaListenerManager"
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ELOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

using namespace std;

class EvaListenerManager {
public:
    EvaListenerManager(JavaVM *javaVM_) : javaVM(javaVM_) {};

    ~EvaListenerManager();

    bool onVideoConfigReady(list<EvaSrc> &list);

    void onVideoStart();

    void onVideoRender(int frameIndex);

    void onVideoComplete();

    void onVideoDestroy();

    void onFailed(int errorType, string errorMsg);

    void touchCallback(int x, int y, EvaSrc &evaSrcSim);

    void setListener(jobject &_listener) {
        listener = _listener;
    };

private:
    JavaVM *javaVM;
    jobject listener;

    void callVoidMethod(const char *methodName, const char *sign, ...);

    bool attachCurrentThread(JNIEnv *&env, bool &isNeedDetach);

    jobject createEvaSrcSimObj(JNIEnv *env, EvaSrc &evaSrcSim);

};

#endif //YYEVA_EVALISTENERMANAGER_H
