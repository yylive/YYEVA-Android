//
// Created by zengjiale on 2022/9/22.
//

#include "optionparams.h"

OptionParams::~OptionParams() {
    mp4Address.clear();
}

OptionParams *OptionParams::parse(const char *json) {
    if (json == nullptr) return this;

    JSON_Value *root_value = json_parse_string(json);
    JSON_Value_Type jsonValueType = json_value_get_type(root_value);
    if (jsonValueType == JSONObject) {
        JSON_Object *jsonObject = json_value_get_object(root_value);
        if (json_object_has_value(jsonObject, "playerId")) {
            playerId = int(json_object_get_number(jsonObject, "playerId"));
        }
        if (json_object_has_value(jsonObject, "frameRate")) {
            fps = int(json_object_get_number(jsonObject, "frameRate"));
        } else {
            fps = 30;
        }
        if (json_object_has_value(jsonObject, "playCount")) {
            playCount = int(json_object_get_number(jsonObject, "playCount"));
        } else {
            playCount = 1;
        }
        if (json_object_has_value(jsonObject, "isEnableH264")) {
            isEnableH264 = int(json_object_get_boolean(jsonObject, "isEnableH264"));
        } else {
            isEnableH264 = true;
        }

        if (json_object_has_value(jsonObject, "isEnableH265")) {
            isEnableH265 = int(json_object_get_boolean(jsonObject, "isEnableH265"));
        } else {
            isEnableH265 = false;
        }
        if (json_object_has_value(jsonObject, "isMute")) {
            isMute = int(json_object_get_boolean(jsonObject, "isMute"));
        } else {
            isMute = true;
        }
        if (json_object_has_value(jsonObject, "usingSurfaceView")) {
            usingSurfaceView = int(json_object_get_boolean(jsonObject, "usingSurfaceView"));
        }
        if (json_object_has_value(jsonObject, "mp4Address")) {
            mp4Address.clear();
            mp4Address = json_object_get_string(jsonObject, "mp4Address");
        } else {
            mp4Address = "";
        }
        if (json_object_has_value(jsonObject, "scaleType")) {
            scaleType = int(json_object_get_number(jsonObject, "scaleType"));
        } else {
            scaleType = 1;
        }
        if (json_object_has_value(jsonObject, "disableAudio")) {
            disableAudio = bool(json_object_get_boolean(jsonObject, "disableAudio"));
        } else {
            disableAudio = true;
        }
    }
    return this;
}
