package com.yy.yyeva.view

import android.media.*
import android.util.Log
import com.yy.yyeva.EvaAnimPlayer
import com.yy.yyeva.decoder.Decoder
import com.yy.yyeva.decoder.HandlerHolder
import com.yy.yyeva.file.IEvaFileContainer
import com.yy.yyeva.util.EvaConstant
import com.yy.yyeva.util.ELog
import com.yy.yyeva.util.EvaMediaUtil
import java.lang.RuntimeException

class EvaAudioPlayer(val playerEva: EvaAnimPlayer) {

    companion object {
        private const val TAG = "${EvaConstant.TAG}.AudioPlayer"
    }

    var extractor: MediaExtractor? = null
    var decoder: MediaCodec? = null
    var audioTrack: AudioTrack? = null
    val decodeThread = HandlerHolder(null, null)
    var isRunning = false
    var playLoop = 0
    var isStopReq = false
    var needDestroy = false
    var isPause = false

    private fun prepareThread(): Boolean {
        return Decoder.createThread(decodeThread, "anim_audio_thread")
    }

    fun start(evaFileContainer: IEvaFileContainer) {
        isStopReq = false
        needDestroy = false
        if (!prepareThread()) return
        if (isRunning) {
            stop()
        }
        isRunning = true
        decodeThread.handler?.post {
            try {
                startPlay(evaFileContainer)
                isPause = false
            } catch (e: Throwable) {
                ELog.e(TAG, "Audio exception=$e", e)
                release()
            }
        }
    }

    fun pause() {
        ELog.i(TAG, "pause")
        isPause = true
    }

    fun resume() {
        ELog.i(TAG, "resume")
        isPause = false
    }

    fun stop() {
        isStopReq = true
    }

    private fun startPlay(evaFileContainer: IEvaFileContainer) {
        val extractor = EvaMediaUtil.getExtractor(evaFileContainer)
        this.extractor = extractor
        val audioIndex = EvaMediaUtil.selectAudioTrack(extractor)
        if (audioIndex < 0) {
            ELog.e(TAG, "cannot find audio track")
            release()
            return
        }
        extractor.selectTrack(audioIndex)
        val format = extractor.getTrackFormat(audioIndex)
        val mime = format.getString(MediaFormat.KEY_MIME) ?: ""
        ELog.i(TAG, "audio mime=$mime")
        if (!EvaMediaUtil.checkSupportCodec(mime)) {
            ELog.e(TAG, "mime=$mime not support")
            release()
            return
        }

        val decoder = MediaCodec.createDecoderByType(mime).apply {
            configure(format, null, null, 0)
            //跳转到需要的跳转位置
            if (playerEva.startPoint > 0) {
                //等待视频端设置sampleTime位置，因为音频和视频的p帧并不是相同，所以需要锚定以视频的时间为准
                while (playerEva.sampleTime == 0L) {
                    continue
                }
                extractor.seekTo(playerEva.sampleTime, MediaExtractor.SEEK_TO_CLOSEST_SYNC)
                ELog.i(TAG, "startPoint ${playerEva.startPoint}, sampleTime：${extractor.sampleTime}")
                extractor.advance()
            }

            start()
        }
        this.decoder = decoder

        val decodeInputBuffers = decoder.inputBuffers
        var decodeOutputBuffers = decoder.outputBuffers

        val bufferInfo = MediaCodec.BufferInfo()
        var sampleRate = format.getInteger(MediaFormat.KEY_SAMPLE_RATE)
        val channelConfig = getChannelConfig(format.getInteger(MediaFormat.KEY_CHANNEL_COUNT))

        if (playerEva.audioSpeed != 1.0f) { //设置音频播放速度
            sampleRate = (sampleRate * playerEva.audioSpeed).toInt()
        }
        
        val bufferSize = AudioTrack.getMinBufferSize(sampleRate, channelConfig, AudioFormat.ENCODING_PCM_16BIT)
        val audioTrack = AudioTrack(AudioManager.STREAM_MUSIC, sampleRate, channelConfig, AudioFormat.ENCODING_PCM_16BIT, bufferSize, AudioTrack.MODE_STREAM)
        this.audioTrack = audioTrack
        val state = audioTrack.state
        if (state != AudioTrack.STATE_INITIALIZED) {
            release()
            ELog.e(TAG, "init audio track failure")
            return
        }
        audioTrack.play()
        val timeOutUs = 1000L
        var isEOS = false
        while (!isStopReq) {

            if (isPause) {
                continue
            }

            if (!isEOS) {
                val inputIndex = decoder.dequeueInputBuffer(timeOutUs)
                if (inputIndex >= 0) {
                    val inputBuffer = decodeInputBuffers[inputIndex]
                    inputBuffer.clear()
                    val sampleSize = extractor.readSampleData(inputBuffer, 0)
                    if (sampleSize < 0) {
                        isEOS = true
                        decoder.queueInputBuffer(inputIndex, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM)
                    } else {
                        decoder.queueInputBuffer(inputIndex, 0, sampleSize, 0, 0)
                        extractor.advance()
                    }
                }
            }
            val outputIndex = decoder.dequeueOutputBuffer(bufferInfo, 0)
            if (outputIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                decodeOutputBuffers = decoder.outputBuffers
            }
            if (outputIndex >= 0) {
                val outputBuffer = decodeOutputBuffers[outputIndex]
                val chunkPCM = ByteArray(bufferInfo.size)
                outputBuffer.get(chunkPCM)
                outputBuffer.clear()
                audioTrack.write(chunkPCM, 0, bufferInfo.size)
                decoder.releaseOutputBuffer(outputIndex, false)
            }

            if (isEOS && bufferInfo.flags and MediaCodec.BUFFER_FLAG_END_OF_STREAM != 0) {
                if (--playLoop > 0) {
                    ELog.d(TAG, "Reached EOS, looping -> playLoop")
                    extractor.seekTo(0, MediaExtractor.SEEK_TO_CLOSEST_SYNC)
                    decoder.flush()
                    isEOS = false
                } else {
                    ELog.i(TAG, "decode finish")
                    release()
                    break
                }
            }
        }
        release()
    }


