package com.yy.yyeva.decoder

import android.graphics.Bitmap
import android.graphics.SurfaceTexture
import android.media.MediaCodec
import android.media.MediaCodecInfo
import android.media.MediaExtractor
import android.media.MediaFormat
import android.opengl.GLES20
import android.os.Build
import android.view.Surface
import com.yy.yyeva.EvaAnimPlayer
import com.yy.yyeva.file.IEvaFileContainer
import com.yy.yyeva.util.ELog
import com.yy.yyeva.util.EvaConstant
import com.yy.yyeva.util.EvaJniUtil
import com.yy.yyeva.util.EvaMediaUtil
import java.nio.ByteBuffer
import java.nio.ByteOrder


class EvaHardDecoder(playerEva: EvaAnimPlayer) : Decoder(playerEva), SurfaceTexture.OnFrameAvailableListener {

    companion object {
        private const val TAG = "${EvaConstant.TAG}.HardDecoder"
    }

    private var glTexture: SurfaceTexture? = null
    private val bufferInfo by lazy { MediaCodec.BufferInfo() }
    private var needDestroy = false

    // 动画的原始尺寸
    private var videoWidth = 0
    private var videoHeight = 0

    // 动画对齐后的尺寸
    private var alignWidth = 0
    private var alignHeight = 0

    // 动画是否需要走YUV渲染逻辑的标志位
    private var needYUV = false
    private var outputFormat: MediaFormat? = null
    // 暂停
    private var isPause = false
    private var isRestart = false
    private var retryCount = 0  //初始化失败重试次数
    private val lock = Object()  //用于暂停和帧抽取速度调整
    private var evaFileContainer: IEvaFileContainer? = null

    override fun prepareToPlay(evaFileContainer: IEvaFileContainer) {
        ELog.i(TAG, "prepareToPlay")
        isStopReq = false
        isPause = true
        needDestroy = false
        isRunning = true
        renderThread.handler?.post {
            startPlay(evaFileContainer)
        }
    }

    override fun play() {
        resume()
    }

    override fun start(evaFileContainer: IEvaFileContainer) {
        isStopReq = false
        isPause = false
        needDestroy = false
        isRunning = true
        renderThread.handler?.post {
            startPlay(evaFileContainer)
        }
    }

    override fun onFrameAvailable(surfaceTexture: SurfaceTexture?) {
        if (isStopReq) return
        ELog.d(TAG, "onFrameAvailable")
        renderData()
    }

    /**
     * 渲染元素MP4
     */
    fun renderData() {
        renderThread.handler?.post {
            try {
                glTexture?.apply {
                    updateTexImage()
                    //渲染mp4数据
                    EvaJniUtil.renderFrame(playerEva.controllerId)
                    //元素混合
                    playerEva.pluginManager.onRendering()
                    //录制
                    if (playerEva.isVideoRecord) {
                        playerEva.mediaRecorder.encodeFrameInThread(false, timestamp)
                    }
                    //交换前后景数据
                    EvaJniUtil.renderSwapBuffers(playerEva.controllerId)
                }
            } catch (e: Throwable) {
                ELog.e(TAG, "render exception=$e", e)
            }
        }
    }

