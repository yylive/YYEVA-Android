package com.yy.yyevav2.view

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.SurfaceTexture
import android.os.Build
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.Surface
import android.view.TextureView
import com.yy.yyevav2.log.EvaLog
import com.yy.yyevav2.player.IEvaRender

class EvaTextureView @kotlin.jvm.JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null, defStyleAttr: Int = 0
) : TextureView(context, attrs, defStyleAttr), IEvaGlView, TextureView.SurfaceTextureListener,
    SurfaceTexture.OnFrameAvailableListener {

    private val TAG = EvaTextureView::class.java.simpleName

    private var surface: Surface? = null
    private var curSurfaceTexture: SurfaceTexture? = null
    private var render: IEvaRender? = null

    init {
        isOpaque = false
        onInit()
    }

    private fun onInit() {
        surfaceTextureListener = this
    }

    override fun getSurface(): Surface? = this.surface

    override fun setEvaRender(render: IEvaRender) {
        this.render = render
    }

    override fun onSurfaceTextureAvailable(surface: SurfaceTexture, width: Int, height: Int) {
        this.surface = Surface(surface)
        this.surface?.let { s ->
            val textureId = render?.onSurfaceCreated(s) ?: 0
            EvaLog.i(TAG, "conSurfaceCreated $textureId")
            textureId.let {
                if (it > 0) {
                    this.curSurfaceTexture = render?.getDecodeTexture()
                    this.curSurfaceTexture?.let {
                        render?.onSurfaceTextureAvailable(it)
                    }
                    if (render == null || render!!.getDecodeTexture() == null) {
                        EvaLog.e(TAG, "render getDecodeTexture null")
                    }
                    render?.getDecodeTexture()?.setOnFrameAvailableListener(this)
                    render?.onSurfaceTextureSizeChanged(width, height)
                    EvaLog.i(TAG, "onSurfaceTextureAvailable $textureId")
                } else {
                    EvaLog.e(TAG, "textureId $textureId < 0")
                }
            }
//            curSurfaceTexture?.setOnFrameAvailableListener(this)
        }
    }

    override fun onSurfaceTextureSizeChanged(surface: SurfaceTexture, width: Int, height: Int) {
        render?.onSurfaceTextureSizeChanged(width, height)
    }

    override fun onSurfaceTextureDestroyed(surface: SurfaceTexture): Boolean {
        render?.onSurfaceTextureDestroyed()
        curSurfaceTexture?.run {
            setOnFrameAvailableListener(null)
            release()
        }
        curSurfaceTexture = null
        this.surface = null
        return belowKitKat()
    }

    /**
     * fix Error detachFromGLContext crash
     */
    @SuppressLint("ObsoleteSdkInt")
    private fun belowKitKat(): Boolean {
        return Build.VERSION.SDK_INT <= 19
    }

    override fun onSurfaceTextureUpdated(surface: SurfaceTexture) {
    }

    override fun onFrameAvailable(surfaceTexture: SurfaceTexture?) {
        EvaLog.i(TAG, "updateTexImage")
        surfaceTexture?.updateTexImage()
        render?.onFrameAvailable(surfaceTexture)
    }

    override fun dispatchTouchEvent(event: MotionEvent?): Boolean {
        event?.let {
            render?.touchPoint(it.x.toInt(), it.y.toInt())
        }
        return super.dispatchTouchEvent(event)
    }
}