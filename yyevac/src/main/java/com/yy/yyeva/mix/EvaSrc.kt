package com.yy.yyeva.mix

import android.graphics.Bitmap
import android.graphics.Color
import com.yy.yyeva.util.EvaConstant
import com.yy.yyeva.EvaAnimConfig
import com.yy.yyeva.util.ELog
import org.json.JSONObject

class EvaSrc {
    companion object {
        private const val TAG = "${EvaConstant.TAG}.EvaSrc"
    }

    enum class SrcType(val type: String) {
        UNKNOWN("unknown"),
        IMG("img"),
        TXT("txt"),
    }

    enum class LoadType(val type: String) {
        UNKNOWN("unknown"),
        NET("net"), // 网络加载的图片
        LOCAL("local"), // 本地加载的图片
    }

    enum class FitType(val type: String) {
        FIT_XY("fitXY"), // 按原始大小填充纹理
        CENTER_FULL("centerFull"), // 以纹理中心点放置
        CENTER_FIT("centerFit"), // 以纹理中心点显示整个放置
    }

    enum class Style(val style: String) {
        DEFAULT("default"),
        BOLD("b"), // 文字粗体
    }

    var srcId = ""
    var w = 0
    var h = 0
    var srcType = SrcType.UNKNOWN
    var loadType = LoadType.UNKNOWN
    var srcTag = ""
    var scaleMode = "scaleFill"  //scaleFill aspectFit aspectFill
    var bitmap: Bitmap? = null
    var txt = ""
    var style = Style.DEFAULT
    var color: Int = 0
    var fitType = FitType.FIT_XY
    var frontSize = 0
    var textAlign = "center" // center, right, left

    constructor(effect: EvaAnimConfig.Effect) {
        srcId = effect.effectId.toString()
        w = effect.effectWidth
        h = effect.effectHeight
        var colorStr = effect.fontColor
        if (colorStr.isEmpty()) {
            colorStr = "#000000"
        }
        color = Color.parseColor(colorStr)
        srcTag = effect.effectTag
        txt= srcTag
        frontSize = effect.fontSize
        textAlign = effect.textAlign
        srcType = when(effect.effectType) {
            SrcType.IMG.type -> SrcType.IMG
            SrcType.TXT.type -> SrcType.TXT
            else -> SrcType.UNKNOWN
        }
        fitType = FitType.FIT_XY
        ELog.i(TAG, "${toString()} color=$colorStr")
    }

    constructor(json: JSONObject) {
        srcId = json.getString("srcId")
        w = json.getInt("w")
        h = json.getInt("h")
        // 可选
        var colorStr = json.optString("color", "#000000")
        if (colorStr.isEmpty()) {
            colorStr = "#000000"
        }
        color = Color.parseColor(colorStr)
        srcTag = json.getString("srcTag")
        txt = srcTag

        srcType = when(json.getString("srcType")) {
            SrcType.IMG.type -> SrcType.IMG
            SrcType.TXT.type -> SrcType.TXT
            else -> SrcType.UNKNOWN
        }
        loadType = when(json.getString("loadType")) {
            LoadType.NET.type -> LoadType.NET
            LoadType.LOCAL.type -> LoadType.LOCAL
            else -> LoadType.UNKNOWN
        }
        fitType = when(json.getString("fitType")) {
            FitType.CENTER_FULL.type -> FitType.CENTER_FULL
            else -> FitType.FIT_XY
        }

        // 可选
        style = when(json.optString("style", "")) {
            Style.BOLD.style -> Style.BOLD
            else -> Style.DEFAULT
        }
        ELog.i(TAG, "${toString()} color=$colorStr")
    }



    override fun toString(): String {
        return "Src(srcId='$srcId', srcType=$srcType, loadType=$loadType, srcTag='$srcTag', bitmap=$bitmap, txt='$txt, textAlign = $textAlign)"
    }

}

class EvaSrcMap(effects: MutableList<EvaAnimConfig.Effect>) {
    val map = HashMap<String, EvaSrc>()

    init {
        for (i in 0 until effects.size) {
            val src = EvaSrc(effects[i])
            if (src.srcType != EvaSrc.SrcType.UNKNOWN) {
                map[src.srcId] = src
            }
        }
    }
}