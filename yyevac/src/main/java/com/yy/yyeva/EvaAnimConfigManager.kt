package com.yy.yyeva

import android.annotation.TargetApi
import android.graphics.Bitmap
import android.graphics.Color
import android.media.MediaDataSource
import android.media.MediaMetadataRetriever
import android.os.Build
import android.os.SystemClock
import android.util.Base64
import android.util.Log
import com.yy.yyeva.file.EvaAssetsEvaFileContainer
import com.yy.yyeva.file.EvaFileContainer
import com.yy.yyeva.file.IEvaFileContainer
import com.yy.yyeva.util.EvaConstant
import com.yy.yyeva.util.ELog
import com.yy.yyeva.util.PointRect
import org.json.JSONObject
import java.io.ByteArrayOutputStream
import java.io.FileInputStream
import java.io.IOException
import java.lang.Exception
import java.nio.ByteBuffer
import java.util.zip.Inflater
import kotlin.math.abs


/**
 * 配置管理
 */
class EvaAnimConfigManager(var playerEva: EvaAnimPlayer) {

    var config: EvaAnimConfig? = null
    var isParsingConfig = false // 是否正在读取配置
    private var audioSpeed = 1.0f
    private var startDetect = 0L

    companion object {
        private const val TAG = "${EvaConstant.TAG}.EvaAnimConfigManager"
        // 是否使用HSV颜色空间判断灰度（更精确但稍慢）
        private const val USE_HSV_GRAY_DETECT = true
    }

    /**
     * 解析配置
     * @return true 解析成功 false 解析失败
     */
    fun parseConfig(evaFileContainer: IEvaFileContainer, enableVersion1: Boolean, defaultVideoMode: Int, defaultFps: Int): Int {
        try {
            isParsingConfig = true
            // 解析mp4配置
            val time = SystemClock.elapsedRealtime()
            val result = parse(evaFileContainer, defaultVideoMode, defaultFps)
            ELog.i(TAG, "parseConfig cost=${SystemClock.elapsedRealtime() - time}ms enableVersion1=$enableVersion1 result=$result")
            if (!result) {
                isParsingConfig = false
                return EvaConstant.REPORT_ERROR_TYPE_PARSE_CONFIG
            }
//            if (config?.isDefaultConfig == true && !enableVersion1) {
//                isParsingConfig = false
//                return EvaConstant.REPORT_ERROR_TYPE_PARSE_CONFIG
//            }
            // 插件解析配置
            val resultCode = config?.let {
                playerEva.pluginManager.onConfigCreate(it)
            } ?: EvaConstant.OK
            isParsingConfig = false
            return resultCode
        } catch (e : Throwable) {
            ELog.e(TAG, "parseConfig error $e", e)
            isParsingConfig = false
            return EvaConstant.REPORT_ERROR_TYPE_PARSE_CONFIG
        }
    }

