package com.yy.yyeva.view

import android.content.res.AssetManager
import android.graphics.Bitmap
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

    //设置起始播放位置 毫秒
    //硬解某些机型会有跳帧前几帧解析异常的问题，不建议使用。
    fun setStartPoint(startPoint: Long)

    fun supportMask(isSupport: Boolean, isEdgeBlur: Boolean)

    //设置视频帧数和倍率
    fun setVideoFps(fps: Int, speed: Float = 1.0f) //speed为倍速
    //设置音频倍速
    fun setAudioSpeed(speed: Float = 1.0f)

    fun setScaleType(type: ScaleType)

    fun setScaleType(scaleType: IScaleType)

    fun setMute(isMute: Boolean)
    //播放文件
    fun startPlay(file: File)

    //播放本地文件
    fun startPlay(assetManager: AssetManager, assetsPath: String)

    fun startPlay(evaFileContainer: IEvaFileContainer)
    // 暂停播放
    fun pause()
    // 恢复播放
    fun resume()
    //停止播放
    fun stopPlay()
    //是否正在运行
    fun isRunning(): Boolean

    fun getRealSize(): Pair<Int, Int>

    fun updateTextureViewLayout()
    //设置背景图
    fun setBgImage(bg: Bitmap)

    fun hasBgImage(): Boolean
    //是否不透明度mp4
    fun setNormalMp4(isNormalMp4: Boolean)
    //是否停留在最后一帧
    fun setLastFrame(isSetLastFrame: Boolean)
}
