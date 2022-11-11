package com.yy.yyevav2.player

import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.content.ServiceConnection
import android.graphics.Bitmap
import android.graphics.SurfaceTexture
import android.os.Build
import android.os.Handler
import android.os.IBinder
import android.os.Looper
import android.view.Surface
import android.view.View
import com.yy.yyevav2.impl.IEvaAnimListener
import com.yy.yyevav2.impl.IEvaRemoteListener
import com.yy.yyevav2.log.EvaLog
import com.yy.yyevav2.params.EffectParams
import com.yy.yyevav2.params.OptionParams
import com.yy.yyevav2.params.SourceParams
import com.yy.yyevav2.service.IRemoteCallback
import com.yy.yyevav2.service.IServiceInterface
import com.yy.yyevav2.service.RemoteDecodeService
import com.yy.yyevav2.util.EvaJniUtil
import com.yy.yyevav2.view.EvaSurfaceView
import com.yy.yyevav2.view.EvaTextureView
import com.yy.yyevav2.view.IEvaGlView
import java.lang.Exception

private const val TAG = "EvaPlayerImpl"

class EvaPlayerImpl(private val context: Context, private val params: OptionParams) : IEvaPlayer,
    IEvaRender {

    private val playerTag = "EvaPlayer_${this.hashCode()}"
    private val BG_TAG = "bg_effect"
    private var nativePlayer = 0
    private var surface: SurfaceTexture? = null
    private var mGlView: IEvaGlView? = null
    private val handler = Handler(Looper.getMainLooper())
    private var isPlayed = false
    private var isSurfaceAvailable = false
    private var param: String = ""
    private var hasCreate = false
    private var dt: SurfaceTexture? = null
    private var effectParams = EffectParams()
    private var bg: Bitmap? = null

    private var remoteService: IServiceInterface? = null
    private var serviceConnection: ServiceConnection? = null
    private var remoteListener: IEvaRemoteListener? = null
    private var evaAnimListener: IEvaAnimListener? = null

    init {
        onInit()
        EvaLog.i(TAG, "init ,player:$playerTag")
    }

    private var mDeathRecipient = object: IBinder.DeathRecipient {
        override fun binderDied() {
            if (remoteService == null) {
                return
            }
            remoteService = null
            val intent = Intent(context, RemoteDecodeService::class.java)
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                context.startForegroundService(intent)
            } else {
                context.startService(intent)
            }
            context.bindService(intent, serviceConnection!!, Context.BIND_AUTO_CREATE)
        }
    }

    private fun onInit() {
        if (params.isRemoteService) {
            if (serviceConnection == null) {
                serviceConnection = object : ServiceConnection {
                    override fun onServiceConnected(name: ComponentName?, service: IBinder?) {
                        remoteService = IServiceInterface.Stub.asInterface(service)
                        try {
                            service?.linkToDeath(mDeathRecipient, 0)
                        } catch (e: Exception) {
                            EvaLog.e(TAG, e.toString())
                        }
                        EvaJniUtil.setRemoteService(remoteService!!)
                        mGlView = if (params.usingSurfaceView) {
                            EvaSurfaceView(context)
                        } else {
                            EvaTextureView(context)
                        }
                        mGlView?.setEvaRender(this@EvaPlayerImpl)

                        nativePlayer = EvaJniUtil.createPlayer(playerTag, params)
                        remoteListener?.initCallback()
                        evaAnimListener?.let {
                            EvaJniUtil.setEvaAnimListener(it)
                        }
                        remoteService!!.registerCallback(object : IRemoteCallback.Stub() {
                            override fun onVideoRender(frameIndex: Int) {

                            }

                            override fun onVideoComplete() {
                                evaAnimListener?.onVideoComplete()
                            }

                            override fun onVideoDestroy() {
                                evaAnimListener?.onVideoDestroy()
                            }

                            override fun onFailed(errorType: Int, errorMsg: String?) {
                                evaAnimListener?.onFailed(errorType, errorMsg)
                            }
                        })
                    }

                    override fun onServiceDisconnected(name: ComponentName?) {
                        remoteService?.remoteRelease(nativePlayer)
                    }
                }

//            val intent = Intent()
//            intent.setClassName("com.yy.yyevav2.service", "com.yy.yyevav2.service.RemoteRenderService")
                val intent = Intent(context, RemoteDecodeService::class.java)
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                    context.startForegroundService(intent)
                } else {
                    context.startService(intent)
                }
                context.bindService(intent, serviceConnection!!, Context.BIND_AUTO_CREATE)
            } else {
                evaAnimListener?.let {
                    EvaJniUtil.setEvaAnimListener(it)
                }
            }
        } else {
            mGlView = if (params.usingSurfaceView) {
                EvaSurfaceView(context)
            } else {
                EvaTextureView(context)
            }
            mGlView?.setEvaRender(this)

            nativePlayer = EvaJniUtil.createPlayer(playerTag, params)
        }
    }

    override fun play() {
        if (isPlayed) return
        evaAnimListener?.let {
            EvaJniUtil.setEvaAnimListener(it)
        }
        val code = EvaJniUtil.play(params)
        EvaLog.i(TAG, "play, code:$code")
        if (code == 0) {
            isPlayed = true
        }
    }

    override fun setBgImage(bg: Bitmap) {
        EvaJniUtil.addEffect(BG_TAG, bg, "scaleFill")
    }

    override fun addEffect(name: String, bitmap: Bitmap, scaleMode: String) {
        if (nativePlayer <= 0 && name == BG_TAG) {
            this.bg = bitmap
            return
        }
        if (effectParams.map.containsKey(name)) {
            EvaLog.e(TAG, "Please check same effect name")
        }

        effectParams.map[name] = bitmap
        EvaJniUtil.addEffect(name, bitmap, scaleMode)
        bitmap.recycle()
    }

    override fun setEffectParams(params: EffectParams) {
        if (nativePlayer > 0L) {
            EvaJniUtil.setEffectParams(params.toString())
        }
    }

    override fun setSourceParams(params: SourceParams) {
//        if (nativePlayer > 0L) {
//            nativeSetSourceParams(nativePlayer, params.toParams())
//        }
        EvaLog.i(TAG, "setSourceParams ${params.toParams()}")
        this.param = params.toParams()
        EvaJniUtil.setSourceParams(params.toParams())
    }

    override fun stop() {
        if (isPlayed
            && nativePlayer > 0L
        ) {
            EvaJniUtil.stop(nativePlayer)
        }
        isPlayed = false
    }

    override fun release() {
        EvaLog.i(TAG, "release")
        isPlayed = false
        isSurfaceAvailable = false
        effectParams.map.clear()
        handler.removeCallbacksAndMessages(null)
        surface?.release()
        surface = null
        evaAnimListener = null
    }

    override fun isRunning(): Boolean {
        return nativePlayer > 0L && EvaJniUtil.isRunning(nativePlayer)
    }

    override fun setMute(isMute: Boolean) {
    }

    override fun setLoop(count: Int) {
        params.playCount = count
    }

    override fun getRenderView(): View? = mGlView as? View

    override fun getPlayerTag(): String = playerTag

    override fun onSurfaceCreated(surface: Surface): Int {
//        if (nativePlayer > 0) {
//            hasCreate = true
//            return nativeSetSurface(nativePlayer, surface)
//        }
        EvaLog.i(TAG, "onSurfaceCreated")
        nativePlayer = EvaJniUtil.setSurface(nativePlayer, surface)
        bg?.let {
            EvaJniUtil.addEffect(BG_TAG, it,"scaleFill")
            bg?.recycle()
            bg = null
        }
        dt = SurfaceTexture(nativePlayer)
        EvaJniUtil.setDecodeSurface(nativePlayer, Surface(dt))
        //一定要等egl初始化完成后再去设置背景，因为egl生命周期问题，会导致背景textureId会拿到上一个egl window的资源
        return nativePlayer
    }

    override fun getDecodeTexture(): SurfaceTexture? {
        return dt
    }

    override fun onSurfaceTextureAvailable(surfaceTexture: SurfaceTexture) {
        isSurfaceAvailable = true
        surface = surfaceTexture
//        nativeSetSurface(nativePlayer, Surface(surfaceTexture))
    }

    override fun onSurfaceTextureSizeChanged(width: Int, height: Int) {
//        if (isPlayed) {
//            nativeUpdateViewPoint(nativePlayer, width, height)
//        }
        EvaJniUtil.updateViewPoint(nativePlayer, width, height)
    }

    override fun onSurfaceTextureDestroyed() {
            if (nativePlayer > 0) {
                EvaJniUtil.release(nativePlayer)
                nativePlayer = 0
            }

    }

    override fun onFrameAvailable(surfaceTexture: SurfaceTexture?) {
        EvaJniUtil.renderData()
    }

    override fun setAnimListener(listener: IEvaAnimListener) {
        if (nativePlayer > 0) {
            EvaJniUtil.setEvaAnimListener(listener)
        } else {
            evaAnimListener = listener
        }
    }

    override fun touchPoint(x:Int, y: Int) {
        EvaJniUtil.touchPoint(x, y)
    }

    override fun destroy() {
        remoteService?.asBinder()?.unlinkToDeath(mDeathRecipient, 0)
        serviceConnection?.let {
            EvaLog.i(TAG, "unbindService")
            context.unbindService(it)
        }
    }

    override fun setInitListener(listener: IEvaRemoteListener) {
        this.remoteListener = listener
    }
}