    fun save2DTextureToJPEG(textureId: Int, width: Int, height: Int) {
        val frameBuffers = IntArray(1)
        GLES20.glGenFramebuffers(1, frameBuffers, 0)
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, frameBuffers[0])
        if (frameBuffers[0] != 0 && textureId != 0 && width > 0 && height > 0) {
            GLES20.glFramebufferTexture2D(
                GLES20.GL_FRAMEBUFFER,
                GLES20.GL_COLOR_ATTACHMENT0,
                GLES20.GL_TEXTURE_2D,
                textureId,
                0
            )
            val mByteBuffer = ByteBuffer.allocateDirect(width * height * 4)
            mByteBuffer.order(ByteOrder.LITTLE_ENDIAN)
            GLES20.glReadPixels(
                0,
                0,
                width,
                height,
                GLES20.GL_RGBA,
                GLES20.GL_UNSIGNED_BYTE,
                mByteBuffer
            )
            val bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888)
            if (bitmap != null) {
                bitmap.copyPixelsFromBuffer(mByteBuffer)
                ELog.i(TAG, "copyPixelsFromBuffer")
//                ImageUtil.saveToFile(bitmap, true)
            }
        }
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0)
        GLES20.glDeleteFramebuffers(1, frameBuffers, 0)
    }

    private fun startPlay(evaFileContainer: IEvaFileContainer) {
        this.evaFileContainer = evaFileContainer
        var extractor: MediaExtractor? = null
        var decoder: MediaCodec? = null
        var format: MediaFormat? = null
        var trackIndex = 0
        var duration = 0L

        try {
            extractor = EvaMediaUtil.getExtractor(evaFileContainer)
            trackIndex = EvaMediaUtil.selectVideoTrack(extractor)
            if (trackIndex < 0) {
                throw RuntimeException("No video track found")
            }
            extractor.selectTrack(trackIndex)
            format = extractor.getTrackFormat(trackIndex)
            if (format == null) throw RuntimeException("format is null")

            // 是否支持h265
            if (EvaMediaUtil.checkIsHevc(format)) {
                if (!EvaMediaUtil.checkSupportCodec(EvaMediaUtil.MIME_HEVC)) {
                    onFailed(
                        EvaConstant.REPORT_ERROR_TYPE_HEVC_NOT_SUPPORT,
                        "${EvaConstant.ERROR_MSG_HEVC_NOT_SUPPORT} " +
                                "sdk:${Build.VERSION.SDK_INT}" +
                                ",support hevc:" + EvaMediaUtil.checkSupportCodec(EvaMediaUtil.MIME_HEVC))
                    extractor.release()
                    return
                }
            }

            videoWidth = format.getInteger(MediaFormat.KEY_WIDTH)
            videoHeight = format.getInteger(MediaFormat.KEY_HEIGHT)
            duration = format.getLong(MediaFormat.KEY_DURATION)
            if (!playerEva.isSetFps) {
                if (format.containsKey(MediaFormat.KEY_FRAME_RATE)) {  //防止读不到fps参数的导致无法播放
                    playerEva.fps = format.getInteger(MediaFormat.KEY_FRAME_RATE)
                } else {
                    ELog.i(TAG, "can not get frame-rate from media format")
                    playerEva.fps = playerEva.defaultFps
                }
            }
            // 防止没有INFO_OUTPUT_FORMAT_CHANGED时导致alignWidth和alignHeight不会被赋值一直是0
            alignWidth = videoWidth
            alignHeight = videoHeight
            ELog.i(TAG, "Video size is $videoWidth x $videoHeight")

            // 由于使用mediacodec解码老版本素材时对宽度1500尺寸的视频进行数据对齐，解码后的宽度变成1504，导致采样点出现偏差播放异常
            // 所以当开启兼容老版本视频模式并且老版本视频的宽度不能被16整除时要走YUV渲染逻辑
            // 但是这样直接判断有风险，后期想办法改
            needYUV = videoWidth % 16 != 0 && playerEva.enableVersion1

//            try {
//                if (!prepareRender(needYUV)) {
//                    throw RuntimeException("render create fail")
//                }
//            } catch (t: Throwable) {
//                onFailed(EvaConstant.REPORT_ERROR_TYPE_CREATE_RENDER, "${EvaConstant.ERROR_MSG_CREATE_RENDER} e=$t")
//                release(null, null)
//                return
//            }

            preparePlay(videoWidth, videoHeight)
            if (videoWidth <= 0 || videoHeight <= 0) {
                ELog.e(TAG, "error Video size is $videoWidth x $videoHeight")
                onFailed(EvaConstant.REPORT_ERROR_TYPE_EXTRACTOR_EXC, "error Video size is $videoWidth x $videoHeight")
                extractor.release()
                return
            } else if (EvaJniUtil.getExternalTexture(playerEva.controllerId) != -1
                && playerEva.evaAnimView.getSurfaceTexture() != null) {
                glTexture = playerEva.evaAnimView.getSurfaceTexture()!!.apply {
                    setOnFrameAvailableListener(this@EvaHardDecoder)
                    setDefaultBufferSize(videoWidth, videoHeight)
                }
            } else {
                if (retryCount > 5) {
                    ELog.e(TAG, "eva not init, can not get glTexture")
                    onFailed(
                        EvaConstant.REPORT_ERROR_TYPE_EXTRACTOR_EXC,
                        "eva not init, can not get glTexture"
                    )
                    release(decoder, extractor)
                } else {
                    ELog.e(TAG, "retryCount $retryCount eva not init, can not get glTexture", )
                    retryCount++
                    extractor.release()
                    startPlay(evaFileContainer)
                }
                return
            }
            EvaJniUtil.renderClearFrame(playerEva.controllerId)

        } catch (e: Throwable) {
            if (retryCount > 5) {
                ELog.e(TAG, "MediaExtractor exception e=$e", e)

                onFailed(
                    EvaConstant.REPORT_ERROR_TYPE_EXTRACTOR_EXC,
                    "${EvaConstant.ERROR_MSG_EXTRACTOR_EXC} e=$e"
                )
                release(decoder, extractor)
            } else {
                ELog.e(TAG, "retryCount $retryCount, MediaCodec configure exception e=$e", e)
                retryCount++
                extractor?.release()
                startPlay(evaFileContainer)
            }
            return
        }

        try {
            val mime = format.getString(MediaFormat.KEY_MIME) ?: ""
            ELog.i(TAG, "Video MIME is $mime")
            decoder = MediaCodec.createDecoderByType(mime).apply {
                if (needYUV) {
                    format.setInteger(
                        MediaFormat.KEY_COLOR_FORMAT,
                        MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar
                    )
                    configure(format, null, null, 0)
                } else {
                    configure(format, Surface(glTexture), null, 0)
                }
                //跳转到需要的跳转位置
                if (playerEva.startPoint in 1 .. duration) {
                    extractor.seekTo(playerEva.startPoint, MediaExtractor.SEEK_TO_PREVIOUS_SYNC)
                    ELog.i(TAG, "startPoint ${playerEva.startPoint}, sampleTime：${extractor.sampleTime}")
                    playerEva.sampleTime = extractor.sampleTime
                    extractor.advance()
                }
                start()
                decodeThread.handler?.post {
                    try {
                        startDecode(extractor, this)
                    } catch (e: Throwable) {
                        ELog.e(TAG, "MediaCodec exception e=$e", e)
                        onFailed(
                            EvaConstant.REPORT_ERROR_TYPE_DECODE_EXC,
                            "${EvaConstant.ERROR_MSG_DECODE_EXC} e=$e"
                        )
                        release(decoder, extractor)
                    }
                }
            }
        } catch (e: Throwable) {
            ELog.e(TAG, "retryCount $retryCount, MediaCodec configure exception e=$e", e)
            if (retryCount > 5) {
                onFailed(
                    EvaConstant.REPORT_ERROR_TYPE_DECODE_EXC,
                    "${EvaConstant.ERROR_MSG_DECODE_EXC} e=$e"
                )
                release(decoder, extractor)
                return
            } else {  //加入重试机制
                retryCount++
                startPlay(evaFileContainer)
            }
        }
    }

    override fun restart() {
        ELog.i(TAG, "restart")
        isRestart = true
    }

    override fun pause() {
        ELog.i(TAG, "pause")
        isPause = true
    }

    override fun resume() {
        ELog.i(TAG, "resume")
        isPause = false
        synchronized(lock) {
            lock.notifyAll()
        }
    }

    private fun startDecode(extractor: MediaExtractor, decoder: MediaCodec) {
        val TIMEOUT_USEC = 10000L
        var inputChunk = 0L
        var outputDone = false
        var inputDone = false
        var frameIndex = 0
        var isLoop = false
        var failDequeueCount = 0

        val decoderInputBuffers = decoder.inputBuffers

        if (playerEva.startPoint > 0L) {
            // 得到key对应的帧
            frameIndex = (extractor.sampleTime / ((1000 * 1000 / playerEva.fps))).toInt() - 1
            ELog.e(TAG, "decode frameIndex: $frameIndex")
        }

        while (!outputDone) {
            if (isStopReq) {
                ELog.i(TAG, "stop decode")
                release(decoder, extractor)
                return
            }

            synchronized(lock) {
                if (isPause) {
                    ELog.i(TAG, "lock wait")
                    lock.wait()
                }
            }
            if (isRestart) {
                extractor.seekTo(0, MediaExtractor.SEEK_TO_CLOSEST_SYNC)
                isRestart = false
            }
            try {
                if (!inputDone) {
                    val inputBufIndex = decoder.dequeueInputBuffer(TIMEOUT_USEC)
                    if (inputBufIndex >= 0) {
                        val inputBuf = decoderInputBuffers[inputBufIndex]
                        val chunkSize = extractor.readSampleData(inputBuf, 0)
                        if (chunkSize < 0) {
                            decoder.queueInputBuffer(inputBufIndex, 0, 0, 0L, MediaCodec.BUFFER_FLAG_END_OF_STREAM)
                            inputDone = true
                            ELog.i(TAG, "decode EOS")
                        } else {
                            val presentationTimeUs = extractor.sampleTime
                            decoder.queueInputBuffer(inputBufIndex, 0, chunkSize, presentationTimeUs, 0)
                            ELog.i(TAG, "submitted frame $inputChunk to dec, size=$chunkSize")
                            inputChunk++
                            extractor.advance()
                        }
                    } else {
                        ELog.d(TAG, "input buffer not available")
                    }
                }

                if (!outputDone) {
                    //抽取解码数据或状态
                    val decoderStatus = decoder.dequeueOutputBuffer(bufferInfo, TIMEOUT_USEC)
                    when {
                        decoderStatus == MediaCodec.INFO_TRY_AGAIN_LATER -> ELog.d(TAG, "no output from decoder available")
                        decoderStatus == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED -> ELog.d(TAG, "decoder output buffers changed")
                        decoderStatus == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED -> {
                            outputFormat = decoder.outputFormat
                            outputFormat?.apply {
                                try {
                                    // 有可能取到空值，做一层保护
                                    val stride = getInteger("stride")
                                    val sliceHeight = getInteger("slice-height")
                                    if (stride > 0 && sliceHeight > 0) {
                                        alignWidth = stride
                                        alignHeight = sliceHeight
                                    }
                                } catch (t: Throwable) {
                                    ELog.e(TAG, "$t", t)
                                }
                            }
                            ELog.i(TAG, "decoder output format changed: $outputFormat")
                        }
                        decoderStatus < 0 -> {
                            release(decoder, extractor)
                            throw RuntimeException("unexpected result from decoder.dequeueOutputBuffer: $decoderStatus")
                        }
                        else -> {  //正常解析
                            var loop = 0
                            if (bufferInfo.flags and MediaCodec.BUFFER_FLAG_END_OF_STREAM != 0) {
                                if (this.isLoop) {
                                    loop = 1
                                } else {
                                    loop = --playLoop
                                    playerEva.playLoop = playLoop // 消耗loop次数 自动恢复后能有正确的loop次数
                                    outputDone = playLoop <= 0
                                }
                            }
                            val doRender = !outputDone
                            if (doRender) { //控制帧率时间
                                speedControlUtil.preRender(bufferInfo.presentationTimeUs)
                            }

                            if (needYUV && doRender) {
//                            yuvProcess(decoder, decoderStatus)
                            }
                            //消耗解码数据
                            // release & render
                            decoder.releaseOutputBuffer(decoderStatus, doRender && !needYUV)

                            if (frameIndex == 0 && !isLoop) {
                                onVideoStart()
                            }

                            playerEva.pluginManager.onDecoding(frameIndex)
                            onVideoRender(frameIndex, playerEva.configManager.config)
                            //输出帧数据
                            frameIndex++
                            ELog.d(TAG, "decode frameIndex=$frameIndex")
                            if (loop > 0) {
                                ELog.d(TAG, "Reached EOD, looping")
                                playerEva.pluginManager.onLoopStart()
                                extractor.seekTo(0, MediaExtractor.SEEK_TO_CLOSEST_SYNC)
                                val presentationTimeUs = extractor.sampleTime
                                if (presentationTimeUs == -1L) {
                                    ELog.i(
                                        TAG,
                                        "Reached EOD, presentationTimeUs $presentationTimeUs"
                                    )
                                    decoder.apply {
                                        stop()
                                        release()
                                    }
                                    extractor.release()
                                    renderThread.handler?.post {
                                        startPlay(evaFileContainer!!)
                                    }
                                    break
                                } else {
                                    inputDone = false
                                    decoder.flush()
                                    speedControlUtil.reset()
                                    frameIndex = 0
                                    isLoop = true
                                    onVideoRestart()
                                    onVideoStart(true)
                                }
                            }
                            if (outputDone) {  //输出完成
                                if (playerEva.isSetLastFrame) {
                                    notReleaseLastFrame(decoder, extractor)
                                } else {
                                    release(decoder, extractor)
                                }
                            }
                        }
                    }
                }
            } catch (e: Exception) {  //解码帧失败跳过，直接继续解尝试
                if (failDequeueCount > 5) {
                    ELog.e(TAG, "failDequeueCount $failDequeueCount reason:$e")
                    release(decoder, extractor)
                } else {
                    failDequeueCount++
                    continue
                }
            }
        }
    }

    /**
     * 获取到解码后每一帧的YUV数据，裁剪出正确的尺寸
     */
