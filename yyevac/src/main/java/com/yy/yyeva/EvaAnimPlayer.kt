package com.yy.yyeva

import android.util.Log
import com.yy.yyeva.decoder.Decoder
import com.yy.yyeva.decoder.EvaHardDecoder
import com.yy.yyeva.file.IEvaFileContainer
import com.yy.yyeva.inter.IEvaAnimListener
import com.yy.yyeva.plugin.EvaAnimPluginManager
import com.yy.yyeva.util.EvaConstant
import com.yy.yyeva.util.ELog
import com.yy.yyeva.view.EvaAudioPlayer
import com.yy.yyeva.view.IEvaAnimView

class EvaAnimPlayer(val evaAnimView: IEvaAnimView) {

    companion object {
        private const val TAG = "AnimPlayer"
    }
    @Volatile var controllerId = -1 //底层Native管理器id
    var evaAnimListener: IEvaAnimListener? = null
    var decoder: Decoder? = null
    var evaAudioPlayer: EvaAudioPlayer? = null
    var fps: Int = 0
        set(value) {
            decoder?.fps = value
            field = value
        }
    // 设置默认的fps <= 0 表示以mp4内配置为准 > 0  表示以此设置为准
    var defaultFps: Int = 30
    var isSetFps = false
    var audioSpeed = 1.0f  //音频速度
    var playLoop: Int = 0
        set(value) {
            decoder?.playLoop = value
            evaAudioPlayer?.playLoop = value
            field = value
        }
    var supportMaskBoolean : Boolean = false
    var maskEdgeBlurBoolean : Boolean = false
    // 是否兼容老版本 默认不兼容
    var enableVersion1 : Boolean = false
    // 视频模式
    var videoMode: Int = EvaConstant.VIDEO_MODE_NORMAL_MP4  // 正常mp4
    var isDetachedFromWindow = false
    var isSurfaceAvailable = false
    var startRunnable: Runnable? = null
    var isStartRunning = false // 启动时运行状态
    var isMute = false // 是否静音
    var startPoint = 0L // 开启播放位置
    var sampleTime = 0L // sampleTime实际播放时间

    var isNormalMp4 = false //是否正常的不透明的mp4

    var isSetLastFrame = false //是否停留在最后一帧

//    val configManager = AnimConfigManager(this)
    val configManager = EvaAnimConfigManager(this)
    val pluginManager = EvaAnimPluginManager(this)

    fun onSurfaceTextureDestroyed() {
        isSurfaceAvailable = false
        isStartRunning = false
        startPoint = 0L
        decoder?.destroy()
        evaAudioPlayer?.destroy()
    }

    fun onSurfaceTextureAvailable(width: Int, height: Int) {
        isSurfaceAvailable = true
        startRunnable?.run()
        startRunnable = null
    }


    fun onSurfaceTextureSizeChanged(width: Int, height: Int) {
        decoder?.onSurfaceSizeChanged(width, height)
    }

    fun startPlay(evaFileContainer: IEvaFileContainer) {
        isStartRunning = true
        prepareDecoder()
        if (decoder?.prepareThread() == false) {
            isStartRunning = false
            decoder?.onFailed(EvaConstant.REPORT_ERROR_TYPE_CREATE_THREAD, EvaConstant.ERROR_MSG_CREATE_THREAD)
            decoder?.onVideoComplete()
            return
        }
        // 在线程中解析配置
        decoder?.renderThread?.handler?.post {
            val result = configManager.parseConfig(evaFileContainer, enableVersion1, videoMode, defaultFps)
            if (result != EvaConstant.OK) {
                isStartRunning = false
                decoder?.onFailed(result, EvaConstant.getErrorMsg(result))
                decoder?.onVideoComplete()
                return@post
            }
            ELog.i(TAG, "parse ${configManager.config}")
            val config = configManager.config
            // 如果是默认配置，因为信息不完整onVideoConfigReady不会被调用
            if (config != null && (config.isDefaultConfig || evaAnimListener?.onVideoConfigReady(config) == true)) {
                innerStartPlay(evaFileContainer)
            } else {
                ELog.i(TAG, "onVideoConfigReady return false")
            }
        }
    }

    private fun innerStartPlay(evaFileContainer: IEvaFileContainer) {
        synchronized(EvaAnimPlayer::class.java) {
            if (isSurfaceAvailable) {
                Log.i(TAG, "decoder start")
                isStartRunning = false
                decoder?.start(evaFileContainer)
                if (!isMute) {
                    evaAudioPlayer?.start(evaFileContainer)
                }
            } else {
                 startRunnable = Runnable {
                    innerStartPlay(evaFileContainer)
                 }
                evaAnimView.prepareTextureView()
            }
        }
    }

    fun pause() {
        evaAudioPlayer?.pause()
        decoder?.pause()
    }

    fun resume() {
        evaAudioPlayer?.resume()
        decoder?.resume()
    }

    fun stopPlay() {
        decoder?.stop()
        evaAudioPlayer?.stop()
    }

    fun isRunning(): Boolean {
        ELog.i(TAG, "isStartRunning $isStartRunning, decoderRunning ${decoder?.isRunning?:false}")
        return isStartRunning // 启动过程运行状态
                || (decoder?.isRunning ?: false) // 解码过程运行状态
    }

    private fun prepareDecoder() {
        if (decoder == null) {
            decoder = EvaHardDecoder(this).apply {
                playLoop = this@EvaAnimPlayer.playLoop
                fps = this@EvaAnimPlayer.fps
            }
        }
        if (evaAudioPlayer == null) {
            evaAudioPlayer = EvaAudioPlayer(this).apply {
                playLoop = this@EvaAnimPlayer.playLoop
            }
        }
    }
}