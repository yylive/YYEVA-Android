package com.yy.yyeva.mix

import android.graphics.Bitmap
import android.os.*
import android.view.MotionEvent
import com.yy.yyeva.EvaAnimPlayer
import com.yy.yyeva.util.EvaConstant
import com.yy.yyeva.EvaAnimConfig
import com.yy.yyeva.inter.IEvaFetchResource
import com.yy.yyeva.inter.OnEvaResourceClickListener
import com.yy.yyeva.plugin.IEvaAnimPlugin
import com.yy.yyeva.util.ELog
import com.yy.yyeva.util.EvaBitmapUtil
import com.yy.yyeva.util.EvaJniUtil

class EvaMixAnimPlugin(val playerEva: EvaAnimPlayer): IEvaAnimPlugin {

    companion object {
        private const val TAG = "${EvaConstant.TAG}.MixAnimPlugin"
    }
    var resourceRequestEva: IEvaFetchResource? = null
    var evaResourceClickListener: OnEvaResourceClickListener? = null
    var srcMap: EvaSrcMap? = null
    var frameAll: EvaFrameAll? = null
    var curFrameIndex = -1 // 当前帧
    private var resultCbCount = 0 // 回调次数
    private val mixTouch by lazy { EvaMixTouch(this) }
    var autoTxtColorFill = true // 是否启动自动文字填充 默认开启

    // 同步锁
    private val lock = Object()
    private var forceStopLock = false


    override fun onConfigCreate(config: EvaAnimConfig): Int {
        if (!config.isMix) return EvaConstant.OK
        if (resourceRequestEva == null) {
            ELog.e(TAG, "IFetchResource is empty")
            // 没有设置IFetchResource 当成普通视频播放
            return EvaConstant.OK
        }
        // step 1 parse src
        dillSrc(config)

        // step 2 parse frame
        dillFrame(config)

        //返回渲染管理器id，正常返回值大于0,
        val controllerId = EvaJniUtil.mixConfigCreate(playerEva.controllerId, config.jsonConfig.toString())
        playerEva.controllerId = controllerId

        // step 3 fetch resource
        setResourceSync()

        // step 4 生成文字bitmap
//        val result = createBitmap()
//        if (!result) {
//            return EvaConstant.REPORT_ERROR_TYPE_CONFIG_PLUGIN_MIX
//        }
//
//        // step 5 check resource
//        ELog.i(TAG, "load resource $resultCbCount")
//        srcMap?.map?.values?.forEach {
//            if (it.bitmap == null) {
//                ELog.e(TAG, "missing src $it")
//                return EvaConstant.REPORT_ERROR_TYPE_CONFIG_PLUGIN_MIX
//            } else if (it.bitmap?.config == Bitmap.Config.ALPHA_8) {
//                ELog.e(TAG, "src $it bitmap must not be ALPHA_8")
//                return EvaConstant.REPORT_ERROR_TYPE_CONFIG_PLUGIN_MIX
//            }
//        }
        return EvaConstant.OK
    }

    override fun onRenderCreate() {
        if (playerEva.configManager.config?.isMix == false) return
        ELog.i(TAG, "mix render init")
        EvaJniUtil.mixRenderCreate(playerEva.controllerId)
    }

    override fun onRendering(frameIndex: Int) {
        curFrameIndex = frameIndex
        EvaJniUtil.mixRendering(playerEva.controllerId, frameIndex);
    }


    override fun onRelease() {
        if (playerEva.configManager.config?.isMix == false) return
        val resources = ArrayList<EvaResource>()
        srcMap?.map?.values?.forEach {src ->
            resources.add(EvaResource(src))
        }
        resourceRequestEva?.releaseSrc(resources)
    }

    override fun onDestroy() {
        destroy()
    }

    override fun onDispatchTouchEvent(ev: MotionEvent): Boolean {
        if (evaResourceClickListener == null) {
            return super.onDispatchTouchEvent(ev)
        }
        mixTouch.onTouchEvent(ev).let {resource ->
            if (playerEva.configManager.config?.isMix == true && resource != null) {
                Handler(Looper.getMainLooper()).post {
                    evaResourceClickListener?.onClick(resource)
                }
                return true
            } else {
                return super.onDispatchTouchEvent(ev)
            }
        }
    }

