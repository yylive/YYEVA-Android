package com.yy.yyeva.view

import android.content.Context
import android.graphics.PixelFormat
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.SurfaceView
import com.yy.yyeva.EvaAnimPlayer

class InnerSurfaceView @JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null, defStyleAttr: Int = 0
) : SurfaceView(context, attrs, defStyleAttr) {

    var playerEva: EvaAnimPlayer? = null

    init {
        setBackgroundResource(android.R.color.transparent)
        setZOrderOnTop(true)
        holder.setFormat(PixelFormat.TRANSLUCENT)
    }

    override fun dispatchTouchEvent(ev: MotionEvent?): Boolean {
        val res = playerEva?.isRunning() == true
                && ev != null
                && playerEva?.pluginManager?.onDispatchTouchEvent(ev) == true
        return if (!res) super.dispatchTouchEvent(ev) else true
    }
}