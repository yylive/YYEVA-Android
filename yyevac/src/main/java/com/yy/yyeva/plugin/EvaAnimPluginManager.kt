package com.yy.yyeva.plugin

import android.view.MotionEvent
import com.yy.yyeva.EvaAnimPlayer
import com.yy.yyeva.util.EvaConstant
import com.yy.yyeva.EvaAnimConfig
import com.yy.yyeva.mix.EvaMixAnimPlugin
import com.yy.yyeva.util.ELog

/**
 * 动画插件管理
 */
class EvaAnimPluginManager(val playerEva: EvaAnimPlayer) {
    companion object {
        private const val TAG = "${EvaConstant.TAG}.AnimPluginManager"
        private const val DIFF_TIMES = 4
    }

    private val mixAnimPlugin = EvaMixAnimPlugin(playerEva)

    private val plugins = listOf(mixAnimPlugin)

    // 当前渲染的帧
    private var frameIndex = 0
    // 当前解码的帧
    private var decodeIndex = 0
    // 帧不相同的次数, 连续多次不同则直接使用decodeIndex
    private var frameDiffTimes = 0

    fun getMixAnimPlugin(): EvaMixAnimPlugin? {
        return mixAnimPlugin
    }

    fun onConfigCreate(config: EvaAnimConfig): Int {
        ELog.i(TAG, "onConfigCreateEva")
        plugins.forEach {
            val res = it.onConfigCreate(config)
            if (res != EvaConstant.OK) {
                return res
            }
        }
        return EvaConstant.OK
    }

    fun onRenderCreate() {
        ELog.i(TAG, "onRenderCreate")
        frameIndex = 0
        decodeIndex = 0
        plugins.forEach {
            it.onRenderCreate()
        }
    }

    fun onDecoding(decodeIndex: Int) {
        ELog.i(TAG, "onDecoding decodeIndex=$decodeIndex")
        this.decodeIndex = decodeIndex
        plugins.forEach {
            it.onDecoding(decodeIndex)
        }
    }

    // 开始循环调用
    fun onLoopStart() {
        ELog.i(TAG, "onLoopStart")
        frameIndex = 0
        decodeIndex = 0
    }

    fun onRendering() {
        if (playerEva.configManager.config?.isMix == false) return
        if (decodeIndex > frameIndex + 1 || frameDiffTimes >= DIFF_TIMES) {
            ELog.i(TAG, "jump frameIndex= $frameIndex,decodeIndex=$decodeIndex,frameDiffTimes=$frameDiffTimes")
            frameIndex = decodeIndex
        }
        if (decodeIndex != frameIndex) {
            frameDiffTimes++
        } else {
            frameDiffTimes = 0
        }
        ELog.d(TAG, "onRendering frameIndex=$frameIndex")
        plugins.forEach {
            it.onRendering(frameIndex) // 第一帧 0
        }
        frameIndex++
    }

    fun onRelease() {
        ELog.i(TAG, "onRelease")
        plugins.forEach {
            it.onRelease()
        }
    }

    fun onDestroy() {
        ELog.i(TAG, "onDestroy")
        plugins.forEach {
            it.onDestroy()
        }
    }

    fun onDispatchTouchEvent(ev: MotionEvent): Boolean {
        plugins.forEach {
            if (it.onDispatchTouchEvent(ev)) return true
        }
        return false
    }
}