package com.yy.yyevav2.player

import android.graphics.SurfaceTexture
import android.view.Surface

interface IEvaRender {

    fun onSurfaceCreated(surface: Surface): Int

    fun getDecodeTexture(): SurfaceTexture?

    fun onSurfaceTextureAvailable(surfaceTexture: SurfaceTexture)

    fun onSurfaceTextureSizeChanged(width: Int, height: Int)

    fun onSurfaceTextureDestroyed()

    fun onFrameAvailable(surfaceTexture: SurfaceTexture?)

    fun touchPoint(x:Int, y: Int)
}