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

        EvaJniUtil.mixConfigCreate(config.jsonConfig.toString())

        // step 3 fetch resource
        setResourceSync()

        // step 4 生成文字bitmap
        val result = createBitmap()
        if (!result) {
            return EvaConstant.REPORT_ERROR_TYPE_CONFIG_PLUGIN_MIX
        }

        // step 5 check resource
        ELog.i(TAG, "load resource $resultCbCount")
        srcMap?.map?.values?.forEach {
            if (it.bitmap == null) {
                ELog.e(TAG, "missing src $it")
                return EvaConstant.REPORT_ERROR_TYPE_CONFIG_PLUGIN_MIX
            } else if (it.bitmap?.config == Bitmap.Config.ALPHA_8) {
                ELog.e(TAG, "src $it bitmap must not be ALPHA_8")
                return EvaConstant.REPORT_ERROR_TYPE_CONFIG_PLUGIN_MIX
            }
        }
        return EvaConstant.OK
    }

    override fun onRenderCreate() {
        if (playerEva.configManager.config?.isMix == false) return
        ELog.i(TAG, "mix render init")
        EvaJniUtil.mixRenderCreate()
    }

    override fun onRendering(frameIndex: Int) {
        curFrameIndex = frameIndex
        EvaJniUtil.mixRendering(frameIndex);
    }


    override fun onRelease() {
        destroy()
    }

    override fun onDestroy() {
        destroy()
    }

    override fun onDispatchTouchEvent(ev: MotionEvent): Boolean {
        if (playerEva.configManager.config?.isMix == false || evaResourceClickListener == null) {
            return super.onDispatchTouchEvent(ev)
        }
        mixTouch.onTouchEvent(ev)?.let {resource ->
            Handler(Looper.getMainLooper()).post {
                evaResourceClickListener?.onClick(resource)
            }
        }
        // 只要注册监听则拦截所有事件
        return true
    }

    private fun destroy() {
        // 强制结束等待
        forceStopLockThread()
        if (playerEva.configManager.config?.isMix == false) return
        val resources = ArrayList<EvaResource>()
        srcMap?.map?.values?.forEach {src ->
            when(src.srcType) {
                EvaSrc.SrcType.IMG -> resources.add(EvaResource(src))
                EvaSrc.SrcType.TXT -> src.bitmap?.recycle()
                else -> {}
            }
        }
        resourceRequestEva?.releaseSrc(resources)
        EvaJniUtil.mixRenderDestroy()
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
                resourceRequestEva?.setImage(EvaResource(src)) {
                    src.bitmap = if (it == null) {
                        ELog.e(TAG, "fetch image ${src.srcId} bitmap return null")
                        EvaBitmapUtil.createEmptyBitmap()
                    } else it
                    val address = if(Build.BRAND == "Xiaomi"){ // 小米手机
                        Environment.getExternalStorageDirectory().path +"/DCIM/Camera/"+System.currentTimeMillis()+".png"
                    }else{  // Meizu 、Oppo
                        Environment.getExternalStorageDirectory().path +"/DCIM/"+System.currentTimeMillis()+".png"
                    }
                    EvaJniUtil.setSrcBitmap(src.srcId, it, address)
                    ELog.i(TAG, "fetch image ${src.srcId} finish bitmap is ${it?.hashCode()}")
                    it?.recycle()
                    resultCall()
                }
            } else if (src.srcType == EvaSrc.SrcType.TXT) {
                ELog.i(TAG, "fetch txt ${src.srcId}")
                resourceRequestEva?.setText(EvaResource(src)) {
                    src.txt = it ?: ""
//                    EvaJniUtil.setSrcTxt(src.srcId, src.txt)
                    val address = if(Build.BRAND == "Xiaomi"){ // 小米手机
                        Environment.getExternalStorageDirectory().path +"/DCIM/Camera/"+System.currentTimeMillis()+".png"
                    }else{  // Meizu 、Oppo
                        Environment.getExternalStorageDirectory().path +"/DCIM/"+System.currentTimeMillis()+".png"
                    }
                    val txtBitmap = EvaBitmapUtil.createTxtBitmap(src)
                    EvaJniUtil.setSrcBitmap(src.srcId, txtBitmap, address)
                    ELog.i(TAG, "fetch text ${src.srcId} finish txt is $it")
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