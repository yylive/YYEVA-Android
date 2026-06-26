package com.yy.yyeva

import android.annotation.TargetApi
import android.graphics.Bitmap
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
import com.yy.yyeva.util.EvaVapConfigParser
import com.yy.yyeva.util.GrayColorJudge
import com.yy.yyeva.util.Mp4MetadataReader
import com.yy.yyeva.util.PointRect
import org.json.JSONObject
import java.io.ByteArrayOutputStream
import java.io.FileInputStream
import java.io.IOException
import java.lang.Exception
import java.nio.ByteBuffer
import java.util.zip.Inflater


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
        // json缓存标记
        private const val CACHE_NONE = "none" // yyeffectmp4json未找到，vapc尚未检测（兼容旧缓存）
        private const val CACHE_NONE_VAP_CHECKED = "none_vap_checked" // yyeffectmp4json和vapc均未找到
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

        val mergeInfo = readMergeInfo(evaFileContainer)
        if (!mergeInfo.isNullOrEmpty()) {
            jsonStr =
                zlib(Base64.decode(mergeInfo.toByteArray(), Base64.DEFAULT)).decodeToString()
            ELog.d(TAG, "jsonStr:$jsonStr")
            evaFileContainer.setEvaJson(jsonStr)
        }

        // 1. 有效缓存直接使用
        if (jsonStr.isNotEmpty() && jsonStr != CACHE_NONE && jsonStr != CACHE_NONE_VAP_CHECKED) {
            ELog.i(TAG, "检测正常，使用缓存json $jsonStr")
            return parseConfigJson(jsonStr, defaultFps)
        }

        // 2. 已确认 yyeffectmp4json 和 vapc 都没有，直接走默认配置
        if (jsonStr == CACHE_NONE_VAP_CHECKED) {
            ELog.i(TAG, "${evaFileContainer.getFile()?.path} 不存在json(vapc已检测)")
            setNoJson(evaFileContainer, defaultFps)
            return true
        }

        // 3. jsonStr为空(从未检测) 或 == CACHE_NONE(旧缓存,仅yyeffect未测,vapc未测)
        //    先扫描 yyeffectmp4json（仅空缓存时需要，CACHE_NONE 已知 yyeffect 没有）
        if (jsonStr.isEmpty()) {
            val scanned = scanYyeffectJson(evaFileContainer)
            if (scanned != null) {
                ELog.d(TAG, "jsonStr:$scanned")
                evaFileContainer.setEvaJson(scanned) //检测后认为资源存在json,保存缓存
                return parseConfigJson(scanned, defaultFps)
            }
            ELog.e(TAG, "yyeffectmp4json not found")
        }

        // 4. yyeffectmp4json 没找到，尝试解析 VAP 格式的 vapc box
        val vapcJson = EvaVapConfigParser.parseVapc(evaFileContainer)
        if (vapcJson != null) {
            ELog.i(TAG, "parse vapc config success")
            val vapStr = vapcJson.toString()
            evaFileContainer.setEvaJson(vapStr) //缓存转换后的YYEVA格式json，避免重复解析
            return parseConfigJson(vapStr, defaultFps)
        }

        // 5. yyeffectmp4json 和 vapc 均未找到
        ELog.e(TAG, "yyeffectmp4json and vapc both not found")
        evaFileContainer.setEvaJson(CACHE_NONE_VAP_CHECKED) //标记两者均已检测，不重复检测
        setNoJson(evaFileContainer, defaultFps)
        return true
    }

    private fun readMergeInfo(evaFileContainer: IEvaFileContainer): String? {
        return try {
            val file = evaFileContainer.getFile()
            when {
                file != null && file.exists() -> Mp4MetadataReader.readMergeInfo(file.inputStream())
                evaFileContainer is EvaAssetsEvaFileContainer -> {
                    Mp4MetadataReader.readMergeInfo(evaFileContainer.assetFd.createInputStream())
                }
                else -> null
            }
        } catch (e: Exception) {
            ELog.e(TAG, "readMergeInfo failed: $e", e)
            null
        }
    }

    /**
     * 扫描 mp4 中的 yyeffectmp4json 标记，base64+zlib 解密后返回 json 字符串。
     * @return 解密后的 json；未找到返回 null
     */
    private fun scanYyeffectJson(evaFileContainer: IEvaFileContainer): String? {
        evaFileContainer.startRandomRead()
        val readBytes = ByteArray(1024)
        var readBytesLast = ByteArray(0)
        var jsonStr = ""
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
            return null
        }
        //先用base64解密，再用zlib解密
        return zlib(Base64.decode(jsonStr.toByteArray(), Base64.DEFAULT)).decodeToString()
    }

    /**
     * 用 json 字符串解析配置并设置 fps。
     */
    private fun parseConfigJson(jsonStr: String, defaultFps: Int): Boolean {
        val cfg = config ?: return false
        val jsonObj = JSONObject(jsonStr)
        cfg.jsonConfig = jsonObj
        val result = cfg.parse(jsonObj)
        if (cfg.fps == 0) {
            cfg.fps = defaultFps
        }
        playerEva.fps = cfg.fps
        return result
    }

    private fun setNoJson(evaFileContainer: IEvaFileContainer, defaultFps: Int) {
        if (playerEva.videoMode == EvaConstant.VIDEO_MODE_NORMAL_MP4) {// 没有设置,默认为正常mp4
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
            val count = 10
            Log.i(TAG, "ltIsGray")
            val ltIsGray = isGray(getArray(bitmap, 0, 0, count))
            Log.i(TAG, "rtIsGray")
            val rtIsGray = isGray(getArray(bitmap, w/2, 0, count))
            Log.i(TAG, "lbIsGray")
            val lbIsGray = isGray(getArray(bitmap, 0, h/2, count))
            Log.i(TAG, "rbIsGray")
            val rbIsGray = isGray(getArray(bitmap, w/2, h/2, count))
            Log.i(TAG, "ltIsGray $ltIsGray, rtIsGray $rtIsGray, lbIsGray $lbIsGray, rbIsGray $rbIsGray")

            if (!ltIsGray && !lbIsGray && !rtIsGray && !rbIsGray) {
                //正常mp4
                Log.i(TAG, "正常mp4")
                playerEva.isNormalMp4 = true
            } else if (ltIsGray && lbIsGray && (!rtIsGray || !rbIsGray)) {
                //左灰右彩
                Log.i(TAG, "左灰右彩")
                playerEva.videoMode = EvaConstant.VIDEO_MODE_SPLIT_HORIZONTAL
            } else if ((!ltIsGray || !lbIsGray) && rtIsGray && rbIsGray) {
                //左彩右灰
                Log.i(TAG, "左彩右灰")
                playerEva.videoMode = EvaConstant.VIDEO_MODE_SPLIT_HORIZONTAL_REVERSE
            } else if (ltIsGray && rtIsGray && (!lbIsGray || !rbIsGray)) {
                //上灰下彩
                Log.i(TAG, "上灰下彩")
                playerEva.videoMode = EvaConstant.VIDEO_MODE_SPLIT_VERTICAL
            } else if ((!ltIsGray || !rtIsGray) && lbIsGray && rbIsGray) {
                //上彩下灰
                Log.i(TAG, "上彩下灰")
                playerEva.videoMode = EvaConstant.VIDEO_MODE_SPLIT_VERTICAL_REVERSE
            } else {
                return false
            }
        } else {
            Log.e(TAG, "getConfigManager bitmap is null")
            return false
        }

        return true
    }
    //3*3 + 边缘3+3+1 取点
    private fun getArray(bitmap: Bitmap, start_x: Int, start_y: Int): IntArray {
        val w = bitmap.width
        val h = bitmap.height
        val w_i = w/8
        val h_i = h/8
        val a = IntArray(16)
        a[0] = bitmap.getPixel(start_x + w_i, start_y + h_i)
        a[1] = bitmap.getPixel(start_x + w_i*2, start_y + h_i)
        a[2] = bitmap.getPixel(start_x + w_i*3, start_y + h_i)
        a[3] = bitmap.getPixel(start_x + w_i, start_y + h_i*2)
        a[4] = bitmap.getPixel(start_x + w_i*2, start_y + h_i*2)
        a[5] = bitmap.getPixel(start_x + w_i*3, start_y + h_i*2)
        a[6] = bitmap.getPixel(start_x + w_i, start_y + h_i*3)
        a[7] = bitmap.getPixel(start_x + w_i*2, start_y + h_i*3)
        a[8] = bitmap.getPixel(start_x + w_i*3, start_y + h_i*3)

        //添加靠近十字中线的点
        if (start_x < w/2 && start_y < h/2) {  //第一象限
            //横向靠近中线的三个点
            a[9] = bitmap.getPixel(start_x + w_i, h/2 - 3)
            a[10] = bitmap.getPixel(start_x + w_i*2, h/2 - 3)
            a[11] = bitmap.getPixel(start_x + w_i*3, h/2 - 3)
            //纵向靠近中间线的三个点
            a[12] = bitmap.getPixel(w/2 - 3, start_y + h_i)
            a[13] = bitmap.getPixel(w/2 - 3, start_y + h_i*2)
            a[14] = bitmap.getPixel(w/2 - 3, start_y + h_i*3)
            //靠近重点的点
            a[15] = bitmap.getPixel(w/2 - 3, h/2 - 3)
        } else if (start_x <= w/2 && start_y >= h/2) {  //第二象限
            //横向靠近中线的三个点
            a[9] = bitmap.getPixel(start_x + w_i, h/2 - 3)
            a[10] = bitmap.getPixel(start_x + w_i*2, h/2 - 3)
            a[11] = bitmap.getPixel(start_x + w_i*3, h/2 - 3)
            //纵向靠近中间线的三个点
            a[12] = bitmap.getPixel(w/2 + 3, start_y + h_i)
            a[13] = bitmap.getPixel(w/2 + 3, start_y + h_i*2)
            a[14] = bitmap.getPixel(w/2 + 3, start_y + h_i*3)
            //靠近重点的点
            a[15] = bitmap.getPixel(w/2 + 3, h/2 - 3)
        } else if (start_x < w/2 && start_y >= h/2) {  //第三象限
            //横向靠近中线的三个点
            a[9] = bitmap.getPixel(start_x + w_i, h/2 + 3)
            a[10] = bitmap.getPixel(start_x + w_i*2, h/2 + 3)
            a[11] = bitmap.getPixel(start_x + w_i*3, h/2 + 3)
            //纵向靠近中间线的三个点
            a[12] = bitmap.getPixel(w/2 - 3, start_y + h_i)
            a[13] = bitmap.getPixel(w/2 - 3, start_y + h_i*2)
            a[14] = bitmap.getPixel(w/2 - 3, start_y + h_i*3)
            //靠近重点的点
            a[15] = bitmap.getPixel(w/2 - 3, h/2 + 3)
        } else if (start_x <= w/2 && start_y >= h/2) {  //第四象限
            //横向靠近中线的三个点
            a[9] = bitmap.getPixel(start_x + w_i, h/2 + 3)
            a[10] = bitmap.getPixel(start_x + w_i*2, h/2 + 3)
            a[11] = bitmap.getPixel(start_x + w_i*3, h/2 + 3)
            //纵向靠近中间线的三个点
            a[12] = bitmap.getPixel(w/2 + 3, start_y + h_i)
            a[13] = bitmap.getPixel(w/2 + 3, start_y + h_i*2)
            a[14] = bitmap.getPixel(w/2 + 3, start_y + h_i*3)
            //靠近重点的点
            a[15] = bitmap.getPixel(w/2 + 3, h/2 + 3)
        }

        return a
    }

    //n*n n平均取点
    private fun getArray(bitmap: Bitmap, start_x: Int, start_y: Int, count: Int): IntArray {
        val w = bitmap.width
        val h = bitmap.height
        val w_i = w/(2*(count+1))
        val h_i = h/(2*(count+1))
        val a = IntArray(count * count)
        for(i in 0 until count) {
            for(j in 0 until count) {
                a[i*10 + j] = bitmap.getPixel(start_x + w_i * j, start_y + h_i * i)
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

//    private fun isGray(a:IntArray): Boolean {
//        //一个区域里面的识别点
//        for (c in a) {
////            val hsv = FloatArray(3)
////            //通过使用HSV颜色空间中的S通道进行判断。为此，要将rgb模式转换为hsb模式再去判断，其中：h色相，s饱和度，b对比度
////            //判断饱和度，如果s<10%即可认为是灰度图，至于这个阈值是10％还是15％
////            Color.colorToHSV(c, hsv)
////            Log.i("打印选择的值","H=${hsv[0]} ,S=${hsv[1]} ,V=${hsv[2]}")
////            if (hsv[1] in 0.3..0.99) {  //s饱和度大认为是彩色 s等于1位纯色，当纯黑或纯白的时候
////                return false
////            }
//            //获取rgb值
//            val r = Color.red(c)
//            val g = Color.green(c)
//            val b = Color.blue(c)
//            Log.i("打印选择的值","r=$r ,g=$g ,b=$b")
//            //通过rgb色值差距来判断是否灰度图
//            if ((abs(r-g) > 25 || abs(g-b) > 25 || abs(b-r) > 25)
//                && (r>30 && g>30 && b>30)) {
//                return false
//            }
//        }
//        return true
//    }

    //是否灰度区域
    private fun isGray(a:IntArray): Boolean {
        return GrayColorJudge.isGrayRegion(a)
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
