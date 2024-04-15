package com.yy.yyeva.recorder

import android.media.MediaMuxer
import android.os.Handler
import android.os.HandlerThread
import com.yy.yyeva.util.ELog
import com.yy.yyeva.util.EvaJniUtil
import java.util.concurrent.Semaphore




class EvaMediaRecorder {
    private val TAG = "EvaMediaRecorder"
    private var mediaMuxer: MediaMuxer? = null
    private var videoHandlerThread: HandlerThread? = null
    private var videoHandler: Handler? = null
    private var audioHandlerThread: HandlerThread? = null
    private var audioHandler: Handler? = null
    private var controllerId: Int = -1
    private var isMuxerStart = false
    private var videoRecorder: EvaVideoRecorder? = null
    private var audioRecorder: EvaAudioRecorder? = null

    private val stopSemaphore = Semaphore(1)

    fun initRecord(controllerId: Int, width: Int, height: Int) {
        ELog.i(TAG, "initRecord controllerId $controllerId, width $width, height $height")
        this.controllerId = controllerId //需要共享纹理播放对象id
        videoHandlerThread = HandlerThread("YYEvaVideoCodec")
        videoHandlerThread!!.start()
        videoHandler = Handler(videoHandlerThread!!.looper)
        videoHandler?.post {
            videoRecorder = EvaVideoRecorder()
            videoRecorder?.initVideoRecord(controllerId, width, height)
        }

        audioHandlerThread = HandlerThread("YYEvaAudioCodec")
        audioHandlerThread!!.start()
        audioHandler = Handler(audioHandlerThread!!.looper)
        audioHandler?.post {
            audioRecorder = EvaAudioRecorder()
//            audioRecorder?.initAudioRecord()
        }
    }

    fun startCodecRecord(path: String, speed: Int) {
        ELog.i(TAG, "startCodecRecord $path, $speed")
        //  H.264
        // 播放：
        //  MP4 -> 解复用 (解封装) -> 解码 -> 绘制
        //封装器 复用器
        // 一个 mp4 的封装器 将h.264 通过它写出到文件就可以了
        mediaMuxer = MediaMuxer(path, MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4)
        videoHandler?.post {
            videoRecorder?.startCodecRecord(mediaMuxer!!, speed)
            ELog.i(TAG, "startCodecRecord mediaCode start")
        }
//        audioHandler?.post {
//            audioRecorder?.startCodecRecord(mediaMuxer!!, speed)
//        }
    }

    fun encodeFrameInThread(endOfStream: Boolean, time: Long) {
        videoHandler?.post {
            try {
                videoRecorder?.encodeFrame(endOfStream)
            } catch (e: Exception) {
                ELog.e(TAG, "encodeFrameInThread $e")
            }
            EvaJniUtil.recordRender(controllerId, time)
        }
//        audioHandler?.post {
//            try {
//                audioRecorder?.encodeFrame(endOfStream)
//            } catch (e: Exception) {
//                ELog.e(TAG, "encodeFrameInThread $e")
//            }
//        }
    }

    fun stopCodecRecordInThread() {
        ELog.i(TAG, "stopCodecRecordInThread")
        videoHandler?.post {
            stopSemaphore.acquire()
            videoRecorder?.stopCodecRecord()
            //释放视频
            videoHandler?.looper?.quitSafely()
            videoHandler = null
            videoHandlerThread?.quitSafely()
            videoHandlerThread = null
            stopSemaphore.release()
        }
//        audioHandler?.post {
//            audioRecorder?.stopCodecRecord()
//            //释放音频
//            audioHandler?.looper?.quitSafely()
//            audioHandler = null
//            audioHandlerThread?.quitSafely()
//            audioHandlerThread = null
//        }

        stopSemaphore.acquireUninterruptibly(1)
        if (isMuxerStart) {
            mediaMuxer?.stop()
        }
        mediaMuxer?.release()
        mediaMuxer = null
        isMuxerStart = false
    }
}