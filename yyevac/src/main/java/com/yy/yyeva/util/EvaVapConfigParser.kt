package com.yy.yyeva.util

import com.yy.yyeva.file.IEvaFileContainer
import org.json.JSONArray
import org.json.JSONObject
import java.io.ByteArrayOutputStream

/**
 * VAP 格式配置解析器。
 *
 * VAP 把配置以 JSON 形式存放在 mp4 的 vapc box 中（box 结构: 4字节size + 4字节type 'vapc' + JSON数据），
 * 字段结构与 YYEVA 不同（info/src/frame vs descript/effect/datas）。
 *
 * 这里负责从 mp4 中解析出 vapc box 的 JSON，并转换为 YYEVA 格式，
 * 让后续 [com.yy.yyeva.EvaAnimConfig] / native EvaAnimeConfig 复用同一套解析逻辑。
 */
object EvaVapConfigParser {

    private const val TAG = "${EvaConstant.TAG}.EvaVapConfigParser"
    private const val BOX_TYPE_VAPC = "vapc"

    /** 可能包含子 box 的容器 box，vapc 有可能嵌套其中，递归查找 */
    private val CONTAINER_BOX_TYPES = setOf(
        "moov", "trak", "mdia", "minf", "udta", "stbl", "moof", "traf", "edts"
    )

    /**
     * 从 mp4 的 vapc box 中解析 VAP 配置，并转换为 YYEVA 格式的 JSONObject。
     * @return YYEVA 格式 JSONObject；解析失败返回 null
     */
    fun parseVapc(evaFileContainer: IEvaFileContainer): JSONObject? {
        return try {
            val bytes = readAllBytes(evaFileContainer) ?: return null
            if (bytes.isEmpty()) return null
            val vapcData = findVapcBox(bytes) ?: run {
                ELog.i(TAG, "vapc box not found")
                return null
            }
            val vapConfig = JSONObject(String(vapcData, Charsets.UTF_8))
            convertVapToYYEVA(vapConfig)
        } catch (e: Exception) {
            ELog.e(TAG, "parseVapc error $e", e)
            null
        }
    }

    /** 通过容器接口读取整个文件内容（vapc 解析需要随机访问 box） */
    private fun readAllBytes(container: IEvaFileContainer): ByteArray? {
        return try {
            container.startRandomRead()
            val buffer = ByteArray(8192)
            val output = ByteArrayOutputStream()
            var read: Int
            while (container.read(buffer, 0, buffer.size).also { read = it } > 0) {
                output.write(buffer, 0, read)
            }
            output.toByteArray()
        } catch (e: Exception) {
            ELog.e(TAG, "readAllBytes error $e", e)
            null
        } finally {
            try { container.closeRandomRead() } catch (e: Exception) { /* ignore */ }
        }
    }

    /**
     * 遍历 mp4 box 层级查找 vapc box，返回其数据部分（JSON）。
     * box 结构: 4字节size(大端) + 4字节type + data...，size==1 时紧跟8字节64位size，size==0 时延伸到文件末尾。
     */
    private fun findVapcBox(data: ByteArray): ByteArray? {
        var offset = 0
        while (offset + 8 <= data.size) {
            val boxSize = readUInt32BE(data, offset)
            if (boxSize < 8 && boxSize != 0 && boxSize != 1) break

            val boxType = String(data, offset + 4, 4, Charsets.ISO_8859_1)

            var headerSize = 8
            var actualSize = boxSize.toLong()
            when {
                boxSize == 1 -> { // 64位size
                    if (offset + 16 > data.size) break
                    actualSize = readUInt64BE(data, offset + 8)
                    headerSize = 16
                }
                boxSize == 0 -> { // 延伸到文件末尾
                    actualSize = (data.size - offset).toLong()
                }
            }

            if (boxType == BOX_TYPE_VAPC) {
                val dataOffset = offset + headerSize
                val dataLength = (actualSize - headerSize).toInt().coerceIn(0, data.size - dataOffset)
                if (dataLength > 0) {
                    return data.copyOfRange(dataOffset, dataOffset + dataLength)
                }
                return null
            }

            // 递归查找容器 box，vapc 可能嵌套其中
            if (boxType in CONTAINER_BOX_TYPES && actualSize > headerSize && actualSize < Int.MAX_VALUE) {
                val subEnd = (offset + actualSize).toInt().coerceAtMost(data.size)
                val subData = data.copyOfRange(offset + headerSize, subEnd)
                findVapcBox(subData)?.let { return it }
            }

            if (actualSize <= 0) break
            val nextOffset = offset + actualSize.toInt()
            if (nextOffset <= offset) break  // 防止异常size导致死循环
            offset = nextOffset
        }
        return null
    }

