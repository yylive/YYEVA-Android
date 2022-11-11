package com.yy.yyevav2.view

import android.content.Context
import android.graphics.PixelFormat
import android.graphics.SurfaceTexture
import android.util.AttributeSet
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import com.yy.yyevav2.player.IEvaRender

class EvaSurfaceView @kotlin.jvm.JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null, defStyleAttr: Int = 0
) : SurfaceView(context, attrs, defStyleAttr), IEvaGlView, SurfaceHolder.Callback,
    SurfaceTexture.OnFrameAvailableListener {

    private var surface: Surface? = null
    private var surfaceTexture: SurfaceTexture? = null
    private var render: IEvaRender? = null

    init {
        onInit()
    }

    private fun onInit() {
        setZOrderOnTop(true)
        setBackgroundResource(android.R.color.transparent)
        holder.setFormat(PixelFormat.TRANSLUCENT)
        holder.addCallback(this)
    }

    override fun getSurface(): Surface? = this.surface

    override fun getSurfaceTexture(): SurfaceTexture? = this.surfaceTexture

    override fun setEvaRender(render: IEvaRender) {
        this.render = render
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        this.surface = holder.surface
        this.surface?.let {
            val textureId = render?.onSurfaceCreated(it) ?: 0
            if (textureId > 0) {
                surfaceTexture = SurfaceTexture(textureId)
                surfaceTexture?.let { texture ->
                    render?.onSurfaceTextureAvailable(texture)
                }
            }
        }
        surfaceTexture?.setOnFrameAvailableListener(this)
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        render?.onSurfaceTextureSizeChanged(width, height)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        render?.onSurfaceTextureDestroyed()
        surfaceTexture?.run {
            setOnFrameAvailableListener(null)
            release()
        }
        surfaceTexture = null
        surface = null
    }

    override fun onFrameAvailable(surfaceTexture: SurfaceTexture?) {
        render?.onFrameAvailable(surfaceTexture)
    }
}