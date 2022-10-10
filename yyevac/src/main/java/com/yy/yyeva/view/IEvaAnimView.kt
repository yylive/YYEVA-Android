package com.yy.yyeva.view

import android.content.res.AssetManager
import android.graphics.SurfaceTexture
import android.view.Surface
import com.yy.yyeva.file.IEvaFileContainer
import com.yy.yyeva.inter.IEvaAnimListener
import com.yy.yyeva.inter.IEvaFetchResource
import com.yy.yyeva.inter.OnEvaResourceClickListener
import com.yy.yyeva.util.IScaleType
import com.yy.yyeva.util.ScaleType
import java.io.File

interface IEvaAnimView {

    fun prepareTextureView()

    fun getSurfaceTexture(): SurfaceTexture?

    fun getSurface(): Surface?

    fun setAnimListener(evaAnimListener: IEvaAnimListener?)

    fun setFetchResource(evaFetchResource: IEvaFetchResource?)

    fun setOnResourceClickListener(evaResourceClickListener: OnEvaResourceClickListener?)
    //循环播放数
    fun setLoop(playLoop: Int)

    fun supportMask(isSupport: Boolean, isEdgeBlur: Boolean)

    fun setFps(fps: Int)

    fun setScaleType(type: ScaleType)

    fun setScaleType(scaleType: IScaleType)

    fun setMute(isMute: Boolean)
    //播放文件
    fun startPlay(file: File)

    //播放本地文件
    fun startPlay(assetManager: AssetManager, assetsPath: String)

    fun startPlay(evaFileContainer: IEvaFileContainer)
    //停止播放
    fun stopPlay()
    //是否正在运行
    fun isRunning(): Boolean

    fun getRealSize(): Pair<Int, Int>

    fun updateTextureViewLayout()
}
