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
    external fun getExternalTexture(controllerId: Int): Int

    /**
     * 更新渲染尺寸
     */
    external fun updateViewPoint(controllerId: Int, width: Int, height: Int)

    /**
     * 视频大小变化
     */
    external fun videoSizeChange(controllerId: Int, newWidth: Int, newHeight: Int)

    /**
     * 初始化mp4渲染
     * 返回管理器id
     */
    external fun initRender(controllerId: Int, surface: Surface, isNeedYuv: Boolean, isNormalMp4: Boolean): Int

    /**
     * 设置动效json数据
     */
    external fun setRenderConfig(controllerId: Int, json: String)

    external fun defaultConfig(controllerId: Int, width: Int, height: Int, defaultVideoMode: Int = EvaConstant.VIDEO_MODE_SPLIT_HORIZONTAL_REVERSE)

    /**
     * 设置背景bitmap数据
     */
    external fun setBgBitmap(controllerId: Int, bitmap: Bitmap?)

    /**
     * 设置动效元素bitmap数据
     */
    external fun setSrcBitmap(controllerId: Int, srcId: String, bitmap: Bitmap?, scaleMode: String)

    /**
     * 设置动效元素txt
     */
    external fun setSrcTxt(controllerId: Int, srcId: String, txt: String)

    /**
     * 进行MP4渲染
     */
    external fun renderFrame(controllerId: Int)

    /**
     * 清空MP4渲染
     */
    external fun renderClearFrame(controllerId: Int)

    /**
     * 清空纹理
     */
    external fun releaseTexture(controllerId: Int)

    /**
     * 交换渲染mp4纹理数据
     */
    external fun renderSwapBuffers(controllerId: Int)

    /**
     * 销毁mp4纹理渲染
     */
    external fun destroyRender(controllerId: Int)

    /**
     * 动效元素配置
     */
    external fun mixConfigCreate(controllerId: Int, json: String): Int

    /**
     * 动效元素创建
     */
    external fun mixRenderCreate(controllerId: Int)

    /**
     * 动效元素渲染
     */
    external fun mixRendering(controllerId: Int, frameIndex: Int)

    /**
     * 销毁动效元素
     */
    external fun mixRenderDestroy(controllerId: Int)
}