//    private fun yuvProcess(decoder: MediaCodec, outputIndex: Int) {
//        val outputBuffer = decoder.outputBuffers[outputIndex]
//        outputBuffer?.let {
//            it.position(0)
//            it.limit(bufferInfo.offset + bufferInfo.size)
//            var yuvData = ByteArray(outputBuffer.remaining())
//            outputBuffer.get(yuvData)
//
//            if (yuvData.isNotEmpty()) {
//                var yData = ByteArray(videoWidth * videoHeight)
//                var uData = ByteArray(videoWidth * videoHeight / 4)
//                var vData = ByteArray(videoWidth * videoHeight / 4)
//
//                if (outputFormat?.getInteger(MediaFormat.KEY_COLOR_FORMAT) == MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar) {
//                    yuvData = yuv420spTop(yuvData)
//                }
//
//                yuvCopy(yuvData, 0, alignWidth, alignHeight, yData, videoWidth, videoHeight)
//                yuvCopy(yuvData, alignWidth * alignHeight, alignWidth / 2, alignHeight / 2, uData, videoWidth / 2, videoHeight / 2)
//                yuvCopy(yuvData, alignWidth * alignHeight * 5 / 4, alignWidth / 2, alignHeight / 2, vData, videoWidth / 2, videoHeight / 2)
//
//                render?.setYUVData(videoWidth, videoHeight, yData, uData, vData)
//                renderData()
//            }
//        }
//    }

    private fun yuv420spTop(yuv420sp: ByteArray): ByteArray {
        val yuv420p = ByteArray(yuv420sp.size)
        val ySize = alignWidth * alignHeight
        System.arraycopy(yuv420sp, 0, yuv420p, 0, alignWidth * alignHeight)
        var i = ySize
        var j = ySize
        while (i < ySize * 3 / 2) {
            yuv420p[j] = yuv420sp[i]
            yuv420p[j + ySize / 4] = yuv420sp[i + 1]
            i += 2
            j++
        }
        return yuv420p
    }

    private fun yuvCopy(src: ByteArray, srcOffset: Int, inWidth: Int, inHeight: Int, dest: ByteArray, outWidth: Int, outHeight: Int) {
        for (h in 0 until inHeight) {
            if (h < outHeight) {
                System.arraycopy(src, srcOffset + h * inWidth, dest, h * outWidth, outWidth)
            }
        }
    }

    private fun release(decoder: MediaCodec?, extractor: MediaExtractor?) {
        renderThread.handler?.post {
            EvaJniUtil.renderClearFrame(playerEva.controllerId)
            try {
                ELog.i(TAG, "release")
                decoder?.apply {
                    stop()
                    release()
                }
                extractor?.release()
                glTexture?.release()
                glTexture = null
                speedControlUtil.reset()
                playerEva.pluginManager.onRelease()
                playerEva.videoMode = EvaConstant.VIDEO_MODE_NORMAL_MP4 //重置为MP4
//                render?.releaseTexture()
                EvaJniUtil.releaseTexture(playerEva.controllerId)
            } catch (e: Throwable) {
                ELog.e(TAG, "release e=$e", e)
            }
            isRunning = false
            onVideoComplete()
            if (playerEva.isVideoRecord) {
                playerEva.mediaRecorder.stopCodecRecordInThread()
            }
            if (needDestroy) {
                destroyInner()
            }
            playLoop = 1
        }
    }

    private fun notReleaseLastFrame(decoder: MediaCodec?, extractor: MediaExtractor?) {
        ELog.i(TAG, "notReleaseLastFrame")
        decoder?.apply {
            stop()
            release()
        }
        extractor?.release()
        speedControlUtil.reset()
        playerEva.pluginManager.onRelease()
        isRunning = false
        //播放到最后一帧，也认为播放完成
        onVideoComplete(true)
    }

    private fun releaseLastFrame() {
        if (playerEva.isSetLastFrame) {
            renderThread.handler?.post {
                EvaJniUtil.renderClearFrame(playerEva.controllerId)
                try {
                    ELog.i(TAG, "releaseLastFrame")
                    glTexture?.release()
                    glTexture = null
//                render?.releaseTexture()
                    EvaJniUtil.releaseTexture(playerEva.controllerId)
                } catch (e: Throwable) {
                    ELog.e(TAG, "release e=$e", e)
                }
                isRunning = false
                onVideoComplete()
                if (needDestroy) {
                    destroyInner()
                }
                playerEva.isSetLastFrame = false
            }
        }
    }

    override fun destroy() {
        synchronized(lock) {
            isPause = false
            lock.notifyAll()
        }
        if (isRunning) {
            needDestroy = true
            stop()
        } else {
            releaseLastFrame()
            destroyInner()
        }
    }

    private fun destroyInner() {
        ELog.i(TAG, "destroyInner")
        renderThread.handler?.post {
            evaFileContainer?.close()
            playerEva.pluginManager.onDestroy()
            EvaJniUtil.destroyRender(playerEva.controllerId)
            if (playerEva.isVideoRecord) {
                playerEva.mediaRecorder.stopCodecRecordInThread()
            }
            playerEva.controllerId = -1
            onVideoDestroy()
            destroyThread()
            completeBlock = null
        }
    }
}