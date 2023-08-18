package com.yy.yyeva

import android.R.attr
import android.graphics.Bitmap
import android.graphics.Color
import android.media.MediaMetadataRetriever
import android.os.SystemClock
import android.util.Base64
import android.util.Log
import com.yy.yyeva.file.IEvaFileContainer
import com.yy.yyeva.util.EvaConstant
import com.yy.yyeva.util.ELog
import com.yy.yyeva.util.PointRect
import org.json.JSONObject
import java.io.ByteArrayOutputStream
import java.io.File
import java.io.IOException
import java.lang.Exception
import java.util.zip.Inflater
import kotlin.math.abs


/**
 * 配置管理
 */
class EvaAnimConfigManager(var playerEva: EvaAnimPlayer){

    var config: EvaAnimConfig? = null
    var isParsingConfig = false // 是否正在读取配置
    private var audioSpeed = 1.0f
    private var startDetect = 0L

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
            config.apply {
                isDefaultConfig = true
                this.defaultVideoMode = EvaConstant.VIDEO_MODE_NORMAL_MP4
                fps = defaultFps
            }
            playerEva.fps = config.fps
            return true
        }

        evaFileContainer.startRandomRead()
        val readBytes = ByteArray(1024)
        var readBytesLast = ByteArray(1024)
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
                var index = bufStr.indexOf(matchStart)
                if (index > 0) { //分段1找到匹配开头
                    jsonStr = bufStr.substring(index + matchStart.length)
                    findStart = true
                    index = jsonStr.indexOf(matchEnd)
                    if (index > 0) { //同时包含结尾段进行截取
                        findEnd = true
                        jsonStr = jsonStr.substring(0, index)
                        break
                    }
                } else {
                    if (readBytesLast.isNotEmpty()) {
                        bufStrS = String(readBytes + readBytesLast)
                        var indexS = bufStrS.indexOf(matchStart)
                        if (indexS > 0) { //合并分段找到匹配开头
                            jsonStr = bufStrS.substring(indexS + matchStart.length)
                            findStart = true
                            indexS = jsonStr.indexOf(matchEnd)
                            if (indexS > 0) { // 同时包含结尾段进行截取
                                findEnd = true
                                jsonStr = jsonStr.substring(0, indexS)
                                break
                            }
                        }
                    }
                    //保存分段
                    readBytesLast = readBytes.clone()
                }
            } else {
                bufStr = String(readBytes)
                val index = bufStr.indexOf(matchEnd)
                if (index > 0) { //分段1找到匹配结尾
                    jsonStr += bufStr.substring(0, index)
                    findEnd = true
                    break
                } else if(!readBytesLast.contentEquals(readBytes)) { //判定内容不一致
                    if (readBytesLast.isNotEmpty()) {
                        bufStrS = String(readBytesLast + readBytes)
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
                    readBytesLast = readBytes.clone()
                }
            }
        }

        evaFileContainer.closeRandomRead()

        if (!findStart || !findEnd) {
            ELog.e(TAG, "yyeffectmp4json not found")
            if (playerEva.videoMode == EvaConstant.VIDEO_MODE_NORMAL_MP4) {// 没有设置,默认为正常mp4
                getMp4Type(evaFileContainer.getFile())
            }
            // 按照默认配置生成config
            config?.apply {
                isDefaultConfig = true
                if (playerEva.videoMode == EvaConstant.VIDEO_MODE_NORMAL_MP4) {
                    playerEva.isNormalMp4 = true //设定为正常mp4
                }
                this.defaultVideoMode = playerEva.videoMode
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
        if (config.fps == 0) {
            config.fps = defaultFps
        }
        playerEva.fps = config.fps
        return result
    }

    fun setAudioSpeed(speed: Float) {
        audioSpeed = speed
        playerEva.audioSpeed = audioSpeed
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

    fun getMp4Type(file: File?) {
        if(file != null && file.exists()) {
            val mmr = MediaMetadataRetriever()
            mmr.setDataSource(file.absolutePath)
            //获取播放时长
            val duration = mmr.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION)?.toLong()
            if (duration != null && duration > 0) {
                for(i in 1..6) {
                    startDetect = System.currentTimeMillis()
                    val bitmap =
                        mmr.getFrameAtTime(i* duration/6 * 1000, MediaMetadataRetriever.OPTION_CLOSEST)
                    val isJudge = getConfigManager(bitmap)
                    bitmap?.recycle()
                    Log.i(TAG, "detect image mp4Type ${System.currentTimeMillis() - startDetect}")
                    if (isJudge) {
                        break
                    }
                }
            }
            mmr.release()
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
        //一个区域里面的识别点
        for (c in a) {
            //获取rgb值
            val r = Color.red(c)
            val g = Color.green(c)
            val b = Color.blue(c)
            Log.i("打印选择的值","r=$r ,g=$g ,b=$b")
            //通过rgb色值差距来判断是否灰度图
            if ((abs(r-g) > 25 || abs(g-b) > 25 || abs(b-r) > 25)
                && (r>30 && g>30 && b>30)) {
                return false
            }
        }
        return true
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