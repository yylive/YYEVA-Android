package com.yy.yyeva.util

import android.content.Context
import android.os.Handler
import android.os.Looper
import java.io.*
import java.net.HttpURLConnection
import java.net.URL
import java.util.concurrent.Executors
import java.util.concurrent.ThreadPoolExecutor
import java.util.concurrent.atomic.AtomicInteger
import java.util.zip.Inflater
import java.util.zip.ZipInputStream

/**
 * Created by PonyCui 16/6/18.
 */
private var fileLock: Int = 0
private var isUnzipping = false

class EvaDownloader(context: Context?) {
    private var mContext = context?.applicationContext

    init {
        EvaCache.onCreate(context, EvaCache.Type.FILE)
    }

    @Volatile
    private var mFrameWidth: Int = 0

    @Volatile
    private var mFrameHeight: Int = 0

    interface ParseCompletion {
        fun onComplete(videoItem: EvaVideoEntity)
        fun onError()
    }

    interface PlayCallback{
        fun onPlay(file: List<File>)
    }

    open class FileDownloader {

        var noCache = false

        open fun resume(url: URL, complete: (inputStream: InputStream) -> Unit, failure: (e: Exception) -> Unit): () -> Unit {
            var cancelled = false
            val cancelBlock = {
                cancelled = true
            }
            threadPoolExecutor.execute {
                try {
                    ELog.i(TAG, "================ eva file download start ================")
//                    if (HttpResponseCache.getInstalled() == null && !noCache) {
//                        ELog.e(TAG, "EvaDownloader can not handle cache before install HttpResponseCache. see https://github.com/yyued/SVGAPlayer-Android#cache")
//                        ELog.e(TAG, "在配置 HttpResponseCache 前 EvaDownloader 无法缓存. 查看 https://github.com/yyued/SVGAPlayer-Android#cache ")
//                    }
                    (url.openConnection() as? HttpURLConnection)?.let {
                        it.connectTimeout = 20 * 1000
                        it.requestMethod = "GET"
                        it.setRequestProperty("Connection", "close")
                        it.connect()
                        it.inputStream.use { inputStream ->
                            ByteArrayOutputStream().use { outputStream ->
                                val buffer = ByteArray(4096)
                                var count: Int
                                while (true) {
                                    if (cancelled) {
                                        ELog.i(TAG, "================ eva file download canceled ================")
                                        break
                                    }
                                    count = inputStream.read(buffer, 0, 4096)
                                    if (count == -1) {
                                        break
                                    }
                                    outputStream.write(buffer, 0, count)
                                }
                                if (cancelled) {
                                    ELog.i(TAG, "================ eva file download canceled ================")
                                    return@execute
                                }
                                ByteArrayInputStream(outputStream.toByteArray()).use {
                                    ELog.i(TAG, "================ eva file download complete ================")
                                    complete(it)
                                }
                            }
                        }
                    }
                } catch (e: Exception) {
                    ELog.e(TAG, "================ eva file download fail ================")
                    ELog.e(TAG, "error: ${e.message}")
                    e.printStackTrace()
                    failure(e)
                }
            }
            return cancelBlock
        }
    }

    var fileDownloader = FileDownloader()

    companion object {
        private const val TAG = "EvaDownloader"

        private val threadNum = AtomicInteger(0)
        private var mShareParser = EvaDownloader(null)

        internal var threadPoolExecutor = Executors.newCachedThreadPool { r ->
            Thread(r, "EvaDownloader-Thread-${threadNum.getAndIncrement()}")
        }

        fun setThreadPoolExecutor(executor: ThreadPoolExecutor) {
            threadPoolExecutor = executor
        }

        fun shareParser(): EvaDownloader {
            return mShareParser
        }
    }

    fun init(context: Context) {
        mContext = context.applicationContext
        EvaCache.onCreate(mContext)
    }

    fun setFrameSize(frameWidth: Int, frameHeight: Int) {
        mFrameWidth = frameWidth
        mFrameHeight = frameHeight
    }

    fun decodeFromAssets(
        name: String,
        callback: ParseCompletion?,
        playCallback: PlayCallback? = null
    ) {
        if (mContext == null) {
            ELog.e(TAG, "在配置 EvaDownloader context 前, 无法解析 EVA 文件。")
            return
        }
        ELog.i(TAG, "================ decode $name from assets ================")
        threadPoolExecutor.execute {
            try {
                mContext?.assets?.open(name)?.let {
                    this.decodeFromInputStream(
                        it,
                        EvaCache.buildCacheKey("file:///assets/$name"),
                        callback,
                        true,
                        playCallback,
                        alias = name
                    )
                }
            } catch (e: Exception) {
                this.invokeErrorCallback(e, callback, name)
            }
        }

    }