    private fun release() {
        try {
            decoder?.apply {
                stop()
                release()
            }
            decoder = null
            extractor?.release()
            extractor = null
            audioTrack?.apply {
                pause()
                flush()
                stop()
                release()
            }
            audioTrack = null
        } catch (e: Throwable) {
            ELog.e(TAG, "release exception=$e", e)
        }
        isRunning = false
        if (needDestroy) {
            destroyInner()
        }
    }

    fun destroy() {
        if (isRunning) {
            needDestroy = true
            stop()
        } else {
            destroyInner()
        }
    }

    private fun destroyInner() {
        if (playerEva.isDetachedFromWindow) {
            ELog.i(TAG, "destroyThread")
            decodeThread.handler?.removeCallbacksAndMessages(null)
            decodeThread.thread = Decoder.quitSafely(decodeThread.thread)
        }
    }

    private fun getChannelConfig(channelCount: Int): Int {
        return when (channelCount) {
            1 -> AudioFormat.CHANNEL_CONFIGURATION_MONO
            2 -> AudioFormat.CHANNEL_OUT_STEREO
            3 -> AudioFormat.CHANNEL_OUT_STEREO or AudioFormat.CHANNEL_OUT_FRONT_CENTER
            4 -> AudioFormat.CHANNEL_OUT_QUAD
            5 -> AudioFormat.CHANNEL_OUT_QUAD or AudioFormat.CHANNEL_OUT_FRONT_CENTER
            6 -> AudioFormat.CHANNEL_OUT_5POINT1
            7 -> AudioFormat.CHANNEL_OUT_5POINT1 or AudioFormat.CHANNEL_OUT_BACK_CENTER
            else -> throw RuntimeException("Unsupported channel count: $channelCount")
        }
    }
}