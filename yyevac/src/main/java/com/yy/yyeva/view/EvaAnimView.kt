package com.yy.yyeva.view

import android.annotation.SuppressLint
import android.content.Context
import android.content.res.AssetManager
import android.graphics.Bitmap
import android.graphics.SurfaceTexture
import android.os.Build
import android.os.Handler
import android.os.Looper
import android.util.AttributeSet
import android.util.Log
import android.view.Surface
import android.view.SurfaceHolder
import android.view.View
import android.widget.FrameLayout
import com.yy.yyeva.EvaAnimConfig
import com.yy.yyeva.EvaAnimPlayer
import com.yy.yyeva.file.EvaAssetsEvaFileContainer
import com.yy.yyeva.file.EvaFileContainer
import com.yy.yyeva.file.EvaPref
import com.yy.yyeva.file.IEvaFileContainer
import com.yy.yyeva.inter.IEvaAnimListener
import com.yy.yyeva.inter.IEvaFetchResource
import com.yy.yyeva.inter.OnEvaResourceClickListener
import com.yy.yyeva.util.*
import java.io.File

open class EvaAnimView @JvmOverloads constructor(context: Context, attrs: AttributeSet? = null, defStyleAttr: Int = 0):
    IEvaAnimView,
    FrameLayout(context, attrs, defStyleAttr),
    SurfaceHolder.Callback, SurfaceTexture.OnFrameAvailableListener {

    companion object {
        val TAG = "${EvaConstant.TAG}.AnimView"
    }
    private lateinit var playerEva: EvaAnimPlayer

    private val uiHandler by lazy { Handler(Looper.getMainLooper()) }
    private var surfaceTexture: SurfaceTexture? = null
    private var surface: Surface? = null
    private var evaAnimListener: IEvaAnimListener? = null
    private var innerSurfaceView: InnerSurfaceView? = null
    private var lastEvaFile: IEvaFileContainer? = null
    private val scaleTypeUtil = ScaleTypeUtil()
    private var bg: Bitmap? = null

    // 代理监听
    private val animProxyListener by lazy {
        object : IEvaAnimListener {

            override fun onVideoConfigReady(config: EvaAnimConfig): Boolean {
                scaleTypeUtil.setVideoSize(config.width, config.height)
                return evaAnimListener?.onVideoConfigReady(config) ?: super.onVideoConfigReady(config)
            }

            override fun onVideoStart() {
                evaAnimListener?.onVideoStart()
            }

            override fun onVideoRestart() {
                evaAnimListener?.onVideoRestart()
            }

            override fun onVideoRender(frameIndex: Int, config: EvaAnimConfig?) {
                evaAnimListener?.onVideoRender(frameIndex, config)
            }

            override fun onVideoComplete(lastFrame: Boolean) {
                if (!lastFrame) {
                    hide()
                }
                evaAnimListener?.onVideoComplete()
            }

            override fun onVideoPlayFinish() {
                evaAnimListener?.onVideoPlayFinish()
            }

            override fun onVideoDestroy() {
                hide()
                evaAnimListener?.onVideoDestroy()
            }

            override fun onFailed(errorType: Int, errorMsg: String?) {
                evaAnimListener?.onFailed(errorType, errorMsg)
            }

        }
    }

    // 保证AnimView已经布局完成才加入TextureView
    private var onSizeChangedCalled = false
    private var needPrepareTextureView = false
    @SuppressLint("LongLogTag")
    private val prepareTextureViewRunnable = Runnable {
        Log.i(TAG, "prepareTextureViewRunnable")
        removeAllViews()
        innerSurfaceView = InnerSurfaceView(context).apply {
            playerEva = this@EvaAnimView.playerEva
//            isOpaque = false
//            surfaceTextureListener = this@EvaAnimViewV2
            layoutParams = scaleTypeUtil.getLayoutParam(this)
        }
        innerSurfaceView?.holder?.addCallback(this)
        addView(innerSurfaceView)
    }

    private val updateTextureLayout = Runnable {
        innerSurfaceView?.let {
            val lp = scaleTypeUtil.getLayoutParam(it)
            it.layoutParams = lp
        }
    }


    init {
        hide()
        playerEva = EvaAnimPlayer(this)
        playerEva.evaAnimListener = animProxyListener
        EvaPref.init(context.applicationContext)
    }

    override fun prepareTextureView() {
        if (onSizeChangedCalled) {
            uiHandler.post(prepareTextureViewRunnable)
        } else {
            ELog.e(TAG, "onSizeChanged not called")
            needPrepareTextureView = true
        }
    }

    override fun getSurfaceTexture(): SurfaceTexture? {
        return surfaceTexture
    }

    @SuppressLint("LongLogTag")
    override fun surfaceCreated(holder: SurfaceHolder) {
        playerEva.decoder?.renderThread?.handler?.post {
            playerEva.controllerId = EvaJniUtil.initRender(playerEva.controllerId,
                holder.surface, false, playerEva.isNormalMp4, playerEva.isVideoRecord)
            val textureId = EvaJniUtil.getExternalTexture(playerEva.controllerId)
            if (textureId < 0) {
                Log.e(TAG, "surfaceCreated init OpenGL ES failed!")
            } else {
                bg?.let {
                    EvaJniUtil.setBgBitmap(playerEva.controllerId, it)
                    it.recycle()
                }
                surfaceTexture = SurfaceTexture(textureId)
                surfaceTexture?.setOnFrameAvailableListener(this)
            }
        }
    }

    override fun onFrameAvailable(surface: SurfaceTexture?) {
        this.surfaceTexture = surface
//        playerEva.onSurfaceTextureAvailable(width, height)
    }


    override fun getSurface(): Surface? {
        return surface?:innerSurfaceView?.holder?.surface
    }

    override fun surfaceChanged(surface: SurfaceHolder, format: Int, width: Int, height: Int) {
        ELog.i(TAG, "onSurfaceTextureSizeChanged $width x $height")
        this.surface = surface.surface
        playerEva.onSurfaceTextureAvailable(width, height)
        playerEva.onSurfaceTextureSizeChanged(width, height)
    }

    override fun surfaceDestroyed(surface: SurfaceHolder) {
        ELog.i(TAG, "onSurfaceTextureDestroyed")
        playerEva.onSurfaceTextureDestroyed()
        uiHandler.post {
//            innerSurfaceView?.surfaceTextureListener = null
            this.surfaceTexture?.setOnFrameAvailableListener(null)
            innerSurfaceView = null
            removeAllViews()
        }
    }

//    override fun onSurfaceTextureAvailable(surface: SurfaceTexture, width: Int, height: Int) {
//        ELog.i(TAG, "onSurfaceTextureAvailable width=$width height=$height")
//        this.surface = surface
//        playerEva.onSurfaceTextureAvailable(width, height)
//    }

    override fun onSizeChanged(w: Int, h: Int, oldw: Int, oldh: Int) {
        super.onSizeChanged(w, h, oldw, oldh)
        ELog.i(TAG, "onSizeChanged w=$w, h=$h")
        scaleTypeUtil.setLayoutSize(w, h)
        onSizeChangedCalled = true
        // 需要保证onSizeChanged被调用
        if (needPrepareTextureView) {
            needPrepareTextureView = false
            prepareTextureView()
        }
    }

    override fun onAttachedToWindow() {
        ELog.i(TAG, "onAttachedToWindow")
        super.onAttachedToWindow()
        playerEva.isDetachedFromWindow = false
        // 自动恢复播放
        if (playerEva.playLoop > 0) {
            lastEvaFile?.apply {
                startPlay(this)
            }
        }
    }

    override fun onDetachedFromWindow() {
        ELog.i(TAG, "onDetachedFromWindow")
        super.onDetachedFromWindow()
        if (belowKitKat()) {
            release()
        }
        playerEva.isDetachedFromWindow = true
        playerEva.onSurfaceTextureDestroyed()
    }


    override fun setAnimListener(evaAnimListener: IEvaAnimListener?) {
        this.evaAnimListener = evaAnimListener
    }

    override fun setFetchResource(evaFetchResource: IEvaFetchResource?) {
        playerEva.pluginManager.getMixAnimPlugin()?.resourceRequestEva = evaFetchResource
    }

    override fun setOnResourceClickListener(evaResourceClickListener: OnEvaResourceClickListener?) {
        playerEva.pluginManager.getMixAnimPlugin()?.evaResourceClickListener = evaResourceClickListener
    }

    /**
     * 兼容方案，优先保证表情显示
     */
    open fun enableAutoTxtColorFill(enable: Boolean) {
        playerEva.pluginManager.getMixAnimPlugin()?.autoTxtColorFill = enable
    }

    override fun setLoop(playLoop: Int) {
        playerEva.playLoop = playLoop
    }

    override fun setLoop(isLoop: Boolean) {
        playerEva.isLoop = isLoop
    }

    override fun setStartPoint(startPoint: Long) {
        playerEva.startPoint = startPoint * 1000
    }

    override fun supportMask(isSupport : Boolean, isEdgeBlur : Boolean) {
        playerEva.supportMaskBoolean = isSupport
        playerEva.maskEdgeBlurBoolean = isEdgeBlur
    }

    override fun setLastFrame(isSetLastFrame: Boolean) {
        playerEva.isSetLastFrame = isSetLastFrame
    }

    @Deprecated("Compatible older version mp4, default false")
    fun enableVersion1(enable: Boolean) {
        playerEva.enableVersion1 = enable
    }

    // 兼容老版本视频模式
    @Deprecated("Compatible older version mp4")
    fun setVideoMode(mode: Int) {
        playerEva.videoMode = mode
    }

    override fun setVideoFps(fps: Int, speed: Float) {
        ELog.i(TAG, "setVideoFps=$fps， speed=$speed")
        playerEva.isSetFps = true
        playerEva.defaultFps = (fps * speed).toInt()
    }

    override fun setNormalMp4(isNormalMp4: Boolean) {
        ELog.i(TAG, "isNormalMp4=$isNormalMp4")
        playerEva.isNormalMp4 = isNormalMp4
    }

    override fun setAudioSpeed(speed: Float) {
        ELog.i(TAG, "setAudioSpeed=$speed")
        playerEva.audioSpeed = speed
    }

    override fun setScaleType(type : ScaleType) {
        scaleTypeUtil.currentScaleType = type
    }

    override fun setScaleType(scaleType: IScaleType) {
        scaleTypeUtil.scaleTypeImpl = scaleType
    }

    /**
     * @param isMute true 静音
     */
    override fun setMute(isMute: Boolean) {
        ELog.e(TAG, "set mute=$isMute")
        playerEva.isMute = isMute
    }

    /**
     * 开启录制
     */
    fun setVideoRecord(isVideoRecord: Boolean) {
        ELog.i(TAG, "setVideoRecord=$isVideoRecord")
        playerEva.isVideoRecord = isVideoRecord
    }

    private fun play(videoInfo: EvaVideoEntity) {
        // 播放前强烈建议检查文件的md5是否有改变
        // 因为下载或文件存储过程中会出现文件损坏，导致无法播放
        val file = videoInfo.mCacheDir
        ELog.i(TAG, "play file address ${file.absolutePath}")
        if (!file.exists()) {
            ELog.e(TAG, "${file.absolutePath} is not exist")
        }
        startPlay(file)
    }

    override fun startPlay(file: File) {
        try {
            val fileContainer = EvaFileContainer(file)
            startPlay(fileContainer)
        } catch (e: Throwable) {
            animProxyListener.onFailed(EvaConstant.REPORT_ERROR_TYPE_FILE_ERROR, EvaConstant.ERROR_MSG_FILE_ERROR)
            animProxyListener.onVideoComplete()
        }
    }

    override fun startPlay(assetManager: AssetManager, assetsPath: String) {
        try {
            val fileContainer = EvaAssetsEvaFileContainer(assetManager, assetsPath)
            startPlay(fileContainer)
        } catch (e: Throwable) {
            animProxyListener.onFailed(EvaConstant.REPORT_ERROR_TYPE_FILE_ERROR, EvaConstant.ERROR_MSG_FILE_ERROR)
            animProxyListener.onVideoComplete()
        }
    }


    override fun startPlay(evaFileContainer: IEvaFileContainer) {
        ui {
            if (visibility != View.VISIBLE) {
                ELog.e(TAG, "AnimView is GONE, can't play")
                return@ui
            }
            if (!playerEva.isRunning()) {
                lastEvaFile = evaFileContainer
                playerEva.startPlay(evaFileContainer)
            } else {
                ELog.e(TAG, "is running can not start")
            }
        }
    }

    override fun pause() {
        playerEva.pause()
    }

    override fun resume() {
        playerEva.resume()
    }

    override fun stopPlay() {
        playerEva.stopPlay()
    }

    override fun isRunning(): Boolean {
        return playerEva.isRunning()
    }

    override fun getRealSize(): Pair<Int, Int> {
        return scaleTypeUtil.getRealSize()
    }

    private fun hide() {
        lastEvaFile?.close()
        ui {
            removeAllViews()
        }
    }

    private fun ui(f:()->Unit) {
        if (Looper.myLooper() == Looper.getMainLooper()) f() else uiHandler.post { f() }
    }

    /**
     * fix Error detachFromGLContext crash
     */
    private fun belowKitKat(): Boolean {
        return Build.VERSION.SDK_INT <= 19
    }

    private fun release() {
        try {
            surfaceTexture?.release()
        } catch (error: Throwable) {
            ELog.e(TAG, "failed to release mSurfaceTexture= $surfaceTexture: ${error.message}", error)
        }
        surfaceTexture = null
        bg = null
    }

    override fun updateTextureViewLayout() {
        uiHandler.post(updateTextureLayout)
    }

    override fun setBgImage(bg: Bitmap) {
        this.bg = bg
    }

    override fun hasBgImage(): Boolean {
        return bg != null
    }

    override fun setLog(log: IELog) {
        ELog.log = log
        EvaJniUtil.setLog(log)
    }
}