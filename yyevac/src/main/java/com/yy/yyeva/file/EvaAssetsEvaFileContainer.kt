package com.yy.yyeva.file

import android.content.res.AssetFileDescriptor
import android.content.res.AssetManager
import android.media.MediaExtractor
import com.yy.yyeva.util.EvaConstant
import com.yy.yyeva.util.ELog
import java.io.File

class EvaAssetsEvaFileContainer(assetManager: AssetManager, val assetsPath: String): IEvaFileContainer {

    companion object {
        private const val TAG = "${EvaConstant.TAG}.FileContainer"
    }

    private val assetFd: AssetFileDescriptor = assetManager.openFd(assetsPath)
    private val assetsInputStream: AssetManager.AssetInputStream =
        assetManager.open(assetsPath, AssetManager.ACCESS_STREAMING) as AssetManager.AssetInputStream
    var f: File
    private var md5 = ""

    init {
        ELog.i(TAG, "AssetsFileContainer init")
        f = File(assetsPath)
    }

    override fun setDataSource(extractor: MediaExtractor) {
        if (assetFd.declaredLength < 0) {
            extractor.setDataSource(assetFd.fileDescriptor)
        } else {
            extractor.setDataSource(assetFd.fileDescriptor, assetFd.startOffset, assetFd.declaredLength)
        }
    }

    override fun startRandomRead() {
    }

    override fun read(b: ByteArray, off: Int, len: Int): Int {
        return assetsInputStream.read(b, off, len)
    }

    override fun skip(pos: Long) {
        assetsInputStream.skip(pos)
    }

    override fun closeRandomRead() {
        assetsInputStream.close()
    }

    override fun close() {
        assetFd.close()
        assetsInputStream.close()
    }

    override fun getFile(): File {
        return f
    }

    override fun getMd5(): String {
        if (md5.isEmpty()) {
            md5 = FileUtil.getFileMD5(f)?: ""
        }
        return md5
    }

    override fun setEvaJson(json: String) {
        EvaPref.setEvaJson(f.name, getMd5(), json)
    }

    override fun getEvaJson(): String? {
        return EvaPref.getEvaJson(getMd5())
    }

    override fun setEvaMp4Type(type: Int) {
        EvaPref.setEvaMp4Type(f.name, getMd5(), type)
    }

    override fun getEvaMp4Type(): Int {
        return EvaPref.getEvaMp4Type(getMd5())
    }
}