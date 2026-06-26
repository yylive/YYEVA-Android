package com.yy.yyeva.util

import kotlin.math.max
import kotlin.math.min

object GrayColorJudge {

    // 低亮度像素直接视为灰/黑，避免暗部压缩噪声干扰
    private const val DARK_VALUE_THRESHOLD = 0.12f
    // 饱和度阈值越小越严格，0.18 对大多数压缩视频帧比较稳
    private const val SATURATION_THRESHOLD = 0.18f
    // 区域内至少多少比例的像素满足灰度条件，避免单点误判
    private const val GRAY_RATIO_THRESHOLD = 0.8f

    fun isGrayRegion(pixels: IntArray): Boolean {
        if (pixels.isEmpty()) return false

        var grayCount = 0
        for (pixel in pixels) {
            if (isGrayPixel(pixel)) {
                grayCount++
            }
        }
        return grayCount.toFloat() / pixels.size >= GRAY_RATIO_THRESHOLD
    }

    fun isGrayPixel(color: Int): Boolean {
        val r = color ushr 16 and 0xff
        val g = color ushr 8 and 0xff
        val b = color and 0xff

        val maxChannel = max(max(r, g), b)
        val minChannel = min(min(r, g), b)

        if (maxChannel == 0) return true

        val value = maxChannel / 255f
        if (value <= DARK_VALUE_THRESHOLD) return true

        val saturation = (maxChannel - minChannel).toFloat() / maxChannel.toFloat()
        return saturation <= SATURATION_THRESHOLD
    }
}
