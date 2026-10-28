package com.yy.yyeva.player

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.os.Environment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.Toast
import androidx.recyclerview.widget.RecyclerView
import com.yy.yyeva.EvaAnimConfig
import com.yy.yyeva.inter.IEvaAnimListener
import com.yy.yyeva.inter.IEvaFetchResource
import com.yy.yyeva.inter.OnEvaResourceClickListener
import com.yy.yyeva.mix.EvaResource
import com.yy.yyeva.mix.EvaSrc
import com.yy.yyeva.player.bean.VideoInfo
import com.yy.yyeva.util.ELog
import com.yy.yyeva.util.ScaleType
import com.yy.yyeva.view.EvaAnimViewV3
import java.io.File

class EvaKeyRecyclerAdapter():RecyclerView.Adapter<EvaKeyRecyclerAdapter.EvaKeyHolder>() {
    val TAG = "EvaKeyRecyclerAdapter"
    var data = ArrayList<VideoInfo>()

    fun addBean(info: VideoInfo) {
        data.add(info)
    }

    fun clear() {
        data.clear()
        notifyDataSetChanged()
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): EvaKeyHolder {
        return EvaKeyHolder(LayoutInflater.from(parent.context).inflate(R.layout.item_animview, parent, false))
    }

    override fun onBindViewHolder(holder: EvaKeyHolder, position: Int) {
        holder.itemView.tag = position
        holder.stop()
        holder.setInfo(data[position], position)
    }

    override fun getItemCount(): Int {
        return data.size
    }

    override fun onViewRecycled(holder: EvaKeyHolder) {
        super.onViewRecycled(holder)
        holder.stop()
    }

    class EvaKeyHolder(item: View): RecyclerView.ViewHolder(item) {
        var TAG = ""
        private val dir by lazy {
            // 存放在sdcard应用缓存文件中
            item.context.getExternalFilesDir(null)?.absolutePath ?: Environment.getExternalStorageDirectory().path
        }
        val animView: EvaAnimViewV3 = item.findViewById(R.id.playerView)
        val btnPlay: Button = item.findViewById(R.id.btnPlay)
        val btnStop: Button = item.findViewById(R.id.btnStop)
        lateinit var info: VideoInfo
        private var p = 0

        init {
            /**
             * 开始播放
             */
            btnPlay.setOnClickListener {
                play()
            }
            /**
             * 结束视频
             */
            btnStop.setOnClickListener {
                animView.stopPlay()
            }
        }

        fun setInfo(info: VideoInfo, i: Int) {
            this.info = info
            p = i
            TAG = "EvaKeyHolder_$i"
            // 居中（根据父布局按比例居中并裁剪）
            animView.setScaleType(ScaleType.CENTER_CROP)
            /**
             * 注册资源获取类
             */
            animView.setFetchResource(object : IEvaFetchResource {
                /**
                 * 获取图片资源z
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
                        val bitmap = BitmapFactory.decodeResource(animView.context.resources, drawableId, options)
                        result(bitmap, null)
                        bitmap.recycle()
                    } else if (tag == "anchor_avatar2") { // 此tag是已经写入到动画配置中的tag
                        val drawableId =  R.drawable.ball_2
                        val options = BitmapFactory.Options()
                        options.inScaled = false
                        val bitmap = BitmapFactory.decodeResource(animView.context.resources, drawableId, options)
                        result(bitmap, null)
                        bitmap.recycle()
                    } else if (tag == "anchor_avatar3") { // 此tag是已经写入到动画配置中的tag
                        val drawableId =  R.drawable.chang
                        val options = BitmapFactory.Options()
                        options.inScaled = false
                        val bitmap = BitmapFactory.decodeResource(animView.context.resources, drawableId, options)
                        result(bitmap, EvaSrc.FitType.CENTER_FULL)
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
                        resource.text = "USERNICK\uD83D\uDE04"
                        resource.textAlign = "left"
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
                        animView.context,
                        "tag=${resource.tag} onClick",
                        Toast.LENGTH_LONG
                    ).show()
                }
            })

            // 注册动画监听
            animView.setAnimListener(object : IEvaAnimListener {
                override fun onVideoStart(isRestart: Boolean) {
                    ELog.i(TAG, "onVideoStart")
                }

                override fun onVideoRestart() {
                    ELog.i(TAG, "onVideoReStart")
                }

                override fun onVideoRender(frameIndex: Int, config: EvaAnimConfig?) {
                }

                override fun onVideoComplete(lastFrame: Boolean) {
                    ELog.i(TAG, "onVideoComplete")
                }

                override fun onVideoDestroy() {
                    ELog.i(TAG, "onVideoDestroy")
                }

                override fun onFailed(errorType: Int, errorMsg: String?) {
                    ELog.i(TAG, "onFailed errorType=$errorType errorMsg=$errorMsg")
                }
            })
            /**
             * 开始播放主流程
             * 主要流程都是对AnimViewV3的操作，内部是集成TextureView
             */
//            play()
        }

        fun play() {
            //设置背景图
            val img = BitmapFactory.decodeResource(animView.context.resources, R.drawable.bg)
            animView.setBgImage(img)
            if (animView.isRunning()) {
                ELog.i(TAG, "position $p isRunning")
                return
            }
            // 播放前强烈建议检查文件的md5是否有改变
            // 因为下载或文件存储过程中会出现文件损坏，导致无法播放
            val file = File(dir + "/" + info.fileName)
            if (!file.exists()) {
                ELog.e(TAG, "${info.fileName} is not exist")
            } else {
                //循环三次
//            animView.setLoop(3)
                animView.startPlay(file)
//            val md5 = FileUtil.getFileMD5(file)
            }
        }

        fun stop() {
            if (animView.isRunning()) {
                animView.stopPlay()
            }
        }
    }
}