    fun parse(evaFileContainer: IEvaFileContainer, defaultVideoMode: Int, defaultFps: Int): Boolean {
        val config = EvaAnimConfig()
        this.config = config

        if (playerEva.isNormalMp4) {
            setNormalMp4(defaultFps)
            return true
        }
        var jsonStr = evaFileContainer.getEvaJson() ?: ""  //读取sp缓存
        if (jsonStr.isEmpty()) {
            evaFileContainer.startRandomRead()
            val readBytes = ByteArray(1024)
            var readBytesLast = ByteArray(0)
            var bufStr = ""
            var bufStrS = ""
            val matchStart = "yyeffectmp4json[["
            val matchEnd = "]]yyeffectmp4json"
            var findStart = false
            var findEnd = false
            var actualRead: Int
            // ISO_8859_1 保证 1字节→1字符，避免 UTF-8 多字节替换导致跨块边界检测偏移
            while (evaFileContainer.read(readBytes, 0, readBytes.size).also { actualRead = it } > 0) {
                if (!findStart) { //没找到开头
                    bufStr = String(readBytes, 0, actualRead, Charsets.ISO_8859_1)
                    var index = bufStr.indexOf(matchStart)
                    if (index >= 0) { //分段1找到匹配开头
                        jsonStr = bufStr.substring(index + matchStart.length)
                        findStart = true
                        index = jsonStr.indexOf(matchEnd)
                        if (index >= 0) { //同时包含结尾段进行截取
                            findEnd = true
                            jsonStr = jsonStr.substring(0, index)
                            break
                        }
                    } else {
                        if (readBytesLast.isNotEmpty()) {
                            bufStrS = String(readBytesLast, Charsets.ISO_8859_1) +
                                    String(readBytes, 0, actualRead, Charsets.ISO_8859_1)
                            var indexS = bufStrS.indexOf(matchStart)
                            if (indexS >= 0) { //合并分段找到匹配开头
                                jsonStr = bufStrS.substring(indexS + matchStart.length)
                                findStart = true
                                indexS = jsonStr.indexOf(matchEnd)
                                if (indexS >= 0) { // 同时包含结尾段进行截取
                                    findEnd = true
                                    jsonStr = jsonStr.substring(0, indexS)
                                    break
                                }
                            }
                        }
                        //保存分段（只保留实际读取的字节）
                        readBytesLast = readBytes.copyOf(actualRead)
                    }
                } else {
                    bufStr = String(readBytes, 0, actualRead, Charsets.ISO_8859_1)
                    val index = bufStr.indexOf(matchEnd)
                    if (index >= 0) { //分段1找到匹配结尾
                        jsonStr += bufStr.substring(0, index)
                        findEnd = true
                        break
                    } else if (!readBytesLast.contentEquals(readBytes.copyOf(actualRead))) { //判定内容不一致
                        if (readBytesLast.isNotEmpty()) {
                            bufStrS = String(readBytesLast, Charsets.ISO_8859_1) +
                                    String(readBytes, 0, actualRead, Charsets.ISO_8859_1)
                            val indexS = bufStrS.indexOf(matchEnd)
                            if (indexS >= 0) { //合并分段找到匹配结尾
                                jsonStr = if (indexS > readBytesLast.size) {
                                    jsonStr.substring(
                                        0,
                                        jsonStr.length - (indexS - readBytesLast.size) - 1
                                    )
                                } else {
                                    jsonStr.substring(
                                        0,
                                        jsonStr.length - (readBytesLast.size - indexS)
                                    )
                                }
                                findEnd = true
                                break
                            }
                        }
                        //保存数据
                        jsonStr += bufStr
                        //保存分段
                        readBytesLast = readBytes.copyOf(actualRead)
                    }
                }
            }

            evaFileContainer.closeRandomRead()

            if (!findStart || !findEnd) {
                ELog.e(TAG, "yyeffectmp4json not found")
                evaFileContainer.setEvaJson("none") //检测后认为资源不存在json,不重复检测
                setNoJson(evaFileContainer, defaultFps)
                return true
            } else {
                //先用base64解密，再用zlib解密
                jsonStr =
                    zlib(Base64.decode(jsonStr.toByteArray(), Base64.DEFAULT)).decodeToString()
                ELog.d(TAG, "jsonStr:$jsonStr")
                evaFileContainer.setEvaJson(jsonStr) //检测后认为资源存在json,保存缓存
            }
        } else if (jsonStr == "none") {  //检测过后，认为是null
            ELog.i(TAG, "${evaFileContainer.getFile()?.path} 不存在json")
            setNoJson(evaFileContainer, defaultFps)
            return true
        } else {
            ELog.i(TAG, "检测正常，使用缓存json $jsonStr")
        }

        val jsonObj = JSONObject(jsonStr)
        config.jsonConfig = jsonObj
        val result = config.parse(jsonObj)
        if (config.fps == 0) {
            config.fps = defaultFps
        }
        playerEva.fps = config.fps
        return result
    }

