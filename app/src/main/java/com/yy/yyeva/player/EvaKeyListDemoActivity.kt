package com.yy.yyeva.player

import android.app.Activity
import android.content.Context
import android.net.LinkAddress
import android.os.Bundle
import android.os.Environment
import android.os.Handler
import android.os.Looper
import com.yy.yyeva.EvaAnimConfig
import com.yy.yyeva.inter.IEvaAnimListener
import com.yy.yyeva.util.ELog
import com.yy.yyeva.util.IELog
import android.util.Log
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.yy.yyeva.player.bean.VideoInfo
import kotlinx.android.synthetic.main.activity_anim_demo_recycle.*

/**
 *
 * 必须使用YYEVA插件生成对应的mp4才能播放
 */
class EvaKeyListDemoActivity : Activity(), IEvaAnimListener {

    companion object {
        private const val TAG = "EvaDemoActivity"
    }

    private val dir by lazy {
        // 存放在sdcard应用缓存文件中
        getExternalFilesDir(null)?.absolutePath ?: Environment.getExternalStorageDirectory().path
    }

    // 视频信息
    private val videoInfo = VideoInfo("effect.mp4", "400a778f258ed6bd02ec32defe8ca8be")

    private var adapter: EvaKeyRecyclerAdapter? = null
    private lateinit var layoutManager: LinearLayoutManager
    private var findLastVisibleItemPosition = 0
    private var findFirstVisibleItemPosition = 0

    private val uiHandler by lazy {
        Handler(Looper.getMainLooper())
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_anim_demo_recycle)
        layoutManager = LinearLayoutManager(this)
        eva_recycler.layoutManager = layoutManager
        eva_recycler.addOnScrollListener(object : RecyclerView.OnScrollListener() {
            override fun onScrolled(recyclerView: RecyclerView, dx: Int, dy: Int) {
                super.onScrolled(recyclerView, dx, dy)
                if (dy > 0) { //下滑
                    if (layoutManager.findLastVisibleItemPosition() != findLastVisibleItemPosition) {
                        findLastVisibleItemPosition = layoutManager.findLastVisibleItemPosition()
                        ELog.i(TAG, "downscroll viewholder $findLastVisibleItemPosition")
                        val viewHolder = eva_recycler.findViewHolderForAdapterPosition(findLastVisibleItemPosition) as EvaKeyRecyclerAdapter.EvaKeyHolder
                        viewHolder.play()
                    }
                } else if (dy < 0) { //上划
                    if (layoutManager.findFirstVisibleItemPosition() != findFirstVisibleItemPosition) {
                        findFirstVisibleItemPosition = layoutManager.findFirstCompletelyVisibleItemPosition()
                        ELog.i(TAG, "upscroll viewholder $findFirstVisibleItemPosition")
                        val viewHolder = eva_recycler.findViewHolderForAdapterPosition(findFirstVisibleItemPosition) as EvaKeyRecyclerAdapter.EvaKeyHolder
                        viewHolder.play()
                    }
                }
            }
        })
        adapter = EvaKeyRecyclerAdapter()
        eva_recycler.adapter = adapter
        loadFile()
    }

    private fun init() {
        // 初始化日志
        initLog()
        adapter?.addBean(VideoInfo("effect.mp4", "400a778f258ed6bd02ec32defe8ca8be"))
        adapter?.addBean(VideoInfo("effect.mp4", "400a778f258ed6bd02ec32defe8ca8be"))
        adapter?.addBean(VideoInfo("effect.mp4", "400a778f258ed6bd02ec32defe8ca8be"))
        adapter?.addBean(VideoInfo("effect.mp4", "400a778f258ed6bd02ec32defe8ca8be"))
        adapter?.addBean(VideoInfo("effect.mp4", "400a778f258ed6bd02ec32defe8ca8be"))
        adapter?.addBean(VideoInfo("effect.mp4", "400a778f258ed6bd02ec32defe8ca8be"))
        adapter?.addBean(VideoInfo("effect.mp4", "400a778f258ed6bd02ec32defe8ca8be"))
        adapter?.notifyDataSetChanged()
    }

    /**
     * 视频信息准备好后的回调，用于检查视频准备好后是否继续播放
     * @return true 继续播放 false 停止播放
     */
    override fun onVideoConfigReady(config: EvaAnimConfig): Boolean {
        return true
    }

    /**
     * 视频开始回调
     */
    override fun onVideoStart() {
        ELog.i(TAG, "onVideoStart")
    }

    override fun onVideoRestart() {
        ELog.i(TAG, "onVideoReStart")
    }

    /**
     * 视频渲染每一帧时的回调
     * @param frameIndex 帧索引
     */
    override fun onVideoRender(frameIndex: Int, config: EvaAnimConfig?) {
    }

    /**
     * 视频播放结束(失败也会回调)
     */
    override fun onVideoComplete() {
        ELog.i(TAG, "onVideoComplete")
    }

    /**
     * 播放器被销毁情况下会调用
     */
    override fun onVideoDestroy() {
        ELog.i(TAG, "onVideoDestroy")
    }

    /**
     * 失败回调
     * 一次播放时可能会调用多次
     * @param errorType 错误类型
     * @param errorMsg 错误消息
     */
    override fun onFailed(errorType: Int, errorMsg: String?) {
        ELog.i(TAG, "onFailed errorType=$errorType errorMsg=$errorMsg")
    }

    private fun initLog() {
        ELog.isDebug = false
        ELog.log = object : IELog {
            override fun i(tag: String, msg: String) {
                Log.i(tag, msg)
            }

            override fun d(tag: String, msg: String) {
                Log.d(tag, msg)
            }

            override fun e(tag: String, msg: String) {
                Log.e(tag, msg)
            }

            override fun e(tag: String, msg: String, tr: Throwable) {
                Log.e(tag, msg, tr)
            }
        }
    }

    private fun loadFile() {
        val files = Array(1) {
            videoInfo.fileName
        }
        FileUtil.copyAssetsToStorage(this, dir, files) {
            uiHandler.post {
                init()
            }
        }
    }
}

