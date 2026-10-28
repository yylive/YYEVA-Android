//
// Created by zengjiale on 2022/4/15.
//
#include "datas.h"
#include "descript.h"
#include "effect.h"
#include <list>
#include "android/log.h"

#include "evaanimeconfig.h"

#define LOG_TAG "EvaAnimeConfig"
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ELOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

EvaAnimeConfig* EvaAnimeConfig::parse(const char* json) {
    if (json == nullptr) return nullptr;

    char g_log_info[200];

    JSON_Value *root_value = json_parse_string(json);
    // 获取根类型 object/array
    JSON_Value_Type jsonValueType = json_value_get_type(root_value);
    if (jsonValueType == JSONObject) {
        auto* config = new EvaAnimeConfig();

        JSON_Object *jsonObject = json_value_get_object(root_value);

        //解析descript
        JSON_Value *descript = json_object_get_value(jsonObject, "descript");
//        sprintf(g_log_info, "%s", descript);
//        ELOGV("descript : %s", g_log_info);

        if (descript != nullptr) {
            JSON_Object *descriptJsonObject = json_value_get_object(descript);
//            JSON_Object *descriptJsonObject = json_object(json_parse_string(descript));
            config->descript = new Descript();
            const double width = json_object_get_number(descriptJsonObject, "width");
            config->descript->width = (int)width;
            config->videoWidth = config->descript->width;
            const double height = json_object_get_number(descriptJsonObject, "height");
            config->descript->height = (int)height;
            config->videoHeight = config->descript->height;
            const bool isEffect = json_object_get_boolean(descriptJsonObject, "isEffect");
            config->descript->isEffect = isEffect;
            const double version = json_object_get_number(descriptJsonObject, "version");
            config->descript->version = (int)version;

            JSON_Array* rgbFrameJsonArray = json_object_get_array(descriptJsonObject, "rgbFrame");
            if (rgbFrameJsonArray != nullptr) {
//                JSON_Array *rgbFrameJsonArray = json_array(json_parse_string(rgbFrame));
                if (json_array_get_count(rgbFrameJsonArray) != 4) { //设定为4个数据
                    ELOGV("rgbFrame size not 4");
                } else {
                    config->descript->rgbFrame = new PointRect((int)json_array_get_number(rgbFrameJsonArray, 0),
                                                               (int)json_array_get_number(rgbFrameJsonArray, 1),
                                                               (int)json_array_get_number(rgbFrameJsonArray, 2),
                                                               (int)json_array_get_number(rgbFrameJsonArray, 3));
                    config->width = config->descript->rgbFrame->w;
                    config->height = config->descript->rgbFrame->h;
                    config->rgbPointRect=config->descript->rgbFrame;
                }
            }

            JSON_Array* alphaFrameJsonArray = json_object_get_array(descriptJsonObject, "alphaFrame");
            if (alphaFrameJsonArray != nullptr) {
//                JSON_Array *alphaFrameJsonArray = json_array(json_parse_string(alphaFrame));
                if (json_array_get_count(alphaFrameJsonArray) != 4) { //设定为4个数据
                    ELOGV("alphaFrame size not 4");
                } else {
                    config->descript->alphaFrame = new PointRect((int)json_array_get_number(alphaFrameJsonArray, 0),
                                                               (int)json_array_get_number(alphaFrameJsonArray, 1),
                                                               (int)json_array_get_number(alphaFrameJsonArray, 2),
                                                               (int)json_array_get_number(alphaFrameJsonArray, 3));
                    config->alphaPointRect=config->descript->alphaFrame;
                }
            }
        }

        //解析descript
        JSON_Value *effectValue = json_object_get_value(jsonObject, "effect");

        //解析effect
        JSON_Array *effectJsonArray = json_value_get_array(effectValue);
//        sprintf(g_log_info, "%s", effectJson);
//        ELOGV("effectJson : %s", g_log_info);

        if (effectJsonArray != nullptr) {
//            JSON_Array *effectJsonArray = json_array(json_parse_string(effectJson));
            for (int i = 0; i < json_array_get_count(effectJsonArray); ++i) {
                JSON_Object *effectTmp = json_array_get_object(effectJsonArray, i);
                auto* effect = new Effect();
                effect->effectWidth = (int)json_object_get_number(effectTmp, "effectWidth");
                effect->effectHeight = (int)json_object_get_number(effectTmp, "effectHeight");
                effect->effectId= (int)json_object_get_number(effectTmp, "effectId");
                effect->effectTag = json_object_get_string(effectTmp, "effectTag");
                const char* effectType = json_object_get_string(effectTmp, "effectType");
                if (effectType != NULL) {
                    effect->effectType = effectType;
                }
                const char* fontColor = json_object_get_string(effectTmp, "fontColor");
                if (fontColor != NULL) {
                    effect->fontColor = fontColor;
                }

                const char* scaleMode = json_object_get_string(effectTmp, "scaleMode");
                if (scaleMode != NULL) {
                    effect->scaleMode = scaleMode;
                } else {
                    effect->scaleMode = "scaleFill";
                }

                const char* textAlign = json_object_get_string(effectTmp, "textAlign");
                if (textAlign != NULL) {
                    effect->textAlign = textAlign;
                } else {
                    effect->textAlign = "center";
                }

                effect->fontSize = (int)json_object_get_number(effectTmp, "fontSize");
                config->effects.push_back(*effect);
            }
            if (json_array_get_count(effectJsonArray) > 0) {
                config->isMix = true;
            }
        }

        // 解析datas
        JSON_Value *datasJson = json_object_get_value(jsonObject, "datas");
        JSON_Array *datasJsonArray = json_value_get_array(datasJson);
//        sprintf(g_log_info, "%s", datasJson);
//        ELOGV("datasJson : %s", g_log_info);

        if (datasJsonArray != nullptr) {
//            JSON_Array *datasJsonArray = json_array(json_parse_string(datasJson));
            for (int i = 0; i < json_array_get_count(datasJsonArray); ++i) {
                JSON_Object *datasTmp = json_array_get_object(datasJsonArray, i);
                auto* datas = new Datas();
                datas->frameIndex = (int)json_object_get_number(datasTmp, "frameIndex");
                // 获取其中一个参数
                JSON_Array *dataJsonArray = json_object_get_array(datasTmp, "data");
//                sprintf(g_log_info, "%s", dataJson);
//                ELOGV("dataJson : %s", g_log_info);
                if (dataJsonArray != nullptr) {
//                    JSON_Array *dataJsonArray = json_array(json_parse_string(dataJson));
                    for (int i = 0; i < json_array_get_count(dataJsonArray); ++i) {
                        JSON_Object *dataTmp = json_array_get_object(dataJsonArray, i);
                        Data* data = new Data();
//                        const char *renderFrame = json_object_get_string(dataTmp, "renderFrame");
                        JSON_Array *renderFrameJsonArray = json_object_get_array(dataTmp, "renderFrame");
                        if (renderFrameJsonArray != nullptr) {
//                            JSON_Array *renderFrameJsonArray = json_array(json_parse_string(renderFrame));
                            if (json_array_get_count(renderFrameJsonArray) != 4) { //设定为4个数据
                                ELOGV("rgbFrame size not 4");
                            } else {
                                data->renderFrame = new PointRect((int)json_array_get_number(renderFrameJsonArray, 0),
                                                                           (int)json_array_get_number(renderFrameJsonArray, 1),
                                                                           (int)json_array_get_number(renderFrameJsonArray, 2),
                                                                           (int)json_array_get_number(renderFrameJsonArray, 3));
                            }
                        }
                        data->effectId= (int)json_object_get_number(dataTmp, "effectId");
//                        const char *outputFrame = json_object_get_string(dataTmp, "outputFrame");
                        JSON_Array *outputFrameJsonArray = json_object_get_array(dataTmp, "outputFrame");
                        if (outputFrameJsonArray != nullptr) {
//                            JSON_Array *outputFrameJsonArray = json_array(json_parse_string(outputFrame));
                            if (json_array_get_count(outputFrameJsonArray) != 4) { //设定为4个数据
                                ELOGV("alphaFrame size not 4");
                            } else {
                                data->outputFrame = new PointRect((int)json_array_get_number(outputFrameJsonArray, 0),
                                                                             (int)json_array_get_number(outputFrameJsonArray, 1),
                                                                             (int)json_array_get_number(outputFrameJsonArray, 2),
                                                                             (int)json_array_get_number(outputFrameJsonArray, 3));
                            }
                        }
                        datas->data.push_back(*data);
                    }
                }
                config->datas.push_back(*datas);
            }
//            json_value_free(descript);
//            json_value_free(effectValue);
//            json_value_free(datasJson);
            json_value_free(root_value);
        }
        return config;
    }
    return nullptr;
}

