package com.yy.yyeva

import com.yy.yyeva.util.EvaConstant
import com.yy.yyeva.util.ELog
import com.yy.yyeva.util.PointRect
import org.json.JSONArray
import org.json.JSONException
import org.json.JSONObject

/**
 * {
"descript": { //视频的描述信息
"width": 1808,    //输出视频的宽
"height": 1008,   //输出视频的高
"isEffect": 1,    //是否为动态元素视频
"version": 1,     //插件的版本号
"rgbFrame": [0, 0, 900, 1000], //rgb位置信息
"alphaFrame": [900, 0, 450, 500] //alpha位置信息
},
"effect": [{  //动态元素的遮罩描述信息
"effectWidth": 700,  //动态元素宽
"effectHeight": 1049,//动态元素高
"effectId": 1,       //动态元素索引id
"effectTag": "a1",   //动态元素的tag
"effectType": "txt",  ////动态元素类型 有 txt和img 2种
"fontColor":"#ffffff",  //当为txt类型的时候才存在 如果设计侧未指定，由SDK默认给
"fontSize":13,     //当为txt类型的时候才存在 如果设计侧未指定，由SDK默认给
}...],
"datas": [{   //每一帧的动态元素位置信息
"frameIndex": 0,  //帧索引
"data": [{
"renderFrame": [x, y, w, h], //在画布上的位置
"effectId": 1,    //标志是哪个动态元素
"outputFrame": [x, y, w, h] //在视频区域的位置
}]
} ... ]
}
 *
 * yyeffectmp4json里读取出来的基础配置
 */
class EvaAnimConfig {

    companion object {
        private const val TAG = "${EvaConstant.TAG}.EvaAnimConfig"
    }

    var width: Int = 0
    var height: Int = 0
    var videoWidth: Int = 0
    var videoHeight: Int = 0
    var descript: Descript? = null  //视频描述信息
    var effects: MutableList<Effect>? = null   //动态元素的遮罩描述信息
    var datas: MutableList<Datas>? = null  //每一帧的动态元素位置信息

    var isDefaultConfig = false // 没有effectmp4json配置时默认逻辑
    var fps = 0
    var alphaPointRect = PointRect(0, 0 ,0 ,0) // alpha区域
    var rgbPointRect = PointRect(0, 0, 0, 0) // rgb区域
    var defaultVideoMode = EvaConstant.VIDEO_MODE_SPLIT_HORIZONTAL_REVERSE
    var jsonConfig: JSONObject? = null
    var isMix = false


    /**
     * @return 解析是否成功，失败按默认配置走
     */
    fun parse(json: JSONObject): Boolean {
        return try {
            json.optJSONObject("descript")?.apply {
                descript = Descript(
                    width = optInt("width"),
                    height = optInt("height"),
                    isEffect = optBoolean("isEffect"),
                    version = optInt("version"),
                    rgbFrame = getFrame(getJSONArray("rgbFrame")),
                    alphaFrame = getFrame(getJSONArray("alphaFrame")),
                    fps = optInt("fps"),
                    hasAudio = optBoolean("haAudio"),
                    hasBg = optBoolean("hasBg")
                )
                videoWidth = descript!!.width
                videoHeight = descript!!.height
                width = descript!!.rgbFrame.w
                height = descript!!.rgbFrame.h
                alphaPointRect = descript!!.alphaFrame
                rgbPointRect = descript!!.rgbFrame
                fps = descript!!.fps
            }

            json.optJSONArray("effect")?.apply {
                if (length() > 0) {
                    effects = ArrayList()
                    for (i in 0 until length()) {
                        effects?.add(getEffect(getJSONObject(i)))
                    }
                }
            }

            json.optJSONArray("datas")?.apply {
                if (length() > 0) {
                    datas = ArrayList()
                    for (i in 0 until length()) {
                        datas?.add(getDatas(getJSONObject(i)))
                    }
                }
            }
            if (!effects.isNullOrEmpty() && !datas.isNullOrEmpty()) {
                isMix = true
            }
            true
        } catch (e : JSONException) {
            ELog.e(TAG, "json parse fail $e", e)
            isMix = false
            false
        }
    }

    private fun getFrame(array: JSONArray?): PointRect {
        return if (array == null) {
            PointRect(0,0,0,0)
        } else {
            PointRect(array.optDouble(0).toInt(), array.optDouble(1).toInt(), array.optDouble(2).toInt(), array.optDouble(3).toInt())
        }
    }

