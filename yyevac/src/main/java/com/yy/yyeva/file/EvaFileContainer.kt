package com.yy.yyeva.file

import android.media.MediaExtractor
import com.yy.yyeva.util.EvaConstant
import com.yy.yyeva.util.ELog
import java.io.File
import java.io.FileNotFoundException
import java.io.RandomAccessFile

class EvaFileContainer(private val file: File) : IEvaFileContainer {

    companion object {
        private const val TAG = "${EvaConstant.TAG}.EvaFileContainer"
    }

    private var randomAccessFile: RandomAccessFile? = null
    private var md5 = ""

    init {
        ELog.i(TAG, "FileContainer init")
        if (!(file.exists() && file.isFile && file.canRead())) throw FileNotFoundException("Unable to read $file")
    }

    override fun setDataSource(extractor: MediaExtractor) {
        extractor.setDataSource(file.toString())
    }

    override fun startRandomRead() {
        randomAccessFile = RandomAccessFile(file, "r")
    }

    override fun read(b: ByteArray, off: Int, len: Int): Int {
        return randomAccessFile?.read(b, off, len) ?: -1
    }

    override fun skip(pos: Long) {
        randomAccessFile?.skipBytes(pos.toInt())
    }

    override fun closeRandomRead() {
        randomAccessFile?.close()
    }

    override fun close() {
    }

    override fun getFile(): File {
        return file
    }

    override fun getMd5(): String {
        if (md5.isEmpty()) {
            md5 = FileUtil.getFileMD5(file)?: ""
        }
        return md5
    }

    override fun setEvaJson(json: String) {
        EvaPref.setEvaJson(file.name, getMd5(), json)
    }

    override fun getEvaJson(): String? {
        return EvaPref.getEvaJson(getMd5())
    }

    override fun setEvaMp4Type(type: Int) {
        EvaPref.setEvaMp4Type(file.name, getMd5(), type)
    }

    override fun getEvaMp4Type(): Int {
        return EvaPref.getEvaMp4Type(getMd5())
    }
}