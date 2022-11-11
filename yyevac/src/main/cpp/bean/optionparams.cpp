//
// Created by zengjiale on 2022/9/22.
//

#include "optionparams.h"

OptionParams::~OptionParams() {
    mp4Address.clear();
    filterType.clear();
}

OptionParams *OptionParams::parse(const char *json) {
    if (json == nullptr) return this;

    JSON_Value *root_value = json_parse_string(json);
    JSON_Value_Type jsonValueType = json_value_get_type(root_value);
    if (jsonValueType == JSONObject) {
        JSON_Object *jsonObject = json_value_get_object(root_value);
        fps = int(json_object_get_number(jsonObject, "frameRate"));
        playCount = int(json_object_get_number(jsonObject, "playCount"));
        isEnableH264 = int(json_object_get_boolean(jsonObject, "isEnableH264"));
        isEnableH265 = int(json_object_get_boolean(jsonObject, "isEnableH265"));
        isMute = int(json_object_get_boolean(jsonObject, "isMute"));
        usingSurfaceView = json_object_get_boolean(jsonObject, "usingSurfaceView");
        isRemoteService = json_object_get_boolean(jsonObject, "isRemoteService");
        isMainProcess = json_object_get_boolean(jsonObject, "isMainProcess");
        useFbo = json_object_get_boolean(jsonObject, "useFbo");
        mp4Address.clear();
        mp4Address = json_object_get_string(jsonObject, "mp4Address");
        scaleType = int(json_object_get_number(jsonObject, "scaleType"));
        filterType.clear();
        filterType = json_object_get_string(jsonObject, "filterType");
    }

    return this;
}

bool OptionParams::isNeedRender() {
    return !isRemoteService || (isRemoteService && isMainProcess);
}