    private fun setNoJson(evaFileContainer: IEvaFileContainer, defaultFps: Int) {
        if (playerEva.videoMode == EvaConstant.VIDEO_MODE_NORMAL_MP4 || playerEva.isNoJsonDetect) {// 没有设置,默认为正常mp4
            getMp4Type(evaFileContainer)
        }
        // 按照默认配置生成config
        setNormalMp4(defaultFps)
    }

    private fun setNormalMp4(defaultFps: Int) {
        config?.apply {
            isDefaultConfig = true
            if (playerEva.videoMode == EvaConstant.VIDEO_MODE_NORMAL_MP4) {
                playerEva.isNormalMp4 = true //设定为正常mp4
            }
            this.defaultVideoMode = playerEva.videoMode
            fps = defaultFps
        }
        playerEva.fps = config?.fps ?: defaultFps
    }

    /**
     * 默认配置解析（兼容老视频格式）
     */
    fun defaultConfig(_videoWidth: Int, _videoHeight: Int) {
        if (config?.isDefaultConfig == false) return
        config?.apply {
            videoWidth = _videoWidth
            videoHeight = _videoHeight
            when (defaultVideoMode) {
                EvaConstant.VIDEO_MODE_SPLIT_HORIZONTAL -> {
                    // 视频左右对齐（alpha左\rgb右）
                    width = _videoWidth / 2
                    height = _videoHeight
                    alphaPointRect = PointRect(0, 0, width, height)
                    rgbPointRect = PointRect(width, 0, width, height)
                }
                EvaConstant.VIDEO_MODE_SPLIT_VERTICAL -> {
                    // 视频上下对齐（alpha上\rgb下）
                    width = _videoWidth
                    height = _videoHeight / 2
                    alphaPointRect = PointRect(0, 0, width, height)
                    rgbPointRect = PointRect(0, height, width, height)
                }
                EvaConstant.VIDEO_MODE_SPLIT_HORIZONTAL_REVERSE -> {
                    // 视频左右对齐（rgb左\alpha右）
                    width = _videoWidth / 2
                    height = _videoHeight
                    rgbPointRect = PointRect(0, 0, width, height)
                    alphaPointRect = PointRect(width, 0, width, height)
                }
                EvaConstant.YYEVAColorRegion_AlphaMP4_alphaHalfRightTop -> {
                    width = _videoWidth * 2/ 3
                    height = _videoHeight
                    rgbPointRect = PointRect(0, 0, width, height)
                    alphaPointRect = PointRect(width, 0, width/2, height/2)
                }
                EvaConstant.VIDEO_MODE_SPLIT_VERTICAL_REVERSE -> {
                    // 视频上下对齐（rgb上\alpha下）
                    width = _videoWidth
                    height = _videoHeight / 2
                    rgbPointRect = PointRect(0, 0, width, height)
                    alphaPointRect = PointRect(0, height, width, height)
                }
                else -> {
                    // 默认视频左右对齐（alpha左\rgb右）
                    width = _videoWidth / 2
                    height = _videoHeight
                    alphaPointRect = PointRect(0, 0, width, height)
                    rgbPointRect = PointRect(width, 0, width, height)
                }
            }
        }
    }

    fun zlib(data: ByteArray): ByteArray {
        //定义byte数组用来放置解压后的数据
        //定义byte数组用来放置解压后的数据
        var output = ByteArray(0)
        val decompresser = Inflater()
        decompresser.reset()
        //设置当前输入解压
        //设置当前输入解压
        decompresser.setInput(data, 0, data.size)
        val o = ByteArrayOutputStream(data.size)
        try {
            val buf = ByteArray(1024)
            while (!decompresser.finished()) {
                val i: Int = decompresser.inflate(buf)
                o.write(buf, 0, i)
                if (i == 0) {
                    break
                }
            }
            output = o.toByteArray()
        } catch (e: Exception) {
            output = data
            e.printStackTrace()
        } finally {
            try {
                o.close()
            } catch (e: IOException) {
                e.printStackTrace()
            }
        }
        decompresser.end()
        return output
    }

