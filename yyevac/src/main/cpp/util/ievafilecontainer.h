//
// Created by asus on 2022/6/25.
//

#ifndef YYEVA_IEVAFILECONTAINER_H
#define YYEVA_IEVAFILECONTAINER_H
#include <media/NdkMediaExtractor.h>
#include <fstream>

class IEvaFileContainer {
public:
    virtual void setDataSource(AMediaExtractor* extractor) = 0;

    virtual void startRandomRead() = 0;

    virtual int readBytes(char* b, int off, int len) = 0;

    virtual FILE* getFile() = 0;

    virtual void skip(long pos) = 0;

    virtual void closeRandomRead() = 0;

    virtual void close() = 0;
};


#endif //YYEVA_IEVAFILECONTAINER_H
