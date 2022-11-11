//
// Created by zengjiale on 2022/6/28.
//

#ifndef YYEVA_EVACONSTANT_H
#define YYEVA_EVACONSTANT_H


#include <string>

using namespace std;

const int REPORT_ERROR_TYPE_EXTRACTOR_EXC = 10001; // MediaExtractor exception
const int REPORT_ERROR_TYPE_DECODE_EXC = 10002; // MediaCodec exception
const int REPORT_ERROR_TYPE_CREATE_THREAD = 10003; // 线程创建失败
const int REPORT_ERROR_TYPE_CREATE_RENDER = 10004; // render创建失败
const int REPORT_ERROR_TYPE_PARSE_CONFIG = 10005; // 配置解析失败
const int REPORT_ERROR_TYPE_CONFIG_PLUGIN_MIX = 10006; // 融合动画资源获取失败
const int REPORT_ERROR_TYPE_FILE_ERROR = 10007; // 文件无法读取
const int REPORT_ERROR_TYPE_HEVC_NOT_SUPPORT = 10008; // 不支持h265

using namespace std;
class EvaConstant {
private:
    static EvaConstant* instance;
private:
    EvaConstant() {};
    ~EvaConstant();
    EvaConstant(const EvaConstant&);
    EvaConstant& operator=(const EvaConstant&);
public:
    static EvaConstant* getInstance();

    const string TAG = "EvaConstant";

    // 视频对齐方式 (兼容老版本视频模式)

    const int VIDEO_MODE_SPLIT_HORIZONTAL = 1; // 视频左右对齐（alpha左\rgb右）
    const int VIDEO_MODE_SPLIT_VERTICAL = 2; // 视频上下对齐（alpha上\rgb下）
    const int VIDEO_MODE_SPLIT_HORIZONTAL_REVERSE = 3; // 视频左右对齐（rgb左\alpha右）
    const int VIDEO_MODE_SPLIT_VERTICAL_REVERSE = 4; // 视频上下对齐（rgb上\alpha下）

    const int OK = 0; // 成功

    const string ERROR_MSG_EXTRACTOR_EXC = "0x1 MediaExtractor exception"; // MediaExtractor exception
    const string ERROR_MSG_DECODE_EXC = "0x2 MediaCodec exception"; // MediaCodec exception
    const string ERROR_MSG_CREATE_THREAD = "0x3 thread create fail"; // 线程创建失败
    const string ERROR_MSG_CREATE_RENDER = "0x4 render create fail"; // render创建失败
    const string ERROR_MSG_PARSE_CONFIG = "0x5 parse config fail"; // 配置解析失败
    const string ERROR_MSG_CONFIG_PLUGIN_MIX = "0x6 yyeva fail"; // 融合动画资源获取失败
    const string ERROR_MSG_FILE_ERROR = "0x7 file can't read"; // 文件无法读取
    const string ERROR_MSG_HEVC_NOT_SUPPORT = "0x8 hevc not support"; // 不支持h265

    string getErrorMsg(int errorType, string errorMsg);
};

#endif //YYEVA_EVACONSTANT_H
