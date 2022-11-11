package com.yy.yyevav2.downloder

import java.net.URL

class EvaDownloader {

    interface IDownloader {

        fun download(url: URL, succ: (String) -> Unit, error: (Throwable) -> Unit)

        fun cancel()
    }

    private val defaultDownloader = object : IDownloader {
        
        override fun download(url: URL, succ: (String) -> Unit, error: (Throwable) -> Unit) {
        }

        override fun cancel() {
        }

    }

    init {
        setDownloader(defaultDownloader)
    }

    companion object {
        private var mDownloader: IDownloader? = null

        fun setDownloader(downloader: IDownloader) {
            mDownloader = downloader
        }
    }

    fun download(url: URL, succ: (String) -> Unit, error: (Throwable) -> Unit) {
        mDownloader?.download(url, succ, error)
    }

    fun cancel() {
        mDownloader?.cancel()
    }
}