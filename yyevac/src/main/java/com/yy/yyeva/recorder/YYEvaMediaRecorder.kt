package com.yy.yyeva.recorder

import android.content.Context
import android.media.MediaCodec
import android.media.MediaCodecInfo
import android.media.MediaFormat
import android.media.MediaMuxer
import android.media.MediaRecorder
import android.os.Build
import android.os.Handler
import android.os.HandlerThread
import android.util.Log
import android.view.Surface
import com.yy.yyeva.util.ELog
import com.yy.yyeva.util.EvaJniUtil
import java.io.File
import java.lang.RuntimeException

class YYEvaMediaRecorder {
    private val TAG = "YYEvaMediaRecorder"
    private var mediaRecorder: MediaRecorder? = null
    private var mediaCodec: MediaCodec? = null
    private var mInputSurface: Surface? = null
    private var mediaMuxer: MediaMuxer? = null
    private var index = 0
    private var speed = 0
    private var handlerThread: HandlerThread? = null
    private var handler: Handler? = null
    private var controllerId: Int = -1
    private var isMuxerStart = false
    private var hasInit = false
    private var initJson = ""

    fun startRecord(context: Context, surface: Surface, width: Int, height: Int, address: String) {
        mediaRecorder = MediaRecorder()
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            mediaRecorder?.setInputSurface(surface)
            mediaRecorder?.setOutputFormat(MediaRecorder.OutputFormat.MPEG_4)
            mediaRecorder?.setAudioEncoder(MediaRecorder.AudioEncoder.AAC)
            mediaRecorder?.setVideoEncoder(MediaRecorder.VideoEncoder.H264)
            mediaRecorder?.setVideoSize(width, height)
            val file = File(address)
            mediaRecorder?.setOutputFile(file.absolutePath)

            try {
                mediaRecorder?.prepare()
                mediaRecorder?.start()
                ELog.i(TAG, "Video record start")
            } catch (e: Exception) {
                ELog.e(TAG, e.toString())
            }
        }
    }

    fun stopRecord() {
        if (mediaRecorder != null) {
            try {
                mediaRecorder?.stop()
                mediaRecorder?.release()
                Log.i(TAG, "stopRecord")
            } catch (e: Exception) {
                ELog.e(TAG, e.toString())
            }
        }
    }

    fun initVideoRecord(controllerId: Int, width: Int, height: Int) {
        handlerThread = HandlerThread("YYEvaVideoCodec")
        handlerThread!!.start()
        handler = Handler(handlerThread!!.looper)
        handler?.post {
            // https://developer.android.google.cn/reference/android/media/MediaCodec mediacodec官方介绍
            // 比方MediaCodec的几种状态
            // avc即h264编码
            val mediaFormat = MediaFormat.createVideoFormat(MediaFormat.MIMETYPE_VIDEO_AVC, width, height)
            // 设置颜色格式
            // 本地原始视频格式（native raw video format）：这种格式通过COLOR_FormatSurface标记，并可以与输入或输出Surface一起使用
            mediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface)
            // 设置码率，通常码率越高，视频越清晰，但是对应的视频也越大
            mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, width * height * 4)
            // 设置帧率 三星s21手机camera预览时，支持的帧率为10-30
            // 通常这个值越高，视频会显得越流畅，一般默认设置成30，你最低可以设置成24，不要低于这个值，低于24会明显卡顿，微信为28
            mediaFormat.setInteger(MediaFormat.KEY_FRAME_RATE, 30)
            // 设置 I 帧间隔的时间
            // 通常的方案是设置为 1s，对于图片电影等等特殊情况，这里可以设置为 0，表示希望每一帧都是 KeyFrame
            // IFRAME_INTERVAL是指的帧间隔，这是个很有意思的值，它指的是，关键帧的间隔时间。通常情况下，你设置成多少问题都不大。
            // 比如你设置成10，那就是10秒一个关键帧。但是，如果你有需求要做视频的预览，那你最好设置成1
            // 因为如果你设置成10，那你会发现，10秒内的预览都是一个截图
            mediaFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL,5)
            //编码器
            mediaCodec = MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_VIDEO_AVC)
            //将参数配置给编码器
            mediaCodec?.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE)
            //java.lang.IllegalArgumentException: not a PersistentSurface
