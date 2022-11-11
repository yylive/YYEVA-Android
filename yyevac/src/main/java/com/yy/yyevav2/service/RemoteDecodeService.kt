package com.yy.yyevav2.service

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Context
import android.content.Intent
import android.graphics.Bitmap
import android.os.Build
import android.os.IBinder
import android.view.Surface
import com.yy.yyevav2.impl.EvaSrcSim
import com.yy.yyevav2.impl.IEvaAnimListener
import com.yy.yyevav2.log.EvaLog
import com.yy.yyevav2.util.EvaJniUtil

class RemoteDecodeService: Service(), IEvaAnimListener {

    private val TAG = RemoteDecodeService::class.java.simpleName
    private var remoteCallback: IRemoteCallback? = null

    override fun onCreate() {
        super.onCreate()
        startForeground()
    }

    private fun startForeground() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val CHANNEL_ID = "yyeva";
            val notificationManager = getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
            val channel = NotificationChannel(CHANNEL_ID, "yyeva service channel", NotificationManager.IMPORTANCE_DEFAULT)
            notificationManager.createNotificationChannel(channel)
            EvaLog.i(TAG, "createNotificationChannel")
            val builder = Notification.Builder(this, CHANNEL_ID)
                .setContentTitle("yyeva")
            startForeground(1, builder.build())
            EvaLog.i(TAG, "startForeground")
        }
    }

    override fun onBind(p0: Intent?): IBinder? {
        return iServiceInterface.asBinder()
    }

    override fun onDestroy() {
        super.onDestroy()
    }

    private val iServiceInterface = object : IServiceInterface.Stub() {
        override fun remoteCreatePlayer(tag: String?, optionParams: String?): Int {
            EvaLog.i(TAG, "remoteCreatePlayer")
            if (tag != null && optionParams != null) {
                val id =  EvaJniUtil.nativeCreatePlayer(tag, optionParams)
                EvaJniUtil.setEvaAnimListener(this@RemoteDecodeService)
                return id
            }
            return -1
        }

        override fun remotePlay(optionParams: String?): Int {
            EvaLog.i(TAG, "remotePlay")
            if (optionParams != null) {
                return EvaJniUtil.nativePlay(optionParams)
            }
            return -1
        }

        override fun remoteIsRunning(nativePlayer: Int): Boolean {
            EvaLog.i(TAG, "remoteIsRunning")
            if (nativePlayer > -1) {
                return EvaJniUtil.nativeIsRunning(nativePlayer)
            }
            return false
        }

        override fun remoteStop(nativePlayer: Int): Int {
            EvaLog.i(TAG, "remoteStop")
            if (nativePlayer > -1) {
                return EvaJniUtil.nativeStop(nativePlayer)
            }
            return -1
        }

        override fun remoteRelease(nativePlayer: Int): Int {
            EvaLog.i(TAG, "remoteRelease")
            if (nativePlayer > -1) {
                return EvaJniUtil.nativeRelease(nativePlayer)
            }
            return -1
        }

        override fun remoteDestroySurface(nativePlayer: Int): Int {
            EvaLog.i(TAG, "remoteDestroySurface")
            if (nativePlayer > -1) {
                return EvaJniUtil.nativeDestroySurface(nativePlayer)
            }
            return -1
        }

        override fun remoteSetSurface(nativePlayer: Int, surface: Surface?): Int {
            EvaLog.i(TAG, "remoteSetSurface")
            if (nativePlayer > -1 && surface != null) {
                return EvaJniUtil.nativeSetSurface(nativePlayer, surface)
            }
            return -1
        }

        override fun remoteSetDecodeSurface(nativePlayer: Int, surface: Surface?) {
            EvaLog.i(TAG, "remoteSetDecodeSurface")
            if (nativePlayer > -1 && surface != null) {
                EvaJniUtil.nativeSetDecodeSurface(nativePlayer, surface)
            }
        }

        override fun remoteUpdateViewPoint(nativePlayer: Int, width: Int, height: Int) {
            EvaLog.i(TAG, "remoteUpdateViewPoint")
            if (nativePlayer > -1) {
                EvaJniUtil.nativeUpdateViewPoint(nativePlayer, width, height)
            }
        }

        override fun remoteSetSourceParams(params: String?) {
            EvaLog.i(TAG, "remoteSetSourceParams")
            if (params != null) {
                EvaJniUtil.nativeSetSourceParams(params)
            }
        }

        override fun remoteAddEffect(name: String?, bitmap: Bitmap?, scaleMode: String?) {
            EvaLog.i(TAG, "remoteAddEffect")
            if (name != null && bitmap != null) {
                val sm = scaleMode ?: "scaleFill"
                EvaJniUtil.nativeAddEffect(name, bitmap, sm)
            }
        }

        override fun remoteSetEffectParams(params: String?) {
            EvaLog.i(TAG, "remoteSetEffectParams")
            if (params != null) {
                EvaJniUtil.nativeSetEffectParams(params)
            }
        }

        override fun remoteRenderData() {
            EvaLog.i(TAG, "remoteRenderData")
            EvaJniUtil.nativeRenderData()
        }

        override fun remoteSetPlayParams(params: String?) {
            EvaLog.i(TAG, "remoteSetPlayParams")
            if (params != null) {
                EvaJniUtil.nativeSetPlayParams(params)
            }
        }

        override fun remoteTouchPoint(x: Int, y: Int) {
            EvaLog.i(TAG, "remoteTouchPoint")
            EvaJniUtil.nativeTouchPoint(x, y)
        }

        override fun registerCallback(callback: IRemoteCallback?) {
            EvaLog.i(TAG, "regsiterCallback")
            remoteCallback = callback
        }
    }

    override fun onVideoStart() {
        EvaLog.i(TAG, "onVideoStart")
    }

    override fun onVideoRender(frameIndex: Int) {
        EvaLog.i(TAG, "onVideoRender")
        remoteCallback?.onVideoRender(frameIndex)
    }

    override fun onVideoComplete() {
        EvaLog.i(TAG, "onVideoComplete")
        remoteCallback?.onVideoComplete()
    }


    override fun onVideoDestroy() {
        EvaLog.i(TAG, "onVideoDestroy")
        remoteCallback?.onVideoDestroy()
    }

    override fun onFailed(errorType: Int, errorMsg: String?) {
        EvaLog.i(TAG, "onFailed errorType=$errorType errorMsg=$errorMsg")
        remoteCallback?.onFailed(errorType, errorMsg)
    }

    override fun touchCallback(x: Int, y: Int, evaSrc: EvaSrcSim) {
    }
}