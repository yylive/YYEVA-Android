package com.yy.yyeva.player

import android.app.Activity
import android.content.Context
import android.graphics.*
import android.os.Bundle
import android.os.Environment
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.View
import com.yy.yyeva.player.R
import java.io.File
import java.util.*
import android.text.TextPaint
import android.widget.Toast
import com.yy.yyevav2.EvaPlayer
import com.yy.yyevav2.impl.EvaSrcSim
import com.yy.yyevav2.impl.IEvaAnimListener
import com.yy.yyevav2.log.EvaLog
import com.yy.yyevav2.params.AddressSource
import kotlinx.android.synthetic.main.activity_anim_simple_demo_c.*
import kotlin.math.abs
import kotlin.math.ceil


/**
 *
 * 必须使用YYEVA插件生成对应的mp4才能播放
 */
class EvaCActivity : Activity(), IEvaAnimListener {

    companion object {
        private const val TAG = "EvaCActivity"
    }

    init {
        System.loadLibrary("yyeva2")
    }


    private val dir by lazy {
        // 存放在sdcard应用缓存文件中
        getExternalFilesDir(null)?.absolutePath ?: Environment.getExternalStorageDirectory().path
    }

    // 视频信息
    data class VideoInfo(val fileName: String, val md5: String)

    private val videoInfo = VideoInfo("effect2.mp4", "1e22faf336ecfb36f61cfea27b8077bc")

    // 动画View
    private lateinit var animView: EvaPlayer

