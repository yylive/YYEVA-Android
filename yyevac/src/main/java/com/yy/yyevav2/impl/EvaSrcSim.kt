package com.yy.yyevav2.impl

class EvaSrcSim {
    var w = 0
    var h = 0
    var bold = false
    var txt = ""
    var color = ""
    var scaleMode = "scaleFill"  //scaleFill ,aspectFit, aspectFill
    var fontSize = 0
    var textAlign = "center"
    var srcTag: String = ""
    var srcId: String = ""
    var srcType: String = "unknown"  //unknown, img, txt
    override fun toString(): String {
        return "EvaSrcSim(w=$w, h=$h, bold=$bold, txt='$txt', color=$color, " +
                "fontSize=$fontSize, textAlign=$textAlign, srcTag='$srcTag', srcId='$srcId', srcType='$srcType')"
    }
}