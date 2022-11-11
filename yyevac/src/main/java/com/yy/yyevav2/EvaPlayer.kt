package com.yy.yyevav2

import android.content.Context
import android.graphics.Bitmap
import android.os.Handler
import android.os.Looper
import android.util.AttributeSet
import android.view.View
import android.widget.FrameLayout
import com.yy.yyevav2.downloder.EvaDownloader
import com.yy.yyevav2.impl.EvaSrcSim
import com.yy.yyevav2.impl.IEvaAnimListener
import com.yy.yyevav2.impl.IEvaRemoteListener
import com.yy.yyevav2.log.EvaLog
import com.yy.yyevav2.params.FileSource
import com.yy.yyevav2.params.OptionParams
import com.yy.yyevav2.params.SourceParams
import com.yy.yyevav2.params.URLSource
import com.yy.yyevav2.player.IEvaPlayer
import com.yy.yyevav2.player.PlayerInner
import com.yy.yyevav2.util.EvaBitmapUtil
import java.io.File

class EvaPlayer @kotlin.jvm.JvmOverloads constructor(
    context: Context, attrs: AttributeSet? = null, defStyleAttr: Int = 0
) : FrameLayout(context, attrs, defStyleAttr) {

    private val TAG = EvaPlayer::class.java.simpleName
    private val BG_TAG = "bg_effect"
    private var mAttach = true
    private var mIsStarted = false
    private var mPlayer: IEvaPlayer? = null
    private var mDownloader: EvaDownloader? = null
    private var params = OptionParams()
    private val uiHandler by lazy { Handler(Looper.getMainLooper()) }
    private var v: View? = null
    private var source: SourceParams? = null

    init {
        onInit()
    }

    private fun onInit() {
        initPlayer()
        if (mDownloader != null) {
            mDownloader = EvaDownloader()
        }
    }

    private fun initPlayer() {
        if (null == mPlayer) {
            params.apply {
                usingSurfaceView = false
            }
            mPlayer = PlayerInner(context, params)
            mPlayer?.let {
                v = it.getRenderView()
                if (v != null) {
                    v!!.layoutParams =
                        LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT)
                    addView(v)
                } else {
                    EvaLog.i(TAG, "player view is null")
                    it.setInitListener(object : IEvaRemoteListener {
                        override fun initCallback() {
                            v = it.getRenderView()
                            v!!.layoutParams =
                                LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT)
                            addView(v)
                            EvaLog.i(TAG, "addview remote callback")
                            source?.let {
                                start(it)
                            }
                        }
                    })
                }
            }
        }
    }

    fun setBgImage(bg: Bitmap) {
        mPlayer?.addEffect(BG_TAG, bg, "scaleFill")
    }

    fun setLoop(count: Int) {
        params.playCount = count
        mPlayer?.setLoop(count)
    }

    fun start(source: SourceParams) {
        this.source = source
        EvaLog.i(TAG, "start")
        if (mIsStarted) return
        if (v == null) return
        onInit()
        mIsStarted = true
        if (source is URLSource) {
            mDownloader?.download(source.url, { path ->
                startInner(FileSource(File(path)))
            }, {
                mIsStarted = false
            })
        } else {
            startInner(source)
        }
    }

    private fun startInner(source: SourceParams) {
        if (mAttach) {
            mPlayer?.run {
//                setSourceParams(source)
                params.mp4Address = source.toParams()
                play()
            }
        }
    }

    fun setAnimListener(listener: IEvaAnimListener) {
        if (mAttach) {
            onInit()
            mPlayer?.setAnimListener(listener)
        }
    }

    fun addEffect(name: String, bitmap: Bitmap) {
        mPlayer?.addEffect(name, bitmap, "scaleFill")
    }

    fun addImgEffect(bitmap: Bitmap, src: EvaSrcSim) {
        addImgEffect(bitmap, src, src.scaleMode)
    }

    fun addImgEffect(bitmap: Bitmap, src: EvaSrcSim, scaleMode: String?) {
        if (src.srcTag == BG_TAG) {
            EvaLog.e(TAG, "$BG_TAG key is used for background source ,please use setBgImage Api")
            return
        }
        val sm = scaleMode ?: src.scaleMode
        mPlayer?.addEffect(src.srcTag, bitmap, sm)
    }

    fun addTxtEffect(txt: String, src: EvaSrcSim) {
        addTxtEffect(txt, src, src.scaleMode)
    }

    fun addTxtEffect(txt: String, src: EvaSrcSim, scaleMode: String?) {
        src.txt = txt
        val bitmap = EvaBitmapUtil.createTxtBitmap(src)
        val sm = scaleMode ?: src.scaleMode
        mPlayer?.addEffect(src.srcTag, bitmap, sm)
        bitmap.recycle()
    }

    fun stop() {
        mIsStarted = false
        mDownloader?.cancel()
        mPlayer?.stop()
    }

    private fun ui(f:()->Unit) {
        if (Looper.myLooper() == Looper.getMainLooper()) f() else uiHandler.post { f() }
    }

    fun release() {
        ui {
            removeAllViews()
        }
        mIsStarted = false
        mDownloader?.cancel()
        mDownloader = null
        mPlayer?.release()
        mPlayer?.destroy()
        mPlayer = null
        source = null
    }

    override fun onAttachedToWindow() {
        super.onAttachedToWindow()
        mAttach = true
    }

    override fun onDetachedFromWindow() {
        super.onDetachedFromWindow()
        mAttach = false
        release()
    }

    fun destroy() {
        mPlayer?.destroy()
    }
}