//            mediaCodec?.setInputSurface(surface) //设置surface
            //交给虚拟屏幕 通过opengl 将预览的纹理 绘制到这一个虚拟屏幕中
            //这样MediaCodec 就会自动编码 inputSurface 中的图像
            mInputSurface = mediaCodec?.createInputSurface()
            this.controllerId = controllerId //需要共享纹理播放对象id
            EvaJniUtil.initVideoRecord(controllerId, mInputSurface!!)
            hasInit = true
            if (initJson.isNotEmpty()) {
                setRecordRenderConfig(controllerId, initJson)
                initJson = ""
                ELog.i(TAG, "init config after surface init")
            }
        }
    }

    fun setRecordRenderConfig(controllerId: Int, json: String) {
        if (hasInit) {
            handler?.post {
                EvaJniUtil.setRecordRenderConfig(controllerId, json)
            }
        } else {
            ELog.i(TAG, "setRecordRenderConfig not init")
            initJson = json
        }
    }

    fun startCodecRecord(path: String, speed: Int) {
        ELog.i(TAG, "startCodecRecord $path, $speed")
        if (mediaCodec != null) {
            this.speed = speed
            handler?.post {
                //  H.264
                // 播放：
                //  MP4 -> 解复用 (解封装) -> 解码 -> 绘制
                //封装器 复用器
                // 一个 mp4 的封装器 将h.264 通过它写出到文件就可以了
                mediaMuxer = MediaMuxer(path, MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4)
                mediaCodec?.start()
                ELog.i(TAG, "startCodecRecord mediaCode start")
            }
        } else {
            ELog.e(TAG, "startCodecRecord mediaCodec null")
        }
    }

    fun encodeFrameInThread(endOfStream: Boolean, time: Long) {
        handler?.post {
            try {
                encodeFrame(endOfStream)
            } catch (e: Exception) {
                ELog.e(TAG, e.toString())
            }
            EvaJniUtil.recordRender(controllerId, time)
        }
    }

    fun encodeFrame(endOfStream: Boolean) {
        if (mediaCodec == null || mediaMuxer == null) {
            return
        }

        //不录了， 给mediacodec一个标记
        if (endOfStream) {
            ELog.d(TAG, "sending EOS to encoder")
            mediaCodec?.signalEndOfInputStream()
        }
        //输出缓冲区
        val bufferInfo = MediaCodec.BufferInfo()
        // 希望将已经编码完的数据都 获取到 然后写出到mp4文件
        while (true) {
            val status = mediaCodec!!.dequeueOutputBuffer(bufferInfo, 10000L)
            ELog.i(TAG, "drainEncoder($endOfStream), encoder Status($status)")
            //让我们重试  1、需要更多数据  2、可能还没编码为完（需要更多时间）
            if (status == MediaCodec.INFO_TRY_AGAIN_LATER) {
                // 如果是停止 我继续循环
                // 继续循环 就表示不会接收到新的等待编码的图像
                // 相当于保证mediacodec中所有的待编码的数据都编码完成了，不断地重试 取出编码器中的编码好的数据
                // 标记不是停止 ，我们退出 ，下一轮接收到更多数据再来取输出编码后的数据
                if (!endOfStream) {
                    //不写这个 会卡太久了，没有必要 你还是在继续录制的，还能调用这个方法的
                    break
                } else {
                    ELog.d(TAG, "no output available, spinning to await EOS")
                }
            } else if (status == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                if (isMuxerStart) {
                    throw RuntimeException("format changed twice")
                }
                val outputFormat = mediaCodec!!.outputFormat
                //配置封装器
                // 增加一路指定格式的媒体流 视频
                index = mediaMuxer!!.addTrack(outputFormat)
                ELog.i(TAG, "videotrack: $index")
                mediaMuxer!!.start()
                isMuxerStart = true
            } else if (status == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                //忽略
            } else {
                val outputBuffer = mediaCodec!!.getOutputBuffer(status)
                if (bufferInfo.flags and MediaCodec.BUFFER_FLAG_CODEC_CONFIG != 0) {
                    ELog.d(TAG, "ignoring BUFFER_FLAG_CODEC_CONFIG")
                    bufferInfo.size = 0
                }

                if (outputBuffer != null && bufferInfo.size != 0) {
                    ELog.i(TAG, "encode bufferInfo size: ${bufferInfo.size}")
                    if (!isMuxerStart) {
                        throw RuntimeException("muxer hasn't started")
                    }
                    //根据偏移定位
                    outputBuffer.position(bufferInfo.offset)
                    //ByteBuffer 可读写总长度
                    outputBuffer.limit(bufferInfo.offset + bufferInfo.size)
                    //写出
                    mediaMuxer!!.writeSampleData(index, outputBuffer, bufferInfo)
                } else {
                    ELog.e(TAG, "encodeFrame outputBuffer null")
                }
                //输出缓冲区 我们就使用完了，可以回收了，让mediacodec继续使用
                mediaCodec!!.releaseOutputBuffer(status, false)
                //结束
                if (bufferInfo.flags and MediaCodec.BUFFER_FLAG_END_OF_STREAM != 0) {
                    if (!endOfStream) {
                        ELog.e(TAG, "reached end of stream unexpectedly")
                    } else {
                        ELog.d(TAG, "end of stream reached")
                    }
                    break
                }
                if (endOfStream) {
                    stopCodecRecordInThread()
                }
            }
        }
    }

    fun getRecordSurface(): Surface? {
        return mInputSurface
    }

    fun stopCodecRecordInThread() {
        ELog.i(TAG, "stopCodecRecordInThread")
        handler?.post {
            stopCodecRecord()
        }
    }

    fun stopCodecRecord() {
        handler?.post {
            EvaJniUtil.stopRecord(controllerId)
            encodeFrame(true)
            mediaCodec?.stop()
            mediaCodec?.release()
            mediaCodec = null
            if (isMuxerStart) {
                mediaMuxer?.stop()
            }
            mediaMuxer?.release()
            mediaMuxer = null
            isMuxerStart = false
            handler?.looper?.quitSafely()
            handler = null
            handlerThread?.quitSafely()
            handlerThread = null
        }
    }
}