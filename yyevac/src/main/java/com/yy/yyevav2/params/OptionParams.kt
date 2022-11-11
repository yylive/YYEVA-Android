package com.yy.yyevav2.params

import org.json.JSONObject

class OptionParams {
    var frameRate = 30
    var playCount = 1
    var isEnableH265 = false
    var isEnableH264 = false
    var isMute = false
    var usingSurfaceView = false
    var isRemoteService = true  //使用多进程
    var isMainProcess = true // 是否主进程
    var useFbo = true
    var mp4Address = ""
    var scaleType = 1   // 1=>裁剪居中， 2=>全屏拉伸  3=》原资源大小
    var filterType = "" //高清算法 hermite lagrange
        set(value) {
            if (value.isNotEmpty()) {
                useFbo = true //一定要使用fbo
            }
            field = value
        }

    fun toJson(): String {
        val json = JSONObject()
        json.put("frameRate", frameRate)
        json.put("playCount", playCount)
        json.put("isEnableH265", isEnableH265)
        json.put("isEnableH264", isEnableH264)
        json.put("isMute", isMute)
        json.put("usingSurfaceView", usingSurfaceView)
        json.put("isRemoteService", isRemoteService)
        json.put("isMainProcess", isMainProcess)
        json.put("useFbo", useFbo)
        json.put("mp4Address", mp4Address)
        json.put("scaleType", scaleType)
        json.put("filterType", filterType)
        return json.toString()
    }
}