    fun getMp4Type(evaFileContainer: IEvaFileContainer) {
        val type = evaFileContainer.getEvaMp4Type()
        if (type > EvaConstant.VIDEO_MODE_NORMAL_MP4_NONE) {  //判断已经检测过
            ELog.i(TAG, "已经检测过，getMp4Type $type")
            playerEva.videoMode = type
            return
        }

        val mmr = MediaMetadataRetriever()
        var canDetectMp4Type = false
        try {
            when {
                evaFileContainer is EvaFileContainer -> {
                    val file = evaFileContainer.getFile()
                    if (file.exists()) {
                        mmr.setDataSource(file.absolutePath)
                        canDetectMp4Type = true
                    }
                }
                evaFileContainer is EvaAssetsEvaFileContainer -> {
                    canDetectMp4Type = setupMmrFromAssets(mmr, evaFileContainer)
                }
            }

            if (canDetectMp4Type) {
                val duration = mmr.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION)?.toLong()
                if (duration != null && duration > 0) {
                    for (i in 1..6) {
                        startDetect = System.currentTimeMillis()
                        val bitmap = mmr.getFrameAtTime(
                            i * duration / 6 * 1000,
                            MediaMetadataRetriever.OPTION_CLOSEST
                        )
                        val isJudge = getConfigManager(bitmap)
                        bitmap?.recycle()
                        ELog.i(TAG, "detect image mp4Type ${System.currentTimeMillis() - startDetect}")
                        if (isJudge) break
                    }
                    ELog.i(TAG, "detect mp4Type ${playerEva.videoMode}")
                    evaFileContainer.setEvaMp4Type(playerEva.videoMode)
                } else {
                    ELog.e(TAG, "getMp4Type: METADATA_KEY_DURATION is null or 0")
                }
            }
        } catch (e: Exception) {
            ELog.e(TAG, "getMp4Type exception: $e", e)
        } finally {
            mmr.release()
        }
    }

    /**
     * 为 assets 路径配置 MediaMetadataRetriever 数据源。
     *
     * API 23+: 使用 MediaDataSource + FileChannel.read(position) 精确定位，
     *   避免 setDataSource(FileDescriptor, offset, length) 在 APK 内嵌 FD 上
     *   部分机型原生 seek 失效导致 extractMetadata 静默返回 null 的问题。
     * API 21-22: 降级为 FileDescriptor 方式尽力而为。
     */
    private fun setupMmrFromAssets(
        mmr: MediaMetadataRetriever,
        container: EvaAssetsEvaFileContainer
    ): Boolean {
        return try {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                mmr.setDataSource(container.createMediaDataSource())
            } else {
                // API 21-22：FileDescriptor offset 方式，少数旧机型可能仍失效
                val fd = container.assetFd
                if (fd.declaredLength >= 0) {
                    mmr.setDataSource(fd.fileDescriptor, fd.startOffset, fd.declaredLength)
                } else {
                    mmr.setDataSource(fd.fileDescriptor)
                }
            }
            true
        } catch (e: Exception) {
            Log.e(TAG, "setupMmrFromAssets failed: $e", e)
            false
        }
    }

    /**
     * 为 MediaMetadataRetriever 创建可靠的 MediaDataSource（API 23+）。
     *
     * setDataSource(FileDescriptor, offset, length) 在 APK 内嵌 FD 上，部分设备原生层
     * lseek 行为不正确，导致 extractMetadata 静默返回 null。
     * MediaDataSource 通过 FileChannel.read(position) 在 Java 层完成随机定位，
     * 绕开底层 seek 问题。
     */
    @TargetApi(Build.VERSION_CODES.M)
    private fun EvaAssetsEvaFileContainer.createMediaDataSource(): MediaDataSource {
        val startOffset = assetFd.startOffset
        val totalLength = assetFd.declaredLength
        // FileChannel.read(ByteBuffer, position) 是线程安全的随机读，不改变 channel 当前位置
        val channel = FileInputStream(assetFd.fileDescriptor).channel

        return object : MediaDataSource() {
            override fun readAt(position: Long, buffer: ByteArray, offset: Int, size: Int): Int {
                if (position >= totalLength) return -1
                val toRead = minOf(size.toLong(), totalLength - position).toInt()
                val bb = ByteBuffer.wrap(buffer, offset, toRead)
                return try {
                    val n = channel.read(bb, startOffset + position)
                    if (n <= 0) -1 else n
                } catch (e: IOException) {
                    -1
                }
            }

            override fun getSize(): Long = totalLength

            // 底层 fd 由 assetFd 管理，此处不关闭
            override fun close() {}
        }
    }

    /**
     * 返回已经判断完成
     */
    fun getConfigManager(bitmap: Bitmap?): Boolean {
        if (bitmap != null) {
            val w = bitmap.width
            val h = bitmap.height
            
            val sampleCount = 20
            Log.i(TAG, "ltIsGray")
            val ltIsGray = isGray(getArray(bitmap, 0, 0, sampleCount), sampleCount)
            Log.i(TAG, "rtIsGray")
            val rtIsGray = isGray(getArray(bitmap, w/2, 0, sampleCount), sampleCount)
            Log.i(TAG, "lbIsGray")
            val lbIsGray = isGray(getArray(bitmap, 0, h/2, sampleCount), sampleCount)
            Log.i(TAG, "rbIsGray")
            val rbIsGray = isGray(getArray(bitmap, w/2, h/2, sampleCount), sampleCount)
            Log.i(TAG, "ltIsGray $ltIsGray, rtIsGray $rtIsGray, lbIsGray $lbIsGray, rbIsGray $rbIsGray")

            val grayCount = listOf(ltIsGray, rtIsGray, lbIsGray, rbIsGray).count { it }
            
            when (grayCount) {
                0 -> {
                    //正常mp4
                    Log.i(TAG, "正常mp4")
                    playerEva.isNormalMp4 = true
                }
                2 -> {
                    when {
                        ltIsGray && lbIsGray && !rtIsGray && !rbIsGray -> {
                            //左灰右彩
                            Log.i(TAG, "左灰右彩")
                            playerEva.videoMode = EvaConstant.VIDEO_MODE_SPLIT_HORIZONTAL
                        }
                        !ltIsGray && !lbIsGray && rtIsGray && rbIsGray -> {
                            //左彩右灰
                            Log.i(TAG, "左彩右灰")
                            playerEva.videoMode = EvaConstant.VIDEO_MODE_SPLIT_HORIZONTAL_REVERSE
                        }
                        ltIsGray && rtIsGray && !lbIsGray && !rbIsGray -> {
                            //上灰下彩
                            Log.i(TAG, "上灰下彩")
                            playerEva.videoMode = EvaConstant.VIDEO_MODE_SPLIT_VERTICAL
                        }
                        !ltIsGray && !rtIsGray && lbIsGray && rbIsGray -> {
                            //上彩下灰
                            Log.i(TAG, "上彩下灰")
                            playerEva.videoMode = EvaConstant.VIDEO_MODE_SPLIT_VERTICAL_REVERSE
                        }
                        else -> {
                            Log.e(TAG, "无法识别的布局模式")
                            return false
                        }
                    }
                }
                else -> {
                    Log.e(TAG, "灰度区域数量异常: $grayCount")
                    return false
                }
            }
        } else {
            Log.e(TAG, "getConfigManager bitmap is null")
            return false
        }

        return true
    }
    //n*n n平均取点
    private fun getArray(bitmap: Bitmap, start_x: Int, start_y: Int, count: Int): IntArray {
        val w = bitmap.width
        val h = bitmap.height
        val regionW = w / 2
        val regionH = h / 2
        
        val stepX = regionW / (count + 1)
        val stepY = regionH / (count + 1)
        
        val a = IntArray(count * count)
        for(i in 0 until count) {
            for(j in 0 until count) {
                val x = start_x + stepX * (j + 1)
                val y = start_y + stepY * (i + 1)
                if (x < w && y < h) {
                    a[i * count + j] = bitmap.getPixel(x, y)
                }
            }
        }

        return a
    }

    //w_c*h_c 区域取点色值
    private fun getArrayAuto(bitmap: Bitmap, start_x: Int, start_y: Int): IntArray {
        val w = bitmap.width
        val h = bitmap.height
        //宽高区域取点，除以2是四分一区域
        val w_c = bitmap.width  / 30 /2
        val h_c = bitmap.height / 30 /2

        //宽高取点间距
        val w_i = w / (2 * (w_c + 1))
        val h_i = h / (2 * (w_c + 1))
        val a = IntArray(w_c * h_c)
        for(i in 0 until w_c) {
            for(j in 0 until h_c) {
                //获取取点的色值
                a[i*10 + j] = bitmap.getPixel(start_x + w_i * j, start_y + h_i * i)
            }
        }

        return a
    }

    //是否灰度区域
    private fun isGray(a:IntArray, sampleCount: Int = 10): Boolean {
        var grayPixelCount = 0
        val totalPixels = a.size
        
        for (c in a) {
            val isGrayPixel = if (USE_HSV_GRAY_DETECT) {
                isGrayByHSV(c)
            } else {
                isGrayByRGB(c)
            }
            
            if (isGrayPixel) {
                grayPixelCount++
            }
        }
        
        // 85%以上的像素为灰度才判定为灰度区域
        val grayRatio = grayPixelCount.toFloat() / totalPixels
        return grayRatio >= 0.85
    }
    
    /**
     * 使用HSV颜色空间判断是否为灰度像素
     * @param pixel 像素颜色值
     * @return true表示灰度像素
     */
    private fun isGrayByHSV(pixel: Int): Boolean {
        val hsv = FloatArray(3)
        Color.colorToHSV(pixel, hsv)
        // hsv[0]=色相, hsv[1]=饱和度, hsv[2]=亮度
        // 饱和度<15% 或 亮度<10% 视为灰度
        return hsv[1] < 0.15f || hsv[2] < 0.1f
    }
    
    /**
     * 使用RGB颜色空间判断是否为灰度像素
     * @param pixel 像素颜色值
     * @return true表示灰度像素
     */
    private fun isGrayByRGB(pixel: Int): Boolean {
        val r = Color.red(pixel)
        val g = Color.green(pixel)
        val b = Color.blue(pixel)
        
        val maxDiff = maxOf(abs(r - g), abs(g - b), abs(b - r))
        val avgBrightness = (r + g + b) / 3
        
        // RGB差值<=20视为灰度，或亮度<=20视为暗色/黑色
        return maxDiff <= 20 || avgBrightness <= 20
    }
    //获取视频的关键帧时间列表
    fun getMp4KeyframeTimes(filePath: String?): LongArray {
        val retriever = MediaMetadataRetriever()
        retriever.setDataSource(filePath)
        val durationString = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION)
        val duration = durationString!!.toLong() * 1000 // 转换为毫秒
        val numFramesString =
            retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_FRAME_COUNT)
        val numFrames = numFramesString!!.toInt()
        val keyframeTimes = LongArray(numFrames)
        for (i in 0 until numFrames) {
            val timeString =
                retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_CAPTURE_FRAMERATE)
            val time = (timeString!!.toFloat() * i * 1000).toLong() // 转换为毫秒
            keyframeTimes[i] = time.coerceAtMost(duration)
        }
        retriever.release()
        return keyframeTimes
    }
}