package com.yy.yyeva.file

import android.annotation.TargetApi
import android.media.MediaExtractor
import android.os.Build
import java.io.ByteArrayInputStream
import java.io.File

@TargetApi(Build.VERSION_CODES.M)
class EvaStreamContainerEva(private val bytes: ByteArray) : IEvaFileContainer {

    private var stream: ByteArrayInputStream = ByteArrayInputStream(bytes)

    override fun setDataSource(extractor: MediaExtractor) {
        val dataSource = EvaStreamMediaDataSource(bytes)
        extractor.setDataSource(dataSource)
    }

    override fun startRandomRead() {
    }

    override fun read(b: ByteArray, off: Int, len: Int): Int {
        return stream.read(b, off, len)
    }

    override fun skip(pos: Long) {
        stream.skip(pos)
    }

    override fun closeRandomRead() {
    }

    override fun close() {
        stream.close()
    }

    override fun getFile(): File? {
        return null
    }
}
