package com.yy.yyeva

import android.os.SystemClock
import android.util.Base64
import com.yy.yyeva.file.IEvaFileContainer
import com.yy.yyeva.util.EvaConstant
import com.yy.yyeva.util.ELog
import com.yy.yyeva.util.PointRect
import org.json.JSONObject
import java.io.ByteArrayOutputStream
import java.io.IOException
import java.lang.Exception
import java.util.zip.Inflater


/**
 * 配置管理
 */
class EvaAnimConfigManager(var playerEva: EvaAnimPlayer){

    var config: EvaAnimConfig? = null
    var isParsingConfig = false // 是否正在读取配置

    companion object {
        private const val TAG = "${EvaConstant.TAG}.EvaAnimConfigManager"
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
            if (config?.isDefaultConfig == true && !enableVersion1) {
                isParsingConfig = false
                return EvaConstant.REPORT_ERROR_TYPE_PARSE_CONFIG
            }
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

        evaFileContainer.startRandomRead()
        val readBytes = ByteArray(512)
        var readBytesLast = ByteArray(512)
        var bufStr = ""
        var bufStrS = ""
        val matchStart = "yyeffectmp4json[["
        val matchEnd = "]]yyeffectmp4json"
        var findStart = false
        var findEnd = false
        var jsonStr = ""
        while (evaFileContainer.read(readBytes, 0, readBytes.size) > 0) {
            if (!findStart) { //没找到开头
                bufStr = String(readBytes)
                val index = bufStr.indexOf(matchStart)
                if (index > 0) { //分段1找到匹配开头
                    jsonStr = bufStr.substring(index + matchStart.length)
                    findStart = true
                } else {
                    if (readBytesLast.isNotEmpty()) {
                        bufStrS = String(readBytes + readBytesLast)
                        val indexS = bufStrS.indexOf(matchStart)
                        if (indexS > 0) { //合并分段找到匹配开头
                            jsonStr = bufStrS.substring(indexS + matchStart.length)
                            findStart = true
                        }
                    }
                    //保存分段
                    readBytesLast = readBytes
                }
            } else {
                bufStr = String(readBytes)
                val index = bufStr.indexOf(matchEnd)
                if (index > 0) { //分段1找到匹配结尾
                    jsonStr += bufStr.substring(0, index)
                    findEnd = true
                    break
                } else {
                    if (readBytesLast.isNotEmpty()) {
                        bufStrS = String(readBytes + readBytesLast)
                        val indexS = bufStrS.indexOf(matchEnd)
                        if (indexS > 0) { //合并分段找到匹配结尾
                            jsonStr = jsonStr.substring(0, jsonStr.length - (indexS - readBytesLast.size) - 1)
                            findEnd = true
                            break
                        }
                    }
                    //保存数据
                    jsonStr += bufStr
                    //保存分段
                    readBytesLast = readBytes
                }
            }
        }

        evaFileContainer.closeRandomRead()

        if (!findStart || !findEnd) {
            ELog.e(TAG, "yyeffectmp4json not found")
            // 按照默认配置生成config
            config?.apply {
                isDefaultConfig = true
                this.defaultVideoMode = defaultVideoMode
                fps = defaultFps
            }
            playerEva.fps = config.fps
            return true
        } else {
            //先用base64解密，再用zlib解密
            jsonStr = zlib(Base64.decode(jsonStr.toByteArray(), Base64.DEFAULT)).decodeToString()
            ELog.d(TAG, "jsonStr:$jsonStr")
        }

        val jsonObj = JSONObject(jsonStr)
        config.jsonConfig = jsonObj
        val result = config!!.parse(jsonObj)
        if (defaultFps > 0) {
            config.fps = defaultFps
        }
        playerEva.fps = config.fps
        return result
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
}