    private fun getEffect(effect: JSONObject?): Effect {
        return if (effect == null) {
            Effect(0, 0,0, "", "", "","", 0, "center")
        } else {
            Effect(effect.optInt("effectWidth"), effect.optInt("effectHeight"), effect.optInt("effectId"),
                effect.optString("effectTag"), effect.optString("effectType"), effect.optString("scaleMode"),
                effect.optString("fontColor"), effect.optInt("fontSize"), effect.optString("textAlign"))
        }
    }

    private fun getDatas(datas: JSONObject?): Datas {
        return if (datas == null) {
            Datas(0, ArrayList<Data>())
        } else {
            val dataList = ArrayList<Data>()

            datas.getJSONArray("data").apply {
                if (length() > 0) {
                    for (i in 0 until length()) {
                        dataList.add(getData(getJSONObject(i)))
                    }
                }
            }

            Datas(datas.optInt("frameIndex"), dataList)
        }
    }

    private fun getData(data: JSONObject?): Data {
        return if (data == null) {
            Data(PointRect(0, 0, 0, 0), 0, PointRect(0, 0, 0, 0))
        } else {
            Data(getFrame(data.getJSONArray("renderFrame")), data.optInt("effectId"),
                getFrame(data.getJSONArray("outputFrame")))
        }
    }

    override fun toString(): String {
        return "EvaAnimConfig(descript=$descript, effects=$effects, datas=$datas)"
    }

    data class FrameRect(val x: Double, val y: Double, val w: Double, val h: Double) {
        override fun toString(): String {
            return "FrameRect(x=$x, y=$y, w=$w, h=$h)"
        }
    }

    /**
     * "descript": { //视频的描述信息
    "width": 1808,    //输出视频的宽
    "height": 1008,   //输出视频的高
    "isEffect": 1,    //是否为动态元素视频
    "version": 1,     //插件的版本号
    "rgbFrame": [0, 0, 900, 1000], //rgb位置信息
    "alphaFrame": [900, 0, 450, 500] //alpha位置信息
    },
     */
    data class Descript(val width: Int, val height: Int, val isEffect: Boolean, val version: Int,
                        val rgbFrame: PointRect, val alphaFrame: PointRect,
                        val fps: Int, val hasAudio: Boolean,
                        val hasBg: Boolean) {
        override fun toString(): String {
            return "Descript(width=$width, height=$height, isEffect=$isEffect, version=$version, " +
                    "rgbFrame=$rgbFrame, alphaFrame=$alphaFrame, " +
                    "fps=$fps, hasAudio=$hasAudio, hasBg=$hasBg)"
        }
    }

    /**
     * "effect": [{  //动态元素的遮罩描述信息
    "effectWidth": 700,  //动态元素宽
    "effectHeight": 1049,//动态元素高
    "effectId": 1,       //动态元素索引id
    "effectTag": "a1",   //动态元素的tag
    "effectType": "txt",  ////动态元素类型 有 txt和img 2种
    "fontColor":"#ffffff",  //当为txt类型的时候才存在 如果设计侧未指定，由SDK默认给
    "fontSize":13,     //当为txt类型的时候才存在 如果设计侧未指定，由SDK默认给
    "textAlign":"center",     //当为txt类型是时候，文字对齐方式
    }...],
     */
    data class Effect(val effectWidth: Int, val effectHeight: Int, val effectId: Int,
                      val effectTag: String, val effectType: String, val scaleMode: String,
                      val fontColor: String, val fontSize: Int, val textAlign: String) {
        override fun toString(): String {
            return "Effect(effectWidth=$effectWidth, effectHeight=$effectHeight, effectId=$effectId," +
                    " effectTag='$effectTag', effectType='$effectType', scaleMode='$scaleMode'," +
                    " fontColor='$fontColor', fontSize=$fontSize, textAlign=$textAlign)"
        }
    }

    /**
     * "datas": [{   //每一帧的动态元素位置信息
    "frameIndex": 0,  //帧索引
    "data": [{
    "renderFrame": [x, y, w, h], //在画布上的位置
    "effectId": 1,    //标志是哪个动态元素
    "outputFrame": [x, y, w, h] //在视频区域的位置
    }]
    } ... ]
     */
    data class Datas(val frameIndex: Int, val data: MutableList<Data>) {
        override fun toString(): String {
            return "Datas(frameIndex=$frameIndex, data=$data)"
        }
    }

    /**
     * "data": [{
    "renderFrame": [x, y, w, h], //在画布上的位置
    "effectId": 1,    //标志是哪个动态元素
    "outputFrame": [x, y, w, h] //在视频区域的位置
    }]
     */
    data class Data(val renderFrame: PointRect, val effectId: Int, val outputFrame: PointRect) {
        override fun toString(): String {
            return "Data(renderFrame=$renderFrame, effectId=$effectId, outputFrame=$outputFrame)"
        }
    }
}


