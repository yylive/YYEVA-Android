//
// Created by zengjiale on 2022/8/26.
//

#include "evaconfig.h"

EvaConfig::EvaConfig() {

}

EvaConfig::~EvaConfig() {
    if (config != nullptr) {
        config = nullptr;
    }
}

int EvaConfig::parseConfig(IEvaFileContainer *fileContainer, int defaultVideoMode, int defaultFps) {
    try {
        isParsingConfig = true;
        bool result = parse(fileContainer, defaultVideoMode, defaultFps);
        if (!result) {
            isParsingConfig = false;
            return REPORT_ERROR_TYPE_PARSE_CONFIG;
        }
        if (config != nullptr && config->isDefaultConfig) {
            isParsingConfig = false;
            return REPORT_ERROR_TYPE_PARSE_CONFIG;
        }
//
//        if (config != nullptr && playerEva != nullptr) {
//            int result = playerEva->pluginManager->onConfigCreate(config);
//            isParsingConfig = false;
//            return result;
//        } else {
//            isParsingConfig = false;
//            return EvaConstant::getInstance()->OK;
//        }

        if (config == nullptr) {
            isParsingConfig = false;
            return REPORT_ERROR_TYPE_PARSE_CONFIG;
        } else {
            return EvaConstant::getInstance()->OK;
        }
    } catch (exception e) {
        ELOGE("parseConfig error %s", e.what());
        isParsingConfig = false;
        return REPORT_ERROR_TYPE_PARSE_CONFIG;
    }
}

//元素json写入在mp4 moov metaData，是atom结构体，因为使用ffmpeg封装，所以需要ffmpeg才能简单读取。如果不使用ffmpeg，只能通过遍历文件读取。
bool EvaConfig::parse(IEvaFileContainer *fileContainer, int defaultVideoMode, int defaultFps) {
    config = new EvaAnimeConfig();
    fileContainer->startRandomRead();
    string readBytesLast(sizeof(char)*512, '\0');
    string bufStr(sizeof(char)*512, '\0');
    string bufStrS(sizeof(char)*512, '\0');
    string matchStart = "yyeffectmp4json[[";
    string matchEnd = "]]yyeffectmp4json";
    bool findStart = false;
    bool findEnd = false;
    string jsonStr = "";
    FILE* file = fileContainer->getFile();
    //string &bufStr[0]能够读入'\0', char*无法读入"\0"
    while (fread(&bufStr[0], sizeof(char), sizeof(char)*512, file) > 0) {
        if (!findStart) { //没找到开头
            int index = bufStr.find(matchStart);
            if (index > 0) { //分段1找到匹配开头
                jsonStr = bufStr.substr(index + matchStart.length());
                findStart = true;
                index = jsonStr.find(matchEnd);
                if (index > 0) { //同时包含结尾段进行截取
                    findEnd = true;
                    jsonStr = jsonStr.substr(0, index);
                    break;
                }
            } else {
                if (!readBytesLast.empty()) {
                    bufStrS.clear();
                    bufStrS.append(bufStr);
                    bufStrS.append(readBytesLast);
                    int indexS = bufStrS.find(matchStart);
                    if (indexS > 0) { //合并分段找到匹配开头
                        jsonStr = bufStrS.substr(indexS + matchStart.length());
                        findStart = true;
                        indexS = jsonStr.find(matchEnd);
                        if (indexS > 0) { // 同时包含结尾段进行截取
                            findEnd = true;
                            jsonStr = jsonStr.substr(0, indexS);
                            break;
                        }
                    }
                }
                //保存分段
                readBytesLast.clear();
                readBytesLast.append(bufStr);
            }
        } else {
            int index = bufStr.find(matchEnd);
            if (index > 0) { //分段1找到匹配结尾
                jsonStr += bufStr.substr(0, index);
                findEnd = true;
                break;
            } else {
                if (!readBytesLast.empty()) {
                    bufStrS.clear();
                    bufStrS.append(bufStr);
                    bufStrS.append(readBytesLast);
                    int indexS = bufStrS.find(matchEnd);
                    if (indexS > 0) { //合并分段找到匹配结尾
                        jsonStr = jsonStr.substr(0, jsonStr.length() - (indexS - readBytesLast.size() + 1));
                        findEnd = true;
                        break;
                    }
                }
                //保存数据
                jsonStr += bufStr;
                //保存分段
                readBytesLast.clear();
                readBytesLast.append(bufStr);
            }
        }
    }
    bufStr.clear();
    bufStrS.clear();

    fileContainer->closeRandomRead();
    if (!findStart || ! findEnd) {
        ELOGE("yyeffectmp4json not found");
        if (config != nullptr) {
            config->isDefaultConfig = true;
            config->defaultVideoMode = defaultVideoMode;
            config->fps = defaultFps;
        }
//        playerEva->fps = config->fps;
        return true;
    } else {
        //base64无法先转为string，再传输string给zlib转换，需要转换为vector<char>,不然会出现转换失败
        if (jsonStr.empty()) {
            ELOGE("deocde jsonStr empty");
        } else {
//            DLONGLOG(LOG_TAG,  jsonStr.c_str());
            jsonStr = ZUtil::zUncompress(CBase64::base64_decode(jsonStr));
        }
    }
    if (config != nullptr) {
        try {
//            DLONGLOG(LOG_TAG,  jsonStr.c_str());
            config = config->parse(jsonStr.c_str());
        } catch (exception e) {
            ELOGE("config parse error %s", e.what());
            return false;
        }

        if (defaultFps > 0) {
            config->fps = defaultFps;
        }
    }

    return config->descript != nullptr;
}

