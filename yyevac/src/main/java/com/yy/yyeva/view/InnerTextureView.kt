package com.yy.yyeva.view

import android.content.Context
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.TextureView
import com.yy.yyeva.EvaAnimPlayer
import com.yy.yyeva.util.ELog

class InnerTextureView @JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null, defStyleAttr: Int = 0
) : TextureView(context, attrs, defStyleAttr) {
    val TAG = "InnerTextureView"
    var playerEva: EvaAnimPlayer? = null

    init {
        isOpaque = false
    }

    override fun dispatchTouchEvent(ev: MotionEvent?): Boolean {
        val isRunning = playerEva?.isRunning() == true
        val isDispatch = ev != null && (playerEva?.pluginManager?.onDispatchTouchEvent(ev) == true)
        val isHasBg = playerEva?.evaAnimView?.hasBgImage() == true //有背景也是直接拦截掉
        ELog.i(TAG, "isRunning playerEva isRunning：$isRunning, isDispatch : $isDispatch, hasBg: $isHasBg")
        return if (isRunning && !isDispatch && !isHasBg) super.dispatchTouchEvent(ev) else true
    }
}