    fun decodeFromURL(
        url: URL,
        callback: ParseCompletion?,
        playCallback: PlayCallback? = null
    ): (() -> Unit)? {
        if (mContext == null) {
            ELog.e(TAG, "在配置 EvaDownloader context 前, 无法解析 EVA 文件。")
            return null
        }
        val urlPath = url.toString()
        ELog.i(TAG, "================ decode from url: $urlPath ================")
        val cacheKey = EvaCache.buildCacheKey(url)
        return if (EvaCache.isCached(cacheKey)) {
            ELog.i(TAG, "this url cached")
            threadPoolExecutor.execute {
                if (EvaCache.isDefaultCache()) {
                    this.decodeFromCacheKey(cacheKey, callback, alias = urlPath)
                } else {
                    this.decodeFromEvaFileCacheKey(cacheKey, callback, playCallback, alias = urlPath)
                }
            }
            return null
        } else {
            ELog.i(TAG, "no cached, prepare to download")
            fileDownloader.resume(url, {
                this.decodeFromInputStream(
                    it,
                    cacheKey,
                    callback,
                    false,
                    playCallback,
                    alias = urlPath
                )
            }, {
                ELog.e(
                    TAG,
                    "================ EVA file: $url download fail ================"
                )
                this.invokeErrorCallback(it, callback, alias = urlPath)
            })
        }
    }

    /**
     * 读取解析本地缓存的 eva 文件.
     */
    fun decodeFromEvaFileCacheKey(
        cacheKey: String,
        callback: ParseCompletion?,
        playCallback: PlayCallback?,
        alias: String? = null
    ) {
        threadPoolExecutor.execute {
            try {
                ELog.i(TAG, "================ decode $alias from eva cachel file to entity ================")
                FileInputStream(EvaCache.buildMp4File(cacheKey)).use { inputStream ->
                    readAsBytes(inputStream)?.let { bytes ->
                        if (isZipFile(bytes)) {
                            this.decodeFromCacheKey(cacheKey, callback, alias)
                        } else {
                            ELog.i(TAG, "inflate start")
                            inflate(bytes)?.let {
                                ELog.i(TAG, "inflate complete")
                                val videoItem = EvaVideoEntity(
                                    EvaCache.buildMp4File(cacheKey),
                                    mFrameWidth,
                                    mFrameHeight
                                )
                                callback?.onComplete(videoItem)
//                                ELog.i(TAG, "EvaVideoEntity prepare start")
//                                videoItem.prepare({
//                                    ELog.i(TAG, "EvaVideoEntity prepare success")
//                                    this.invokeCompleteCallback(videoItem, callback, alias)
//                                },playCallback)

                            } ?: this.invokeErrorCallback(
                                Exception("inflate(bytes) cause exception"),
                                callback,
                                alias
                            )
                        }
                    } ?: this.invokeErrorCallback(
                        Exception("readAsBytes(inputStream) cause exception"),
                        callback,
                        alias
                    )
                }
            } catch (e: java.lang.Exception) {
                this.invokeErrorCallback(e, callback, alias)
            } finally {
                ELog.i(TAG, "================ decode $alias from eva cachel file to entity end ================")
            }
        }
    }

