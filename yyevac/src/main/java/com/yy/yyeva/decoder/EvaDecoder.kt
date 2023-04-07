package com.yy.yyeva.decoder

import android.os.Build
import android.os.HandlerThread
import android.os.Handler
import android.util.Log
import com.yy.yyeva.EvaAnimConfig
import com.yy.yyeva.EvaAnimPlayer
import com.yy.yyeva.file.IEvaFileContainer
import com.yy.yyeva.inter.IEvaAnimListener
import com.yy.yyeva.util.EvaConstant
import com.yy.yyeva.util.ELog
import com.yy.yyeva.util.EvaJniUtil
import com.yy.yyeva.util.SpeedControlUtil
import org.json.JSONObject


abstract class Decoder(val playerEva: EvaAnimPlayer) : IEvaAnimListener {

    companion object {
        private const val TAG = "${EvaConstant.TAG}.Decoder"

        fun createThread(handlerHolder: HandlerHolder, name: String): Boolean {
            try {
                if (handlerHolder.thread == null || handlerHolder.thread?.isAlive == false) {
                    handlerHolder.thread = HandlerThread(name).apply {
                        start()
                        handlerHolder.handler = Handler(looper)
                    }
                }
                return true
            } catch (e: OutOfMemoryError) {
                ELog.e(TAG, "createThread OOM", e)
            }
            return false
        }

        fun quitSafely(thread: HandlerThread?): HandlerThread? {
            thread?.apply {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
                    thread.quitSafely()
                } else {
                    thread.quit()
                }
            }
            return null
        }
    }

    //    var render: IRenderListener? = null
    val renderThread = HandlerHolder(null, null)
    val decodeThread = HandlerHolder(null, null)
    private var surfaceWidth = 0
    private var surfaceHeight = 0
    var fps: Int = 0
        set(value) {
            //设置帧率
            speedControlUtil.setFixedPlaybackRate(value)
            field = value
        }
    var playLoop = 0 // 循环播放次数
    var isRunning = false // 是否正在运行
    var isStopReq = false // 是否需要停止
    val speedControlUtil by lazy { SpeedControlUtil() }

    abstract fun start(evaFileContainer: IEvaFileContainer)

    fun stop() {
        Log.i(TAG, "stop true")
        isStopReq = true
    }

    abstract fun pause()

    abstract fun resume()

    abstract fun destroy()

    fun prepareThread(): Boolean {
        return createThread(renderThread, "anim_render_thread") && createThread(decodeThread, "anim_decode_thread")
    }

    fun prepareRender(needYUV: Boolean): Boolean {
        ELog.i(TAG, "prepareRender")
        playerEva.evaAnimView.getSurface()?.apply {
            playerEva.controllerId = EvaJniUtil.initRender(playerEva.controllerId, this, needYUV, playerEva.isNormalMp4)
            return true
        }
        return false
    }

    fun preparePlay(videoWidth: Int, videoHeight: Int) {
        playerEva.configManager.defaultConfig(videoWidth, videoHeight)
        playerEva.configManager.config?.apply {
//            render?.setAnimConfig(this)
            if (isDefaultConfig) {
                if (playerEva.isNormalMp4) {  //正常MP4播放
                    EvaJniUtil.defaultConfig(
                        playerEva.controllerId,
                        videoWidth,
                        videoHeight,
                        -1  //正常宽高
                    )
                } else {
                    EvaJniUtil.defaultConfig(
                        playerEva.controllerId,
                        videoWidth,
                        videoHeight,
                        defaultVideoMode
                    )
                }
                playerEva.evaAnimListener?.onVideoConfigReady(this)
                playerEva.evaAnimView.updateTextureViewLayout()
            } else if (jsonConfig != null) {
                EvaJniUtil.setRenderConfig(playerEva.controllerId, jsonConfig.toString())
            }
        }

        playerEva.pluginManager.onRenderCreate()
    }

    /**
     * decode过程中视频尺寸变化
     * 主要是没有16进制对齐的老视频
     */
    fun videoSizeChange(newWidth: Int, newHeight: Int) {
        EvaJniUtil.videoSizeChange(playerEva.controllerId, newWidth,newHeight)
    }


    fun destroyThread() {
        if (playerEva.isDetachedFromWindow) {
            ELog.i(TAG, "destroyThread")
            renderThread.handler?.removeCallbacksAndMessages(null)
            decodeThread.handler?.removeCallbacksAndMessages(null)
            renderThread.thread = quitSafely(renderThread.thread)
            decodeThread.thread = quitSafely(decodeThread.thread)
            renderThread.handler = null
            decodeThread.handler = null
        }
    }

    fun onSurfaceSizeChanged(width: Int, height: Int) {
        surfaceWidth = width
        surfaceHeight = height
        EvaJniUtil.updateViewPoint(playerEva.controllerId, width,height)
        Log.i(TAG, "updateViewPoint $width, $height")
    }

    override fun onVideoStart() {
        ELog.i(TAG, "onVideoStart")
        playerEva.evaAnimListener?.onVideoStart()
    }

    override fun onVideoRestart() {
        ELog.i(TAG, "onVideoRestart")
        playerEva.evaAnimListener?.onVideoRestart()
    }

    override fun onVideoRender(frameIndex: Int, config: EvaAnimConfig?) {
        ELog.d(TAG, "onVideoRender")
        playerEva.evaAnimListener?.onVideoRender(frameIndex, config)
    }

    override fun onVideoComplete() {
        ELog.i(TAG, "onVideoComplete")
        playerEva.evaAnimListener?.onVideoComplete()
    }

    override fun onVideoDestroy() {
        ELog.i(TAG, "onVideoDestroy")
        playerEva.evaAnimListener?.onVideoDestroy()
    }

    override fun onFailed(errorType: Int, errorMsg: String?) {
        ELog.e(TAG, "onFailed errorType=$errorType, errorMsg=$errorMsg")
        playerEva.evaAnimListener?.onFailed(errorType, errorMsg)
    }
}

data class HandlerHolder(var thread: HandlerThread?, var handler: Handler?)