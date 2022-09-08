package com.yy.yyeva.util

import android.content.Context
import android.os.Environment
import java.io.File
import java.net.URL
import java.security.MessageDigest

/**
 * EVA 缓存管理
 */
object EvaCache {
    enum class Type {
        DEFAULT,
        FILE
    }

    private const val TAG = "EvaCache"
    private var type: Type = Type.DEFAULT

    private var cacheDir: String = "/"
        get() {
            if (field != "/") {
                val dir = File(field)
                if (!dir.exists()) {
                    dir.mkdirs()
                }
            }
            return field
        }


    fun onCreate(context: Context?) {
        onCreate(context, Type.DEFAULT)
    }

    fun onCreate(context: Context?, type: Type) {
        if (isInitialized()) return
        context ?: return
//        cacheDir = "${context.cacheDir.absolutePath}/eva/"
        cacheDir = context.getExternalFilesDir(null)?.absolutePath ?: Environment.getExternalStorageDirectory().path
        cacheDir = "$cacheDir/"
        File(cacheDir).takeIf { !it.exists() }?.mkdirs()
        this.type = type
    }

    /**
     * 清理缓存
     */
    fun clearCache() {
        if (!isInitialized()) {
            ELog.e(TAG, "EVACache is not init!")
            return
        }
        EvaDownloader.threadPoolExecutor.execute {
            clearDir(cacheDir)
            ELog.i(TAG, "Clear eva cache done!")
        }
    }

    // 清除目录下的所有文件
    internal fun clearDir(path: String) {
        try {
            val dir = File(path)
            dir.takeIf { it.exists() }?.let { parentDir ->
                parentDir.listFiles()?.forEach { file ->
                    if (!file.exists()) {
                        return@forEach
                    }
                    if (file.isDirectory) {
                        clearDir(file.absolutePath)
                    }
                    file.delete()
                }
            }
        } catch (e: Exception) {
            ELog.e(TAG, "Clear eva cache path: $path fail", e)
        }
    }

    fun isInitialized(): Boolean {
        return "/" != cacheDir && File(cacheDir).exists()
    }

    fun isDefaultCache(): Boolean = type == Type.DEFAULT

    fun isCached(cacheKey: String): Boolean {
        return if (isDefaultCache()) {
            buildCacheDir(cacheKey)
        } else {
            buildMp4File(
                cacheKey
            )
        }.exists()
    }

    fun buildCacheKey(str: String): String {
        val messageDigest = MessageDigest.getInstance("MD5")
        messageDigest.update(str.toByteArray(charset("UTF-8")))
        val digest = messageDigest.digest()
        var sb = ""
        for (b in digest) {
            sb += String.format("%02x", b)
        }
        return sb
    }

    fun buildCacheKey(url: URL): String = buildCacheKey(url.toString())

    fun buildCacheDir(cacheKey: String): File {
        return File("$cacheDir$cacheKey/")
    }

    fun buildMp4File(cacheKey: String): File {
        return File("$cacheDir$cacheKey.mp4")
    }

    fun buildAudioFile(audio: String): File {
        return File("$cacheDir$audio.mp3")
    }

}