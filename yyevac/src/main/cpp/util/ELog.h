//
// Created by lijinlong03 on 8/29/22.
//

#ifndef YYEVA_FFMPEG_ELOG_H
#define YYEVA_FFMPEG_ELOG_H

#define ELOGD(TAG, FORMAT, ...) yyeva::ELog::d(TAG, FORMAT, ##__VA_ARGS__)
#define ELOGI(TAG, FORMAT, ...) yyeva::ELog::i(TAG, FORMAT, ##__VA_ARGS__)
#define ELOGV(TAG, FORMAT, ...) yyeva::ELog::v(TAG, FORMAT, ##__VA_ARGS__)
#define ELOGW(TAG, FORMAT, ...) yyeva::ELog::w(TAG, FORMAT, ##__VA_ARGS__)
#define ELOGE(TAG, FORMAT, ...) yyeva::ELog::e(TAG, FORMAT, ##__VA_ARGS__)

#include <android/log.h>

extern "C" void android_log_write(int level, const char *tag, const char *text); //java层实现

enum {
    verbose = 0, debug, info, warning, error
};

namespace yyeva {

    class ELog {

    public:
        static void d(const char *tag, const char *format, ...);

        static void v(const char *tag, const char *format, ...);

        static void i(const char *tag, const char *format, ...);

        static void w(const char *tag, const char *format, ...);

        static void e(const char *tag, const char *format, ...);

    private:
        static void logWriteImpl(int level, const char *tag, const char *msg);
    };
}

#endif //YYEVA_FFMPEG_ELOG_H