EvaAnimeConfig* EvaAnimeConfig::defaultConfig(int _videoWidth, int _videoHeight, int defaultVideoMode) {
    auto* config = new EvaAnimeConfig();
    config->videoWidth = _videoWidth;
    config->videoHeight = _videoHeight;
    switch (defaultVideoMode) {
        case 1: // 视频对齐方式 (兼容老版本视频模式) // 视频左右对齐（alpha左\rgb右）
            config->width = _videoWidth / 2;
            config->height = _videoHeight;
            config->alphaPointRect = new PointRect(0, 0, config->width, config->height);
            config->rgbPointRect = new PointRect(config->width, 0, config->width, config->height);
            break;
        case 2:// 视频左右对齐（alpha左\rgb右）
            config->width = _videoWidth;
            config->height = _videoHeight / 2;
            config->alphaPointRect = new PointRect(0, 0, config->width, config->height);
            config->rgbPointRect = new PointRect(0, config->height, config->width, config->height);
            break;
        case 3: // 视频左右对齐（rgb左\alpha右）
            config->width = _videoWidth / 2;
            config->height = _videoHeight;
            config->rgbPointRect = new PointRect(0, 0, config->width, config->height);
            config->alphaPointRect = new PointRect(config->width, 0, config->width, config->height);
            break;
        case 4: // 视频上下对齐（rgb上\alpha下）
            config->width = _videoWidth;
            config->height = _videoHeight / 2;
            config->rgbPointRect = new PointRect(0, 0, config->width, config->height);
            config->alphaPointRect = new PointRect(0, config->height, config->width, config->height);
            break;
        default: // -1为正常的MP4参数
            config->width = _videoWidth;
            config->height = _videoHeight;
            config->rgbPointRect= new PointRect(0, 0, config->width, config->height);
            config->alphaPointRect = new PointRect(config->width, 0, config->width, config->height);
            break;
    }
    return config;
}