    private fun readUInt32BE(data: ByteArray, offset: Int): Int {
        return ((data[offset].toInt() and 0xff) shl 24) or
                ((data[offset + 1].toInt() and 0xff) shl 16) or
                ((data[offset + 2].toInt() and 0xff) shl 8) or
                (data[offset + 3].toInt() and 0xff)
    }

    private fun readUInt64BE(data: ByteArray, offset: Int): Long {
        var v = 0L
        for (i in 0 until 8) {
            v = (v shl 8) or (data[offset + i].toLong() and 0xff)
        }
        return v
    }

    /**
     * 将 VAP 格式转换为 YYEVA 格式。
     *
     * VAP:                       YYEVA:
     *   info.videoW/videoH   ->    descript.width/height
     *   info.rgbFrame        ->    descript.rgbFrame
     *   info.aFrame          ->    descript.alphaFrame
     *   info.isVapx          ->    descript.isEffect
     *   src[]                ->    effect[]  (srcId 数组索引 -> effectId)
     *   frame[].obj[]        ->    datas[].data[]  (按 srcId 索引匹配 effectId)
     */
    private fun convertVapToYYEVA(vapConfig: JSONObject): JSONObject {
        val info = vapConfig.optJSONObject("info") ?: vapConfig

        val w = info.optInt("w")
        val h = info.optInt("h")
        val videoW = info.optInt("videoW")
        val videoH = info.optInt("videoH")
        val isVapx = info.optInt("isVapx")
        val fps = info.optInt("fps")
        val rgbFrame = info.optJSONArray("rgbFrame")
        val alphaFrame = info.optJSONArray("aFrame")

        val yyeva = JSONObject()

        // descript
        val descript = JSONObject()
        descript.put("width", if (videoW > 0) videoW else w)
        descript.put("height", if (videoH > 0) videoH else h)
        descript.put("isEffect", isVapx == 1)
        descript.put("version", info.optInt("v"))
        if (fps > 0) descript.put("fps", fps)
        descript.put("rgbFrame", if (rgbFrame != null && rgbFrame.length() >= 4) rgbFrame else intArray(0, 0, w, h))
        descript.put("alphaFrame", if (alphaFrame != null && alphaFrame.length() >= 4) alphaFrame else intArray(0, 0, w, h))
        yyeva.put("descript", descript)

        // effect: src -> effect，用 src 数组索引作为 effectId
        val sources = vapConfig.optJSONArray("src")
        if (sources != null) {
            val effects = JSONArray()
            for (i in 0 until sources.length()) {
                val src = sources.optJSONObject(i) ?: continue
                val effect = JSONObject()
                effect.put("effectId", i)  // 用数组索引，保证与 data.effectId 一致
                effect.put("effectTag", src.optString("srcTag"))
                effect.put("effectType", src.optString("srcType"))
                effect.put("effectWidth", src.optInt("w"))
                effect.put("effectHeight", src.optInt("h"))
                val color = src.optString("color")
                if (color.isNotEmpty()) effect.put("fontColor", color)
                val fitType = src.optString("fitType")
                if (fitType.isNotEmpty()) effect.put("scaleMode", fitType)
                effects.put(effect)
            }
            yyeva.put("effect", effects)
        }

        // datas: frame -> datas
        val frames = vapConfig.optJSONArray("frame")
        if (frames != null) {
            val datas = JSONArray()
            for (i in 0 until frames.length()) {
                val frame = frames.optJSONObject(i) ?: continue
                val frameIndex = frame.optInt("i")
                val objs = frame.optJSONArray("obj") ?: continue
                val dataList = JSONArray()
                for (j in 0 until objs.length()) {
                    val obj = objs.optJSONObject(j) ?: continue
                    val srcIndex = findSrcIndexBySrcId(obj.optString("srcId"), sources)
                    val data = JSONObject()
                    data.put("effectId", srcIndex)
                    obj.optJSONArray("frame")?.let { if (it.length() >= 4) data.put("renderFrame", it) }
                    obj.optJSONArray("mFrame")?.let { if (it.length() >= 4) data.put("outputFrame", it) }
                    dataList.put(data)
                }
                val dataObj = JSONObject()
                dataObj.put("frameIndex", frameIndex)
                dataObj.put("data", dataList)
                datas.put(dataObj)
            }
            yyeva.put("datas", datas)
        }

        return yyeva
    }

    /** 按 VAP 的 srcId 字符串查找其在 src 数组中的索引，用作 effectId */
    private fun findSrcIndexBySrcId(srcId: String, sources: JSONArray?): Int {
        if (sources == null || srcId.isEmpty()) return 0
        for (i in 0 until sources.length()) {
            val src = sources.optJSONObject(i) ?: continue
            if (src.optString("srcId") == srcId) return i
        }
        return 0
    }

    private fun intArray(vararg values: Int): JSONArray {
        val arr = JSONArray()
        for (v in values) arr.put(v)
        return arr
    }
}
