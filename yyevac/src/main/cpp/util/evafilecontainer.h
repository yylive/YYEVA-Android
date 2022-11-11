//
// Created by zengjiale on 2022/8/22.
//

#ifndef YYEVA_EVAFILECONTAINER_H
#define YYEVA_EVAFILECONTAINER_H

#include "ievafilecontainer.h"
#include <android/log.h>
#include <string>
#include <stdio.h>

#define LOG_TAG "EvaFileContainer"
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ELOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

using namespace std;
class EvaFileContainer: public IEvaFileContainer {
public:
    EvaFileContainer(string fileAddress);

    ~EvaFileContainer();

    void setDataSource(AMediaExtractor* extractor);

    void startRandomRead();

    int readBytes(char* b, int off, int len);

    FILE* getFile();

    void skip(long pos);

    void closeRandomRead();

    void close();

private:
    string fileAddress;
    FILE *file;
};


#endif //YYEVA_EVAFILECONTAINER_H
