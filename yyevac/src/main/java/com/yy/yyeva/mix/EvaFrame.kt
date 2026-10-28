package com.yy.yyeva.mix

import android.util.SparseArray
import com.yy.yyeva.EvaAnimConfig
import com.yy.yyeva.util.PointRect

/**
 * 单帧描述
 */
class EvaFrame(index: Int, d: EvaAnimConfig.Data) {
    var srcId = ""
    var z = 0
    var frame: PointRect
    var mFrame: PointRect
    var mt = 0 // 遮罩旋转角度v2 版本只支持 0 与 90度

    init {
        srcId = d.effectId.toString()
        z = index
        frame = d.renderFrame
        mFrame = d.outputFrame
    }
}


/**
 * 一帧的集合
 */
class EvaFrameSet(data: EvaAnimConfig.Datas) {
    var index = 0 // 哪一帧
    val list = ArrayList<EvaFrame>()
    init {
        index = data.frameIndex
        for (i in 0 until data.data.size) {
            val frame = EvaFrame(i, data.data[i])
            list.add(frame)
        }
    }
}

/**
 * 所有帧
 */
class EvaFrameAll(datas: MutableList<EvaAnimConfig.Datas>) {
    // 每一帧的集合
    val map = SparseArray<EvaFrameSet>()

    init {
        for (data in datas) {
            val frameSet = EvaFrameSet(data)
            map.put(frameSet.index, frameSet)
        }
    }
}