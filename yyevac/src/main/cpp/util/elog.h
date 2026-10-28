//
// Created by Cangwang on 2024/4/12.
//

#pragma once

#include <jni.h>
#include <string>
#include <cstdarg> // 用于va_list, va_start, va_end等宏
#include <mutex>


using namespace std;
namespace yyeva {
    class ELog {
    public:
        ~ELog();

        void setJavaVM(JavaVM* _javaVm) {
            javaVM = _javaVm;
        };

        void setELog(jobject &_elog) {
            elog = _elog;
        };

        void i(string tagName, char* format, ...);

        void d(string tagName, char* format, ...);

        void e(string tagName, char* format, ...);

        static ELog* get();

        mutex mtx;

    private:
        ELog(): javaVM(nullptr){};
        int BUF_MAX_SIZE = 2048;
        JavaVM *javaVM;
        jobject elog = nullptr;

        void log(string level, string tagName, char* format, va_list args);

        void log(string level, string tagName, string msg);

        bool attachCurrentThread(JNIEnv *&env, bool &isNeedDetach);
    };
    static once_flag s_init_instance_flag;
    static ELog* controller;
}
