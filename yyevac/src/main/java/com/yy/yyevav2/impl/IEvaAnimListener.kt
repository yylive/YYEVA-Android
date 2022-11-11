package com.yy.yyevav2.impl

import kotlin.collections.ArrayList

interface IEvaAnimListener {

    /**
     * 配置准备好后回调
     * @return true 继续播放 false 结束播放
     */
    fun onVideoConfigReady(evaSrc: ArrayList<EvaSrcSim>): Boolean {
        return true // 默认继续播放
    }

    /**
     * 开始播放
     */
    fun onVideoStart()


    /**
     * 视频渲染每一帧时的回调
     * @param frameIndex 帧索引
     */
    fun onVideoRender(frameIndex: Int)

    /**
     * 视频播放结束
     */
    fun onVideoComplete()

    /**
     * 视频被销毁
     */
    fun onVideoDestroy()

    /**
     * 失败回调
     * @param errorType 错误类型
     * @param errorMsg 错误消息
     */
    fun onFailed(errorType: Int, errorMsg: String?)

    /**
     * 触摸回调
     */
    fun touchCallback(x: Int, y: Int, evaSrc: EvaSrcSim)
}