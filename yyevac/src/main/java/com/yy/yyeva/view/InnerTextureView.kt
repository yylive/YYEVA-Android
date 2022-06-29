package com.yy.yyeva.view

import android.content.Context
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.TextureView
import com.yy.yyeva.EvaAnimPlayer

class InnerTextureView @JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null, defStyleAttr: Int = 0
) : TextureView(context, attrs, defStyleAttr) {

    var playerEva: EvaAnimPlayer? = null

    init {
        isOpaque = false
    }

    override fun dispatchTouchEvent(ev: MotionEvent?): Boolean {
        val res = playerEva?.isRunning() == true
                && ev != null
                && playerEva?.pluginManager?.onDispatchTouchEvent(ev) == true
        return if (res) super.dispatchTouchEvent(ev) else true
    }
}