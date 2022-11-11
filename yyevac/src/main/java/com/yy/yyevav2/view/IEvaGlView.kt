package com.yy.yyevav2.view

import android.graphics.SurfaceTexture
import android.view.Surface
import com.yy.yyevav2.player.IEvaRender

interface IEvaGlView {

    fun getSurface(): Surface?

    fun getSurfaceTexture(): SurfaceTexture?

    fun setEvaRender(render: IEvaRender)
}