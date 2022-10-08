package com.yy.yyeva.mix

import android.view.MotionEvent
import com.yy.yyeva.util.PointRect

/**
 * 触摸事件拦截
 */
class EvaMixTouch(private val mixAnimPlugin: EvaMixAnimPlugin) {

    fun onTouchEvent(ev: MotionEvent): EvaResource? {
        val (viewWith, viewHeight) = mixAnimPlugin.playerEva.evaAnimView.getRealSize()
        val videoWith = mixAnimPlugin.playerEva.configManager.config?.width ?: return null
        val videoHeight = mixAnimPlugin.playerEva.configManager.config?.height ?: return null

        if (viewWith == 0 || viewHeight == 0) return null

        when(ev.action) {
            MotionEvent.ACTION_DOWN -> {
                val x = ev.x * videoWith / viewWith.toFloat()
                val y = ev.y * videoHeight / viewHeight.toFloat()
                val list = mixAnimPlugin.frameAll?.map?.get(mixAnimPlugin.curFrameIndex)?.list
                list?.forEach {frame ->
                    val src = mixAnimPlugin.srcMap?.map?.get(frame.srcId) ?: return@forEach
                    if (calClick(x.toInt(), y.toInt(), frame.frame)) {
                        return EvaResource(src)
                    }
                }
            }
        }
        return null
    }


    private fun calClick(x: Int, y: Int, frame: PointRect): Boolean {
        return x >= frame.x && x <= (frame.x + frame.w)
                && y >= frame.y && y <= (frame.y + frame.h)
    }
}