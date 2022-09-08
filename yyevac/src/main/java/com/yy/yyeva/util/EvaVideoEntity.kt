package com.yy.yyeva.util

import java.io.File

class EvaVideoEntity {

    var mCacheDir: File
    var mFrameHeight = 0
    var mFrameWidth = 0

    constructor(cacheDir: File) : this(cacheDir, 0, 0)

    constructor(cacheDir: File, frameWidth: Int, frameHeight: Int) {
        mFrameWidth = frameWidth
        mFrameHeight = frameHeight
        mCacheDir = cacheDir
    }
}