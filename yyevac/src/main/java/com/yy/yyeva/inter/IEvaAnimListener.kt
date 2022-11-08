package com.yy.yyeva.inter

import com.yy.yyeva.EvaAnimConfig

interface IEvaAnimListener {

    /**
     * 配置准备好后回调
     * @return true 继续播放 false 结束播放
     */
    fun onVideoConfigReady(config: EvaAnimConfig): Boolean {
        return true // 默认继续播放
    }

    /**
     * 开始播放
     */
    fun onVideoStart()

    /**
     * 循环播放开始
     */
    fun onVideoRestart()


    /**
     * 视频渲染每一帧时的回调
     * @param frameIndex 帧索引
     */
    fun onVideoRender(frameIndex: Int, config: EvaAnimConfig?)

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
}