void EvaConfig::defaultConfig(int _videoWidth, int _videoHeight) {
    if (config == nullptr) return;
    if (!config->isDefaultConfig) return;
    config->videoWidth = _videoWidth;
    config->videoHeight = _videoHeight;
    config->width = _videoWidth / 2;
    config->height = _videoHeight;
    config->alphaPointRect = new PointRect(0, 0, config->width, config->height);
    if (config->defaultVideoMode == EvaConstant::getInstance()->VIDEO_MODE_SPLIT_HORIZONTAL) {
        // 视频左右对齐（alpha左\rgb右）
        config->rgbPointRect = new PointRect(config->width, 0, config->width, config->height);
    } else if (config->defaultVideoMode == EvaConstant::getInstance()->VIDEO_MODE_SPLIT_VERTICAL) {
        // 视频上下对齐（alpha上\rgb下）
        config->rgbPointRect = new PointRect( 0, config->height, config->width, config->height);
    } else if (config->defaultVideoMode == EvaConstant::getInstance()->VIDEO_MODE_SPLIT_HORIZONTAL_REVERSE) {
        // 视频左右对齐（rgb左\alpha右）
        config->rgbPointRect = new PointRect(config->width, 0, config->width, config->height);
    } else if (config->defaultVideoMode == EvaConstant::getInstance()->VIDEO_MODE_SPLIT_VERTICAL_REVERSE) {
        // 视频上下对齐（rgb上\alpha下）
        config->rgbPointRect = new PointRect( 0, config->height, config->width, config->height);
    } else {
        config->rgbPointRect = new PointRect(config->width, 0, config->width, config->height);
    }
}

void EvaConfig::DLONGLOG(const char* tag, const char *msg) {
    const size_t max_len = 300;
    size_t len = strlen(msg);
    if (len < max_len) {
        DEBUGPRINT(tag, msg);
        return;
    }

    DEBUGPRINT(tag, "==============start of long log===============");
    int lineNo = 0;
    char szLine[max_len];
    size_t prefixLen = 0;
    size_t consumed = 0;
    do {
        memset(szLine, '\0', max_len);
        prefixLen = snprintf(szLine, max_len, "line[%d] --->  ", lineNo);
        strncat(szLine, msg + consumed, max_len - 1 - prefixLen);
        DEBUGPRINT(tag, szLine);

        consumed += strlen(szLine) - prefixLen;
        ++lineNo;
    } while (consumed < len);
    DEBUGPRINT(tag, "==============end of long log===============");
}
