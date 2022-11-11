package com.yy.yyeva.util

import android.graphics.Bitmap
import android.view.Surface

object EvaJniUtil {
    init {
        System.loadLibrary("yyeva")
    }

    external fun nativeProcessBitmap(bitmap: Bitmap)

    /**
     * 获取底层的textureId，用于创建和绑定surfaceView或textureView
     */
    external fun getExternalTexture(): Int

    /**
     * 更新渲染尺寸
     */
    external fun updateViewPoint(width: Int, height: Int)

    /**
     * 视频大小变化
     */
    external fun videoSizeChange(newWidth: Int, newHeight: Int)

    /**
     * 初始化mp4渲染
     */
    external fun initRender(surface: Surface, isNeedYuv: Boolean): Int

    /**
     * 设置动效json数据
     */
    external fun setRenderConfig(json: String)

    external fun defaultConfig(width: Int, height: Int, defaultVideoMode: Int = EvaConstant.VIDEO_MODE_SPLIT_HORIZONTAL_REVERSE)

    /**
     * 设置背景bitmap数据
     */
    external fun setBgBitmap(bitmap: Bitmap?)

    /**
     * 设置动效元素bitmap数据
     */
    external fun setSrcBitmap(srcId: String, bitmap: Bitmap?, address: String, scaleMode: String)

    /**
     * 设置动效元素txt
     */
    external fun setSrcTxt(srcId: String, txt: String)

    /**
     * 进行MP4渲染
     */
    external fun renderFrame()

    /**
     * 清空MP4渲染
     */
    external fun renderClearFrame()

    /**
     * 清空纹理
     */
    external fun releaseTexture()

    /**
     * 交换渲染mp4纹理数据
     */
    external fun renderSwapBuffers()

    /**
     * 销毁mp4纹理渲染
     */
    external fun destroyRender()

    /**
     * 动效元素配置
     */
    external fun mixConfigCreate(json: String)

    /**
     * 动效元素创建
     */
    external fun mixRenderCreate()

    /**
     * 动效元素渲染
     */
    external fun mixRendering(frameIndex: Int)

    /**
     * 销毁动效元素
     */
    external fun mixRenderDestroy()
}