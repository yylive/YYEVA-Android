package com.yy.yyevav2.player

import android.content.Context
import android.graphics.Bitmap
import android.view.View
import com.yy.yyevav2.impl.IEvaAnimListener
import com.yy.yyevav2.impl.IEvaRemoteListener
import com.yy.yyevav2.params.EffectParams
import com.yy.yyevav2.params.OptionParams
import com.yy.yyevav2.params.SourceParams

class PlayerInner(private val context: Context, private val params: OptionParams) : IEvaPlayer {

    private var mPlayer: IEvaPlayer? = null
    private val mThread = EvaThread("EVA_THREAD_PLAYER_INNER")

    init {
        onInit()
    }

    private fun onInit() {
        mPlayer = EvaPlayerImpl(context, params)
        mThread.start()
    }

    override fun play() {
//        mThread.queueEvent {
//            mPlayer?.play()
//        }

        mPlayer?.play()
    }

    override fun setLoop(count: Int) {
        mPlayer?.setLoop(count)
    }

    override fun setBgImage(bg: Bitmap) {
        mPlayer?.setBgImage(bg)
    }

    override fun stop() {
//        mThread.queueEvent {
//            mPlayer?.stop()
//        }

        mPlayer?.stop()
    }

    override fun release() {
//        mThread.queueEventFront {
//            mPlayer?.release()
//            mThread.release()
//        }
        mPlayer?.release()
    }

    override fun isRunning(): Boolean = mPlayer?.isRunning() ?: false

    override fun setMute(isMute: Boolean) {
//        mThread.queueEvent {
//            mPlayer?.setMute(isMute)
//        }
        mPlayer?.setMute(isMute)
    }

    override fun getRenderView(): View? = mPlayer?.getRenderView()

    override fun getPlayerTag(): String = mPlayer?.getPlayerTag() ?: ""

    override fun setEffectParams(params: EffectParams) {
//        mThread.queueEvent {
//            mPlayer?.setEffectParams(params)
//        }
        mPlayer?.setEffectParams(params)
    }

    override fun setSourceParams(params: SourceParams) {
//        mThread.queueEvent {
//            mPlayer?.setSourceParams(params)
//        }
        mPlayer?.setSourceParams(params)
    }

    override fun setAnimListener(listener: IEvaAnimListener) {
        mPlayer?.setAnimListener(listener)
    }

    override fun addEffect(name: String, bitmap: Bitmap, scaleMode: String) {
        mPlayer?.addEffect(name, bitmap, scaleMode)
    }

    override fun setInitListener(listener: IEvaRemoteListener) {
        mPlayer?.setInitListener(listener)
    }

    override fun destroy() {
        mPlayer?.destroy()
    }
}