    fun decodeFromInputStream(
        inputStream: InputStream,
        cacheKey: String,
        callback: ParseCompletion?,
        closeInputStream: Boolean = false,
        playCallback: PlayCallback? = null,
        alias: String? = null
    ) {
        if (mContext == null) {
            ELog.e(TAG, "在配置 EvaDownloader context 前, 无法解析 EVA 文件。")
            return
        }
        ELog.i(TAG, "================ decode $alias from input stream ================")
        threadPoolExecutor.execute {
            try {
                readAsBytes(inputStream)?.let { bytes ->
                    if (isZipFile(bytes)) {
                        ELog.i(TAG, "decode from zip file")
                        if (!EvaCache.buildCacheDir(cacheKey).exists() || isUnzipping) {
                            synchronized(fileLock) {
                                if (!EvaCache.buildCacheDir(cacheKey).exists()) {
                                    isUnzipping = true
                                    ELog.i(TAG, "no cached, prepare to unzip")
                                    ByteArrayInputStream(bytes).use {
                                        unzip(it, cacheKey)
                                        isUnzipping = false
                                        ELog.i(TAG, "unzip success")
                                    }
                                }
                            }
                        }
                        this.decodeFromCacheKey(cacheKey, callback, alias)
                    } else {
                        if (!EvaCache.isDefaultCache()) {
                            // 如果 EvaCache 设置类型为 FILE
                            threadPoolExecutor.execute {
                                EvaCache.buildMp4File(cacheKey).let { cacheFile ->
                                    try {
                                        cacheFile.takeIf { !it.exists() }?.createNewFile()
                                        FileOutputStream(cacheFile).write(bytes)
                                        val videoItem = EvaVideoEntity(
                                            EvaCache.buildMp4File(cacheKey),
                                            mFrameWidth,
                                            mFrameHeight
                                        )
                                        callback?.onComplete(videoItem)
                                    } catch (e: Exception) {
                                        ELog.e(TAG, "create cache file fail.", e)
                                        cacheFile.delete()
                                        callback?.onError()
                                    }
                                }

                            }
                        }
//                        ELog.i(TAG, "inflate start")
//                        inflate(bytes)?.let {
//                            ELog.i(TAG, "inflate complete")
//                            val videoItem = EvaVideoEntity(
//                                EvaCache.buildMp4File(cacheKey),
//                                mFrameWidth,
//                                mFrameHeight
//                            )
//                            callback?.onComplete(videoItem)
////                            ELog.i(TAG, "EvaVideoEntity prepare start")
////                            videoItem.prepare({
////                                ELog.i(TAG, "EvaVideoEntity prepare success")
////                                this.invokeCompleteCallback(videoItem, callback, alias)
////                            },playCallback)
//
//                        } ?: this.invokeErrorCallback(
//                            Exception("inflate(bytes) cause exception"),
//                            callback,
//                            alias
//                        )
                    }
                } ?: this.invokeErrorCallback(
                    Exception("readAsBytes(inputStream) cause exception"),
                    callback,
                    alias
                )
            } catch (e: java.lang.Exception) {
                this.invokeErrorCallback(e, callback, alias)
            } finally {
                if (closeInputStream) {
                    inputStream.close()
                }
                ELog.i(TAG, "================ decode $alias from input stream end ================")
            }
        }
    }

    /**
     * @deprecated from 2.4.0
     */
    @Deprecated("This method has been deprecated from 2.4.0.", ReplaceWith("this.decodeFromAssets(assetsName, callback)"))
    fun parse(assetsName: String, callback: ParseCompletion?) {
        this.decodeFromAssets(assetsName, callback,null)
    }

    /**
     * @deprecated from 2.4.0
     */
    @Deprecated("This method has been deprecated from 2.4.0.", ReplaceWith("this.decodeFromURL(url, callback)"))
    fun parse(url: URL, callback: ParseCompletion?) {
        this.decodeFromURL(url, callback,null)
    }

    /**
     * @deprecated from 2.4.0
     */
    @Deprecated("This method has been deprecated from 2.4.0.", ReplaceWith("this.decodeFromInputStream(inputStream, cacheKey, callback, closeInputStream)"))
    fun parse(
        inputStream: InputStream,
        cacheKey: String,
        callback: ParseCompletion?,
        closeInputStream: Boolean = false
    ) {
        this.decodeFromInputStream(inputStream, cacheKey, callback, closeInputStream,null)
    }

    private fun invokeCompleteCallback(
        videoItem: EvaVideoEntity,
        callback: ParseCompletion?,
        alias: String?
    ) {
        Handler(Looper.getMainLooper()).post {
            ELog.i(TAG, "================ $alias parser complete ================")
            callback?.onComplete(videoItem)
        }
    }

    private fun invokeErrorCallback(
        e: Exception,
        callback: ParseCompletion?,
        alias: String?
    ) {
        e.printStackTrace()
        ELog.e(TAG, "================ $alias parser error ================")
        ELog.e(TAG, "$alias parse error", e)
        Handler(Looper.getMainLooper()).post {
            callback?.onError()
        }
    }

