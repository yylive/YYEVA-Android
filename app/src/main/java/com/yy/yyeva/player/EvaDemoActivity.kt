package com.yy.yyeva.player

import android.app.Activity
import android.content.Context
import android.graphics.*
import android.os.Bundle
import android.os.Environment
import android.os.Handler
import android.os.Looper
import android.view.View
import android.widget.Toast
import com.yy.yyeva.EvaAnimConfig
import com.yy.yyeva.inter.IEvaAnimListener
import com.yy.yyeva.inter.IEvaFetchResource
import com.yy.yyeva.inter.OnEvaResourceClickListener
import com.yy.yyeva.mix.EvaResource
import com.yy.yyeva.util.ELog
import com.yy.yyeva.util.IELog
import com.yy.yyeva.util.ScaleType
import java.io.File
import java.util.*
import android.text.TextPaint
import android.util.Log
import com.yy.yyeva.mix.EvaSrc
import com.yy.yyeva.player.bean.VideoInfo
import com.yy.yyeva.view.EvaAnimViewV3
import kotlinx.android.synthetic.main.activity_anim_simple_demo_p.*
import kotlin.math.abs
import kotlin.math.ceil

/**
 *
 * 必须使用YYEVA插件生成对应的mp4才能播放
 */
class EvaDemoActivity : Activity(), IEvaAnimListener {

    companion object {
        private const val TAG = "EvaDemoActivity"
    }

    private val dir by lazy {
        // 存放在sdcard应用缓存文件中
        getExternalFilesDir(null)?.absolutePath ?: Environment.getExternalStorageDirectory().path
    }

    // 视频信息
    private val videoInfo = VideoInfo("effect.mp4", "400a778f258ed6bd02ec32defe8ca8be")


    // 动画View
    private lateinit var animView: EvaAnimViewV3

    private val uiHandler by lazy {
        Handler(Looper.getMainLooper())
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_anim_simple_demo_p)
        // 获取动画view
        animView = playerView
        loadFile()
    }

    private fun init() {
        // 初始化日志
        initLog()
        // 初始化调试开关
        initTestView()
        // 居中（根据父布局按比例居中并裁剪）
        animView.setScaleType(ScaleType.CENTER_CROP)
        /**
         * 注册资源获取类
         */
        animView.setFetchResource(object : IEvaFetchResource {
            /**
             * 获取图片资源
             * 无论图片是否获取成功都必须回调 result 否则会无限等待资源
             */
            override fun setImage(resource: EvaResource, result: (Bitmap?, EvaSrc.FitType?) -> Unit) {
                /**
                 * tag是素材中的一个标记，在制作素材时定义
                 * 解析时由业务读取tag决定需要播放的内容是什么
                 * 比如：一个素材里需要显示多个头像，则需要定义多个不同的tag，表示不同位置，需要显示不同的头像，文字类似
                 */
                val tag = resource.tag
                if (tag == "anchor_avatar1") { // 此tag是已经写入到动画配置中的tag
                    val drawableId = R.drawable.ball_1
                    val options = BitmapFactory.Options()
                    options.inScaled = false
                    val bitmap = BitmapFactory.decodeResource(resources, drawableId, options)
                    result(bitmap, null)
                    bitmap.recycle()
                } else if (tag == "anchor_avatar2") { // 此tag是已经写入到动画配置中的tag
                    val drawableId =  R.drawable.ball_2
                    val options = BitmapFactory.Options()
                    options.inScaled = false
                    val bitmap = BitmapFactory.decodeResource(resources, drawableId, options)
                    result(bitmap, null)
                    bitmap.recycle()
                } else if (tag == "anchor_avatar3") { // 此tag是已经写入到动画配置中的tag
                    val drawableId =  R.drawable.ball_3
                    val options = BitmapFactory.Options()
                    options.inScaled = false
                    val bitmap = BitmapFactory.decodeResource(resources, drawableId, options)
                    result(bitmap, null)
                    bitmap.recycle()
                } else {
                    result(null, null)
                }
            }

            /**
             * 获取文字资源
             */
            override fun setText(resource: EvaResource, result: (EvaResource) -> Unit) {
                val tag = resource.tag
                if (tag == "anchor_nick") { // 此tag是已经写入到动画配置中的tag
                    resource.text = "USERNICK"
                    resource.textAlign = "center"
                    result(resource)
                } else {
                    result(resource)
                }
            }

            /**
             * 播放完毕后的资源回收
             */
            override fun releaseSrc(resources: List<EvaResource>) {
            }
        })

        // 注册点击事件监听
        animView.setOnResourceClickListener(object : OnEvaResourceClickListener {
            override fun onClick(resource: EvaResource) {
                Toast.makeText(
                    this@EvaDemoActivity,
                    "tag=${resource.tag} onClick",
                    Toast.LENGTH_LONG
                ).show()
            }
        })

        // 注册动画监听
        animView.setAnimListener(this)
//        animView.setVideoMode(EvaConstant.VIDEO_MODE_SPLIT_HORIZONTAL)
//        animView.setAudioSpeed(2.0f)
//        animView.setLastFrame(true)
        /**
         * 开始播放主流程
         * 主要流程都是对AnimViewV3的操作，内部是集成TextureView
         */
        play(videoInfo)
    }

    fun generateBitmap(text: String, textSizePx: Int, textColor: Int): Bitmap {
        val textPaint = TextPaint()
        textPaint.textSize = textSizePx.toFloat()
        textPaint.color = textColor
        val width = ceil(textPaint.measureText(text).toDouble()).toInt()
        val fontMetrics: Paint.FontMetrics = textPaint.fontMetrics
        val height = ceil(abs(fontMetrics.bottom) + abs(fontMetrics.top))
            .toInt()
        val bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888)
        val canvas = Canvas(bitmap)
        canvas.drawText(text, 0f, abs(fontMetrics.ascent), textPaint)
        return bitmap
    }


    private fun play(videoInfo: VideoInfo) {
        // 播放前强烈建议检查文件的md5是否有改变
        // 因为下载或文件存储过程中会出现文件损坏，导致无法播放
        Thread {
            val file = File(dir + "/" + videoInfo.fileName)
            if (!file.exists()) {
                ELog.e(TAG, "${videoInfo.fileName} is not exist")
                return@Thread
            }
            animView.startPlay(file)
//            val md5 = FileUtil.getFileMD5(file)
//            if (videoInfo.md5 == md5) {
//                // 开始播放动画文件
//                animView.startPlay(file)
//            } else {
//                ELog.e(TAG, "md5 is not match, error md5=$md5")
//            }
        }.start()
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
    override fun onVideoStart(isRestart: Boolean) {
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
    override fun onVideoComplete(lastFrame: Boolean) {
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


    override fun onPause() {
        super.onPause()
        // 页面切换是停止播放
        animView.stopPlay()
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


    private fun initTestView() {
        btnLayout.visibility = View.VISIBLE
        /**
         * 开始播放
         */
        btnPlay.setOnClickListener {
            play(videoInfo)
        }
        /**
         * 结束视频
         */
        btnStop.setOnClickListener {
            ELog.i(TAG, "btnStop")
            animView.stopPlay()
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

    private fun dp2px(context: Context, dp: Float): Float {
        val scale = context.resources.displayMetrics.density
        return dp * scale + 0.5f
    }
}

