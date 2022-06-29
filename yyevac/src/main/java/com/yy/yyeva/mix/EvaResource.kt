package com.yy.yyeva.mix

import android.graphics.Bitmap

/**
 * 资源描述类
 */
class EvaResource(src: EvaSrc) {
    var id = ""
    var type = EvaSrc.SrcType.UNKNOWN
    var loadType = EvaSrc.LoadType.UNKNOWN
    var tag = ""
    var bitmap: Bitmap? = null

    init {
        id = src.srcId
        type = src.srcType
        loadType = src.loadType
        tag = src.srcTag
        bitmap = src.bitmap
    }
}