    private fun decodeFromCacheKey(
        cacheKey: String,
        callback: ParseCompletion?,
        alias: String?
    ) {
        ELog.i(TAG, "================ decode $alias from cache ================")
        ELog.d(TAG, "decodeFromCacheKey called with cacheKey : $cacheKey")
        if (mContext == null) {
            ELog.e(TAG, "在配置 EvaDownloader context 前, 无法解析 EVA 文件。")
            return
        }
        try {
            val cacheDir = EvaCache.buildCacheDir(cacheKey)
            File(cacheDir, "movie.binary").takeIf { it.isFile }?.let { binaryFile ->
                try {
                    ELog.i(TAG, "binary change to entity")
                    FileInputStream(binaryFile).use {
                        ELog.i(TAG, "binary change to entity success")
                        this.invokeCompleteCallback(
                            EvaVideoEntity(
                                cacheDir,
                                mFrameWidth,
                                mFrameHeight
                            ),
                            callback,
                            alias
                        )
                    }

                } catch (e: Exception) {
                    ELog.e(TAG, "binary change to entity fail", e)
                    cacheDir.delete()
                    binaryFile.delete()
                    throw e
                }
            }
            File(cacheDir, "movie.spec").takeIf { it.isFile }?.let { jsonFile ->
                try {
                    ELog.i(TAG, "spec change to entity")
                    FileInputStream(jsonFile).use { fileInputStream ->
                        ByteArrayOutputStream().use { byteArrayOutputStream ->
                            val buffer = ByteArray(2048)
                            while (true) {
                                val size = fileInputStream.read(buffer, 0, buffer.size)
                                if (size == -1) {
                                    break
                                }
                                byteArrayOutputStream.write(buffer, 0, size)
                            }
//                            byteArrayOutputStream.toString().let {
//                                JSONObject(it).let {
//                                    ELog.i(TAG, "spec change to entity success")
//                                    this.invokeCompleteCallback(
//                                        EvaVideoEntity(
//                                            cacheDir,
//                                            mFrameWidth,
//                                            mFrameHeight
//                                        ),
//                                        callback,
//                                        alias
//                                    )
//                                }
//                            }
                            ELog.i(TAG, "spec change to entity success")
                            this.invokeCompleteCallback(
                                EvaVideoEntity(
                                    cacheDir,
                                    mFrameWidth,
                                    mFrameHeight
                                ),
                                callback,
                                alias
                            )
                        }
                    }
                } catch (e: Exception) {
                    ELog.e(TAG, "$alias movie.spec change to entity fail", e)
                    cacheDir.delete()
                    jsonFile.delete()
                    throw e
                }
            }
        } catch (e: Exception) {
            this.invokeErrorCallback(e, callback, alias)
        }
    }

    private fun readAsBytes(inputStream: InputStream): ByteArray? {
        ByteArrayOutputStream().use { byteArrayOutputStream ->
            val byteArray = ByteArray(2048)
            while (true) {
                val count = inputStream.read(byteArray, 0, 2048)
                if (count <= 0) {
                    break
                } else {
                    byteArrayOutputStream.write(byteArray, 0, count)
                }
            }
            return byteArrayOutputStream.toByteArray()
        }
    }

    private fun inflate(byteArray: ByteArray): ByteArray? {
        val inflater = Inflater()
        inflater.setInput(byteArray, 0, byteArray.size)
        val inflatedBytes = ByteArray(2048)
        ByteArrayOutputStream().use { inflatedOutputStream ->
            while (true) {
                val count = inflater.inflate(inflatedBytes, 0, 2048)
                if (count <= 0) {
                    break
                } else {
                    inflatedOutputStream.write(inflatedBytes, 0, count)
                }
            }
            inflater.end()
            return inflatedOutputStream.toByteArray()
        }
    }

    // 是否是 zip 文件
    private fun isZipFile(bytes: ByteArray): Boolean {
        return bytes.size > 4 && bytes[0].toInt() == 80 && bytes[1].toInt() == 75 && bytes[2].toInt() == 3 && bytes[3].toInt() == 4
    }

    // 解压
    private fun unzip(inputStream: InputStream, cacheKey: String) {
        ELog.i(TAG, "================ unzip prepare ================")
        val cacheDir = EvaCache.buildCacheDir(cacheKey)
        cacheDir.mkdirs()
        try {
            BufferedInputStream(inputStream).use {
                ZipInputStream(it).use { zipInputStream ->
                    while (true) {
                        val zipItem = zipInputStream.nextEntry ?: break
                        if (zipItem.name.contains("../")) {
                            // 解压路径存在路径穿越问题，直接过滤
                            continue
                        }
                        if (zipItem.name.contains("/")) {
                            continue
                        }
                        val file = File(cacheDir, zipItem.name)
                        ensureUnzipSafety(file, cacheDir.absolutePath)
                        FileOutputStream(file).use { fileOutputStream ->
                            val buff = ByteArray(2048)
                            while (true) {
                                val readBytes = zipInputStream.read(buff)
                                if (readBytes <= 0) {
                                    break
                                }
                                fileOutputStream.write(buff, 0, readBytes)
                            }
                        }
                        ELog.e(TAG, "================ unzip complete ================")
                        zipInputStream.closeEntry()
                    }
                }
            }
        } catch (e: Exception) {
            ELog.e(TAG, "================ unzip error ================")
            ELog.e(TAG, "error", e)
            EvaCache.clearDir(cacheDir.absolutePath)
            cacheDir.delete()
            throw e
        }
    }

    fun stop() {
        if (!threadPoolExecutor.isShutdown) {
            threadPoolExecutor.shutdown()
        }
    }

    // 检查 zip 路径穿透
    private fun ensureUnzipSafety(outputFile: File, dstDirPath: String) {
        val dstDirCanonicalPath = File(dstDirPath).canonicalPath
        val outputFileCanonicalPath = outputFile.canonicalPath
        if (!outputFileCanonicalPath.startsWith(dstDirCanonicalPath)) {
            throw IOException("Found Zip Path Traversal Vulnerability with $dstDirCanonicalPath")
        }
    }
}