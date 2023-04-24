package com.yy.yyeva.file

import android.media.MediaExtractor
import java.io.File

interface IEvaFileContainer {

    fun setDataSource(extractor: MediaExtractor)

    fun startRandomRead()

    fun read(b: ByteArray, off: Int, len: Int): Int

    fun skip(pos: Long)

    fun closeRandomRead()

    fun close()

    fun getFile(): File?


}