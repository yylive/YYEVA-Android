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
        return File(assetsPath)
    }
}