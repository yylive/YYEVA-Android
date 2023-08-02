package com.yy.yyeva.mix

import android.graphics.Bitmap
import android.graphics.Typeface

/**
 * 资源描述类
 */
class EvaResource(src: EvaSrc) {
    var id = ""
    var type = EvaSrc.SrcType.UNKNOWN
    var loadType = EvaSrc.LoadType.UNKNOWN
    var tag = ""
    var scaleMode: String = "" // scaleFill, aspectFit, aspectFill
    var bitmap: Bitmap? = null

    var text: String? = null //文字，用户定制
    var textColor = -1          //文字颜色，用户定制
    var fontSize = 0        //文字大小，用户定制
    var textAlign: String? = null    //文字对齐方式，用户定制
    var typeFace: Typeface? = null    //字体样式，用户定制

    init {
        id = src.srcId
        type = src.srcType
        loadType = src.loadType
        tag = src.srcTag
        bitmap = src.bitmap
        if (src.scaleMode.isNotEmpty()) {
            scaleMode = src.scaleMode
        }
    }
}