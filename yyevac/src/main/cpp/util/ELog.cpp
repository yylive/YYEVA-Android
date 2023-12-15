//
// Created by lijinlong03 on 8/29/22.
//
#include <string>
#include "ELog.h"
#include <android/log.h>

#define ELOG_PRINT(LEVEL)  va_list args;\
va_start(args, format);\
char *msg = new char[1024];\
std::vsnprintf(msg, 1024, format, args);\
va_end(args);\
logWriteImpl(LEVEL, tag, msg);\
delete[] msg;

namespace yyeva {

    void ELog::v(const char *tag, const char *format, ...) {
        ELOG_PRINT(verbose)
    }

    void ELog::d(const char *tag, const char *format, ...) {
        ELOG_PRINT(debug)
    }

    void ELog::e(const char *tag, const char *format, ...) {
        ELOG_PRINT(error)
    }

    void ELog::i(const char *tag, const char *format, ...) {
        ELOG_PRINT(info)
    }

    void ELog::w(const char *tag, const char *format, ...) {
        ELOG_PRINT(warning)
    }

    void ELog::logWriteImpl(int level, const char *tag, const char *msg) {
        android_log_write(level, tag, msg);
    }
}