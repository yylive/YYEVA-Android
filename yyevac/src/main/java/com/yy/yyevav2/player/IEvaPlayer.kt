package com.yy.yyevav2.player

import android.graphics.Bitmap
import android.view.View
import com.yy.yyevav2.impl.IEvaAnimListener
import com.yy.yyevav2.impl.IEvaRemoteListener
import com.yy.yyevav2.params.EffectParams
import com.yy.yyevav2.params.SourceParams

interface IEvaPlayer {

    fun play()

    fun stop()

    fun release()

    fun isRunning(): Boolean

    fun setMute(isMute: Boolean)

    fun getRenderView(): View?

    fun getPlayerTag(): String

    fun setEffectParams(params: EffectParams)

    fun setSourceParams(params: SourceParams)

    fun setAnimListener(listener: IEvaAnimListener)

    fun setInitListener(listener: IEvaRemoteListener)

    fun addEffect(name: String, bitmap: Bitmap, scaleMode: String)

    fun setLoop(count: Int)

    fun setBgImage(bg: Bitmap)

    fun destroy()
}