//
// Created by zengjiale on 2022/8/22.
//

#include "evafilecontainer.h"

EvaFileContainer::EvaFileContainer(string fileAddress):file(nullptr) {
    if (!fileAddress.empty()) {
        this->fileAddress = fileAddress;
    } else {
        ELOGE("fileAddress %s is empty", fileAddress.c_str());
    }
}

EvaFileContainer::~EvaFileContainer() {
    this->fileAddress.clear();
}

void EvaFileContainer::setDataSource(AMediaExtractor *extractor) {
//    if (!fileAddress.empty()) {
//        media_status_t err = AMediaExtractor_setDataSource(extractor, fileAddress.c_str());
//        if (err != AMEDIA_OK) {
//            ELOGE("Error setting extractor data source, err %d", err);
//        }
//    }
    if (!fileAddress.empty()) {
        FILE* f = fopen(fileAddress.c_str(), "r");
        int fd = fileno(f);
        media_status_t err = AMediaExtractor_setDataSourceFd(extractor, fd, 0, LONG_MAX);
        fclose(f);
        if (err != AMEDIA_OK) {
            ELOGE("Error setting extractor data source, err %d", err);
        }
    }
}

void EvaFileContainer::startRandomRead() {
    file = fopen(fileAddress.c_str(), "rb");
    if (!this->fileAddress.empty() && file == nullptr) {
        ELOGE("%s is not exit", fileAddress.c_str());
    }
}

int EvaFileContainer::readBytes(char *b, int off, int len) {
    if (file != nullptr) {
        return fread(b, sizeof(char), len, file);
    }
    return -1;
}


FILE* EvaFileContainer::getFile() {
    return file;
}

void EvaFileContainer::skip(long pos) {
//    IEvaFileContainer::skip(pos);
}

void EvaFileContainer::closeRandomRead() {
    if (file != nullptr) {
        fclose(file);
        file = nullptr;
    }
}

void EvaFileContainer::close() {
    if (file != nullptr) {
        fclose(file);
        file = nullptr;
    }
}
