package com.yy.yyeva.file

import android.content.Context
import android.content.SharedPreferences
import com.yy.yyeva.util.ELog
import com.yy.yyeva.util.EvaConstant.VIDEO_MODE_NORMAL_MP4_NONE

object EvaPref {
    val TAG = "EvaPref"

    var sp: SharedPreferences? = null

    fun init(c: Context) {
        if (sp == null) {
            sp = c.getSharedPreferences(TAG, Context.MODE_PRIVATE)
        }
    }

    fun getEvaJson(md5: String?): String? {
        if (sp == null) {
            ELog.e(TAG, "EvaPref not init")
            return null
        }

        if (md5 == null) {
            ELog.e(TAG, "md5$md5 is null")
            return null
        }

        return sp?.getString("yyeva_json_$md5", "")
    }

    fun setEvaJson(fileName: String, md5: String, json: String) {
        ELog.i(TAG, "setEvaJson fileName: $fileName, md5: $md5")
        sp?.edit()?.putString("yyeva_json_$md5", json)?.apply()
    }

    fun setEvaMp4Type(fileName: String, md5: String, type: Int) {
        ELog.i(TAG, "setEvaMp4Type fileName: $fileName, md5: $md5, type: $type")
        sp?.edit()?.putInt("yyeva_mp4_type_$md5", type)?.apply()
    }

    fun getEvaMp4Type(md5: String?): Int {
        if (sp == null) {
            ELog.e(TAG, "EvaPref not init")
            return VIDEO_MODE_NORMAL_MP4_NONE
        }

        if (md5 == null) {
            ELog.e(TAG, "md5$md5 is null")
            return VIDEO_MODE_NORMAL_MP4_NONE
        }

        return sp?.getInt("yyeva_mp4_type_$md5", VIDEO_MODE_NORMAL_MP4_NONE) ?: VIDEO_MODE_NORMAL_MP4_NONE
    }
}