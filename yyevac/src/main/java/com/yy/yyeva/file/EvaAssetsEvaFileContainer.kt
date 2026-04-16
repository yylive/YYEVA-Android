package com.yy.yyeva.file

import android.content.res.AssetFileDescriptor
import android.content.res.AssetManager
import android.media.MediaExtractor
import com.yy.yyeva.util.EvaConstant
import com.yy.yyeva.util.ELog
import java.io.File
import java.io.IOException

class EvaAssetsEvaFileContainer(private val assetManager: AssetManager, val assetsPath: String): IEvaFileContainer {

    companion object {
        private const val TAG = "${EvaConstant.TAG}.FileContainer"
    }

    val assetFd: AssetFileDescriptor = assetManager.openFd(assetsPath)

    // JSON 扫描流：每次 startRandomRead() 重新打开，保证从头扫描
    private var scanInputStream: AssetManager.AssetInputStream? = null

    private val fileName = assetsPath.substringAfterLast('/')
    private var md5 = ""

    init {
        ELog.i(TAG, "AssetsFileContainer init")
    }

    override fun setDataSource(extractor: MediaExtractor) {
        if (assetFd.declaredLength < 0) {
            extractor.setDataSource(assetFd.fileDescriptor)
        } else {
            extractor.setDataSource(assetFd.fileDescriptor, assetFd.startOffset, assetFd.declaredLength)
        }
    }

    // 每次重新打开流，确保 JSON 扫描始终从文件头开始
    override fun startRandomRead() {
        try { scanInputStream?.close() } catch (e: IOException) { /* ignore */ }
        scanInputStream = assetManager.open(assetsPath, AssetManager.ACCESS_STREAMING)
                as AssetManager.AssetInputStream
    }

    override fun read(b: ByteArray, off: Int, len: Int): Int {
        return scanInputStream?.read(b, off, len) ?: -1
    }

    override fun skip(pos: Long) {
        // InputStream.skip() 不保证跳过指定字节数，用读取循环替代
        var remaining = pos
        val buf = ByteArray(4096)
        while (remaining > 0) {
            val n = scanInputStream?.read(buf, 0, minOf(remaining, buf.size.toLong()).toInt()) ?: break
            if (n < 0) break
            remaining -= n
        }
    }

    override fun closeRandomRead() {
        try { scanInputStream?.close() } catch (e: IOException) { /* ignore */ }
        scanInputStream = null
    }

    override fun close() {
        try { assetFd.close() } catch (e: IOException) { /* ignore */ }
        // scanInputStream 由 closeRandomRead() 管理，不在此处重复关闭
    }

    override fun getFile(): File? {
        return null
    }

    override fun getMd5(): String {
        if (md5.isEmpty()) {
            md5 = try {
                FileUtil.getStreamMD5(assetManager.open(assetsPath)) ?: ""
            } catch (e: Exception) {
                ""
            }
        }
        return md5
    }

    override fun setEvaJson(json: String) {
        EvaPref.setEvaJson(fileName, getMd5(), json)
    }

    override fun getEvaJson(): String? {
        return EvaPref.getEvaJson(fileName, getMd5())
    }

    override fun setEvaMp4Type(type: Int) {
        EvaPref.setEvaMp4Type(fileName, getMd5(), type)
    }

    override fun getEvaMp4Type(): Int {
        return EvaPref.getEvaMp4Type(fileName, getMd5())
    }
}