    private val uiHandler by lazy {
        Handler(Looper.getMainLooper())
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_anim_simple_demo_c)
        // 文件加载完成后会调用init方法
        loadFile()
    }

    private fun init() {
        // 初始化调试开关
        initTestView()
        // 获取动画view
        animView = playerView
//        // 居中（根据父布局按比例居中并裁剪）
//        animView.setScaleType(ScaleType.CENTER_CROP)
//        /**
//         * 注册资源获取类
//         */
//        animView.setFetchResource(object : IEvaFetchResource {
//            /**
//             * 获取图片资源
//             * 无论图片是否获取成功都必须回调 result 否则会无限等待资源
//             */
//            override fun setImage(resource: EvaResource, result: (Bitmap?) -> Unit) {
//                /**
//                 * srcTag是素材中的一个标记，在制作素材时定义
//                 * 解析时由业务读取tag决定需要播放的内容是什么
//                 * 比如：一个素材里需要显示多个头像，则需要定义多个不同的tag，表示不同位置，需要显示不同的头像，文字类似
//                 */
//                val srcTag = resource.tag
//                if (srcTag == "anchor_avatar1") { // 此tag是已经写入到动画配置中的tag
//                    val drawableId = R.drawable.ball_1
//                    val options = BitmapFactory.Options()
//                    options.inScaled = false
//                    result(BitmapFactory.decodeResource(resources, drawableId, options))
//                } else if (srcTag == "anchor_avatar2") { // 此tag是已经写入到动画配置中的tag
//                    val drawableId =  R.drawable.ball_2
//                    val options = BitmapFactory.Options()
//                    options.inScaled = false
//                    result(BitmapFactory.decodeResource(resources, drawableId, options))
//                } else if (srcTag == "anchor_avatar3") { // 此tag是已经写入到动画配置中的tag
//                    val drawableId =  R.drawable.ball_3
//                    val options = BitmapFactory.Options()
//                    options.inScaled = false
//                    result(BitmapFactory.decodeResource(resources, drawableId, options))
//                } else {
//                    result(null)
//                }
//            }
//
//            /**
//             * 获取文字资源
//             */
//            override fun setText(resource: EvaResource, result: (String?) -> Unit) {
//                val srcTag = resource.tag
//                if (srcTag == "anchor_nick") { // 此tag是已经写入到动画配置中的tag
//                    result("USERNICK")
//                } else {
//                    result(null)
//                }
//            }
//
//            /**
//             * 播放完毕后的资源回收
//             */
//            override fun releaseSrc(resources: List<EvaResource>) {
//                resources.forEach {
//                    it.bitmap?.recycle()
//                }
//            }
//        })
//
//        // 注册点击事件监听
//        animView.setOnResourceClickListener(object : OnEvaResourceClickListener {
//
//            override fun onClick(resource: EvaResource) {
//                Toast.makeText(
//                    this@EvaCActivity,
//                    "srcTag=${resource.tag} onClick",
//                    Toast.LENGTH_LONG
//                ).show()
//            }
//        })
        /**
         * 开始播放主流程
         * ps: 主要流程都是对AnimView的操作，其它比如队列，或改变窗口大小等操作都不是必须的
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
        // 注册动画监听
        animView.setAnimListener(this)

        val file = File(dir + "/" + videoInfo.fileName)
        if (!file.exists()) {
            EvaLog.e(TAG, "${videoInfo.fileName} is not exist")
        }
//        val md5 = FileUtil.getFileMD5(file)
//        if (videoInfo.md5 == md5) {
//            // 开始播放动画文件
//            animView.start(AddressSource(dir + "/" + videoInfo.fileName))
//        } else {
//            Log.e(TAG, "md5 is not match, error md5=$md5")
//        }
        val img = BitmapFactory.decodeResource(resources, R.drawable.bg)
        animView.setBgImage(img)

        //设置播放次数
//        animView.setLoop(3)
        animView.start(AddressSource(dir + "/" + videoInfo.fileName))
    }


    private fun play(fileName: String) {
        // 播放前强烈建议检查文件的md5是否有改变
        // 因为下载或文件存储过程中会出现文件损坏，导致无法播放
        Thread {
            val file = File(dir + "/" + videoInfo.fileName)
            // 开始播放动画文件
//            animView.startPlay(file)
            animView.start(AddressSource(dir + "/" + videoInfo.fileName))
        }.start()
    }

    /**
     * 视频信息准备好后的回调，用于检查视频准备好后是否继续播放
     * @return true 继续播放 false 停止播放
     */
    override fun onVideoConfigReady(evaSrc: ArrayList<EvaSrcSim>): Boolean {
        Log.i(TAG, "onVideoConfigReady $evaSrc")
        for (src in evaSrc) {
            when (src.srcTag) {
                "anchor_avatar1" -> {
                    val options = BitmapFactory.Options()
                    options.inScaled = false
                    val bitmap1 = BitmapFactory.decodeResource(resources, R.drawable.ball_1, options)
                    animView.addImgEffect(bitmap1, src)
                    bitmap1.recycle()
                }
                "anchor_avatar2" -> {
                    val options = BitmapFactory.Options()
                    options.inScaled = false
                    val bitmap2 = BitmapFactory.decodeResource(resources, R.drawable.ball_2, options)
                    animView.addImgEffect(bitmap2, src)
                    bitmap2.recycle()
                }
                "anchor_avatar3" -> {
                    val options = BitmapFactory.Options()
                    options.inScaled = false
                    val bitmap3 = BitmapFactory.decodeResource(resources, R.drawable.chang, options)
                    animView.addImgEffect(bitmap3, src, "aspectFit")
                    bitmap3.recycle()
                }
                "anchor_nick" -> {
                    src.textAlign = "right"
                    animView.addTxtEffect("USERNICK", src)
                }
            }
        }
        return true
    }

    /**
     * 视频开始回调
     */
    override fun onVideoStart() {
        Log.i(TAG, "onVideoStart")
    }

    /**
     * 视频渲染每一帧时的回调
     * @param frameIndex 帧索引
     */
    override fun onVideoRender(frameIndex: Int) {
        Log.i(TAG, "onVideoRender $frameIndex")
    }

    /**
     * 视频播放结束(失败也会回调onComplete)
     */
    override fun onVideoComplete() {
        Log.i(TAG, "onVideoComplete")
        animView.release()
    }

    /**
     * 播放器被销毁情况下会调用onVideoDestroy
     */
    override fun onVideoDestroy() {
        Log.i(TAG, "onVideoDestroy")
//        animView.release()
    }

    /**
     * 失败回调
     * 一次播放时可能会调用多次，建议onFailed只做错误上报
     * @param errorType 错误类型
     * @param errorMsg 错误消息
     */
    override fun onFailed(errorType: Int, errorMsg: String?) {
        Log.i(TAG, "onFailed errorType=$errorType errorMsg=$errorMsg")
        animView.release()
    }


    override fun onPause() {
        super.onPause()
        // 页面切换是停止播放
        animView.stop()
    }

    override fun touchCallback(x: Int, y: Int, evaSrc: EvaSrcSim) {
        Log.v(TAG, "touchCallback $x $y $evaSrc")
        Toast.makeText(
                    this@EvaCActivity,
                    "srcTag=${evaSrc.srcTag} onClick",
                    Toast.LENGTH_LONG
                ).show()
    }

    private fun initTestView() {
        btnLayout.visibility = View.VISIBLE
        /**
         * 开始播放按钮
         */
        btnPlay.setOnClickListener {

            play(videoInfo)
        }
        /**
         * 结束视频按钮
         */
        btnStop.setOnClickListener {
            animView.stop()
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

    override fun onDestroy() {
        super.onDestroy()
        animView.destroy()
    }
}

