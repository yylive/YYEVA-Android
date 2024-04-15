//
// Created by Cangwang on 2024/4/12.
//

#include "elog.h"

//#define LOG_TAG "ELog"
//#define ELOGE(...) yyeva::ELog::get()->e(LOG_TAG, __VA_ARGS__)
//#define ELOGV(...) yyeva::ELog::get()->i(LOG_TAG, __VA_ARGS__)

yyeva::ELog::~ELog() {
    elog = nullptr;
    javaVM = nullptr;
}

yyeva::ELog* yyeva::ELog::get() {
    call_once(s_init_instance_flag, [&] {
        controller = new ELog();
    });
//    if (controller != nullptr) {
//        controller = new ELog();
//    }
    return controller;
}

void yyeva::ELog::i(string tagName, char* format, ...) {
    lock_guard<std::mutex> lock(mtx);
    va_list args;
    va_start(args, format);
    log("i", tagName, format, args);
    va_end(args); // 清理args
}

void yyeva::ELog::d(string tagName, char* format, ...) {
    lock_guard<std::mutex> lock(mtx);
    va_list args;
    va_start(args, format);
    log("d", tagName, format, args);
    va_end(args); // 清理args
}

void yyeva::ELog::e(string tagName, char* format, ...) {
    lock_guard<std::mutex> lock(mtx);
    va_list args;
    va_start(args, format);
    log("e", tagName, format, args);
    va_end(args); // 清理args
}

void yyeva::ELog::log(string level, string tagName, char *format, va_list args) {
    char* buf = new char[BUF_MAX_SIZE];
    int result = vsnprintf(buf, BUF_MAX_SIZE, format, args); // 使用vsnprintf进行格式化
    // 确保字符串以null字符结尾
    if (result >= 0 && result < BUF_MAX_SIZE) {
        buf[result] = '\0'; // 正常情况下设置null字符
    } else {
        buf[BUF_MAX_SIZE - 1] = '\0'; // 发生截断时设置null字符
    }
    log(level, tagName, buf);
    // 释放内存
    delete[] buf;
    buf = nullptr;
}

void yyeva::ELog::log(string level, string tagName, string msg) {
    JNIEnv *env;
    bool isNeedDetach = false;
    if (!attachCurrentThread(env, isNeedDetach)) {
        return;
    }
    jstring jtag = env->NewStringUTF(tagName.c_str());
    jstring jmsg = env->NewStringUTF(msg.c_str());
    jclass clazz_listener = env->GetObjectClass(elog);
    jmethodID methodId = env->GetMethodID(clazz_listener, level.c_str(),
                                          "(Ljava/lang/String;Ljava/lang/String;)V");
    env->CallVoidMethod(elog, methodId, jtag, jmsg);
    env->DeleteLocalRef(jtag);
    env->DeleteLocalRef(jmsg);
    if (isNeedDetach) {
        javaVM->DetachCurrentThread();
    }
    env = nullptr;
}

bool yyeva::ELog::attachCurrentThread(JNIEnv *&env, bool &isNeedDetach) {
    if (!javaVM) {
        return false;
    }
    isNeedDetach = false;
//    ELOGV("EvaListenerManager::attachCurrentThread");
    jint res = javaVM->GetEnv((void **) &env, JNI_VERSION_1_6);
    if (res != JNI_OK) {
        res = javaVM->AttachCurrentThread(&env, nullptr);
        if (JNI_OK != res) {
            return false;
        }
        isNeedDetach = true;
    }
    return true;
}