    private fun destroy() {
        // 强制结束等待
        forceStopLockThread()
        EvaJniUtil.mixRenderDestroy(playerEva.controllerId)
    }

    private fun dillSrc(config: EvaAnimConfig) {
        config.effects?.apply {
            srcMap = EvaSrcMap(this)
        }
    }


    private fun dillFrame(config: EvaAnimConfig) {
        config.datas?.apply {
            frameAll = EvaFrameAll(this)
        }
    }


    private fun setResourceSync() {
        synchronized(lock) {
            forceStopLock = false // 开始时不会强制关闭
        }
        val time = SystemClock.elapsedRealtime()
        val totalSrc = srcMap?.map?.size ?: 0
        ELog.i(TAG, "load resource totalSrc = $totalSrc")

        resultCbCount = 0
        srcMap?.map?.values?.forEach {src ->
            if (src.srcType == EvaSrc.SrcType.IMG) {
                ELog.i(TAG, "fetch image ${src.srcId}")
                resourceRequestEva?.setImage(EvaResource(src)) { bitmap, fixType ->
                    src.bitmap = if (bitmap == null) {
                        ELog.e(TAG, "fetch image ${src.srcId} bitmap return null")
                        EvaBitmapUtil.createEmptyBitmap()
                    } else bitmap.copy(Bitmap.Config.ARGB_8888, true)
                    val scaleMode = when (fixType) {
                        null -> {
                            src.scaleMode
                        }
                        EvaSrc.FitType.CENTER_FULL -> {
                            "aspectFill"
                        }
                        EvaSrc.FitType.CENTER_FIT -> {
                            "aspectFit"
                        }
                        else -> {
                            "scaleFill"
                        }
                    }
                    EvaJniUtil.setSrcBitmap(playerEva.controllerId, src.srcId, src.bitmap, scaleMode)
                    ELog.i(TAG, "fetch image ${src.srcId} finish bitmap is ${src.bitmap?.hashCode()}")
                    //内部创建完纹理立刻释放
                    src.bitmap?.recycle()
                    resultCall()
                }
            } else if (src.srcType == EvaSrc.SrcType.TXT) {
                ELog.i(TAG, "fetch txt ${src.srcId}")
                resourceRequestEva?.setText(EvaResource(src)) { eva ->
                    src.txt = eva.text ?: ""
                    src.textAlign = eva.textAlign ?: "center"
//                    EvaJniUtil.setSrcTxt(src.srcId, src.txt)
                    if (eva.textColor != -1) {
                        src.color = eva.textColor
                    }
                    if (eva.fontSize > 0) {
                        src.frontSize = eva.fontSize
                    }
                    val txtBitmap = EvaBitmapUtil.createTxtBitmap(src, eva.typeFace)
                    EvaJniUtil.setSrcBitmap(playerEva.controllerId, src.srcId, txtBitmap, src.scaleMode)
                    ELog.i(TAG, "fetch text ${src.srcId} finish txt is ${src.textAlign}")
                    txtBitmap.recycle()
                    resultCall()
                }
            }
        }

        // 同步等待所有资源完成
        synchronized(lock) {
            while (resultCbCount < totalSrc && !forceStopLock) {
                lock.wait()
            }
        }
        ELog.i(TAG, "setResourceSync cost=${SystemClock.elapsedRealtime() - time}ms")
    }

    private fun forceStopLockThread() {
        synchronized(lock) {
            forceStopLock = true
            lock.notifyAll()
        }
    }

    private fun resultCall() {
        synchronized(lock) {
            resultCbCount++
            lock.notifyAll()
        }
    }

    private fun createBitmap(): Boolean {
        return try {
            srcMap?.map?.values?.forEach { src ->
                if (src.srcType == EvaSrc.SrcType.TXT) {
                    src.bitmap = EvaBitmapUtil.createTxtBitmap(src)
                }
            }
            true
        } catch (e: OutOfMemoryError) {
            ELog.e(TAG, "draw text OOM $e", e)
            false
        }
    }
}