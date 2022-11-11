//
// Created by zengjiale on 2022/6/28.
//

#include "evaconstant.h"

EvaConstant* EvaConstant::instance = nullptr;

EvaConstant::~EvaConstant() {
    if (EvaConstant::instance) {
        delete EvaConstant::instance;
        EvaConstant::instance = nullptr;
    }
}

string EvaConstant::getErrorMsg(const int errorType, string errorMsg) {
    string msg;
    switch (errorType) {
        case REPORT_ERROR_TYPE_EXTRACTOR_EXC:
            msg.append(ERROR_MSG_EXTRACTOR_EXC);
        case REPORT_ERROR_TYPE_DECODE_EXC:
            msg.append(ERROR_MSG_DECODE_EXC);
        case REPORT_ERROR_TYPE_CREATE_THREAD:
            msg.append(ERROR_MSG_CREATE_THREAD);
        case REPORT_ERROR_TYPE_CREATE_RENDER:
            msg.append(ERROR_MSG_CREATE_RENDER);
        case REPORT_ERROR_TYPE_PARSE_CONFIG:
            msg.append(ERROR_MSG_PARSE_CONFIG);
        case REPORT_ERROR_TYPE_CONFIG_PLUGIN_MIX:
            msg.append(ERROR_MSG_CONFIG_PLUGIN_MIX);
        default:
            msg.append("unknown");
    }
    if (!errorMsg.empty()) {
        return msg.append(errorMsg);
    } else {
        return msg;
    }
}

EvaConstant *EvaConstant::getInstance() {
    if(instance == nullptr)
        instance = new EvaConstant();
    return instance;
}
