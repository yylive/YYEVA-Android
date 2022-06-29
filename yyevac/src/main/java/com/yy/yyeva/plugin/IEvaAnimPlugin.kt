package com.yy.yyeva.plugin

import android.view.MotionEvent
import com.yy.yyeva.util.EvaConstant
import com.yy.yyeva.EvaAnimConfig

interface IEvaAnimPlugin {

    // 配置生成
    fun onConfigCreate(config: EvaAnimConfig): Int {
        return EvaConstant.OK
    }

    // 渲染初始化
    fun onRenderCreate() {}

    // 解码通知
    fun onDecoding(decodeIndex: Int) {}

    // 每一帧渲染
    fun onRendering(frameIndex: Int) {}

    // 每次播放完毕
    fun onRelease() {}

    // 销毁
    fun onDestroy() {}

    /** 触摸事件
     * @return false 不拦截 true 拦截
     */
    fun onDispatchTouchEvent(ev: MotionEvent): Boolean {return false}
}