package com.yy.yyevav2.util

import android.graphics.Bitmap
import android.view.Surface
import com.yy.yyevav2.impl.IEvaAnimListener
import com.yy.yyevav2.params.OptionParams
import com.yy.yyevav2.service.IServiceInterface

object EvaJniUtil {
    init {
        System.loadLibrary("yyeva2")
    }

    private var service: IServiceInterface? = null

    fun setRemoteService(service: IServiceInterface) {
        this.service = service
    }


    fun createPlayer(tag: String, optionParams: OptionParams): Int {
        optionParams.isMainProcess = false
        service?.remoteCreatePlayer(tag, optionParams.toJson())
        optionParams.isMainProcess = true
        return nativeCreatePlayer(tag, optionParams.toJson())
    }

    fun play(optionParams: OptionParams): Int {
        if (service != null) {
            optionParams.isMainProcess = false
            service?.remotePlay(optionParams.toJson())
        }
        optionParams.isMainProcess = true
        return nativePlay(optionParams.toJson())
    }

    fun isRunning(nativePlayer: Int): Boolean {
        return nativeIsRunning(nativePlayer)
    }

    fun stop(nativePlayer: Int): Int {
        return if (service != null) {
            service?.remoteStop(nativePlayer) ?: -1
        } else {
            nativeStop(nativePlayer)
        }
    }

    fun release(nativePlayer: Int): Int {
        service?.remoteRelease(nativePlayer)
        return nativeRelease(nativePlayer)
    }

    fun destroySurface(nativePlayer: Int): Int {
        service?.remoteDestroySurface(nativePlayer)
        return nativeDestroySurface(nativePlayer)
    }

    fun setSurface(nativePlayer: Int, surface: Surface): Int {
        return nativeSetSurface(nativePlayer, surface)
    }

    fun setDecodeSurface(nativePlayer: Int, surface: Surface) {
        if (service != null) {
            service?.remoteSetDecodeSurface(nativePlayer, surface)
        } else {
            nativeSetDecodeSurface(nativePlayer, surface)
        }
    }

    fun updateViewPoint(nativePlayer: Int, width: Int, height: Int) {
        nativeUpdateViewPoint(nativePlayer, width, height)
    }

    fun setSourceParams(params: String) {
        service?.remoteSetSourceParams(params)
        nativeSetSourceParams(params)
    }

    fun addEffect(name: String, bitmap: Bitmap, scaleMode: String) {
        nativeAddEffect(name, bitmap, scaleMode)
    }

    fun setEffectParams(params: String) {
        if (service != null) {
            service?.remoteSetEffectParams(params)
        } else {
            nativeSetEffectParams(params)
        }
    }

    fun renderData() {
        nativeRenderData()
    }

    fun setPlayParams(params: String) {
        if (service != null) {
            service?.remoteSetPlayParams(params)
        } else {
            nativeSetPlayParams(params)
        }
    }

    fun setEvaAnimListener(listener: IEvaAnimListener) {
//        if (service != null) {
////            service?.remoteSetPlayParams(params)
//        } else {
//            nativeSetEvaAnimListener(listener)
//        }
        nativeSetEvaAnimListener(listener)
    }

    fun touchPoint(x:Int, y:Int) {
        nativeTouchPoint(x, y)
    }

    external fun nativeCreatePlayer(tag: String, optionParams: String): Int
    external fun nativePlay(optionParams: String): Int
    external fun nativeIsRunning(nativePlayer: Int): Boolean
    external fun nativeStop(nativePlayer: Int): Int
    external fun nativeRelease(nativePlayer: Int): Int
    external fun nativeDestroySurface(nativePlayer: Int): Int
    external fun nativeSetSurface(nativePlayer: Int, surface: Surface): Int
    external fun nativeSetDecodeSurface(nativePlayer: Int, surface: Surface)
    external fun nativeUpdateViewPoint(nativePlayer: Int, width: Int, height: Int)
    external fun nativeSetSourceParams(params: String)
    external fun nativeAddEffect(name: String, bitmap: Bitmap, scaleMode: String)
    external fun nativeSetEffectParams(params: String)
    external fun nativeRenderData()
    external fun nativeSetPlayParams(params: String)
    external fun nativeSetEvaAnimListener(listener: IEvaAnimListener)
    external fun nativeTouchPoint(x:Int, y:Int)
}