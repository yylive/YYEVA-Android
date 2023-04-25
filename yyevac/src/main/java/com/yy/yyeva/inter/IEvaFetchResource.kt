package com.yy.yyeva.inter

import android.graphics.Bitmap
import com.yy.yyeva.mix.EvaResource
import com.yy.yyeva.mix.EvaSrc

interface IEvaFetchResource {
    // 获取图片 (暂时不支持Bitmap.Config.ALPHA_8 主要是因为一些机型opengl兼容问题)
    fun setImage(resource: EvaResource, result: (Bitmap?, EvaSrc.FitType?) -> Unit)

    /**
     * 获取文字
     * @param result 设置文件的回调函数，其参数分别是替换文本，文本对齐方式及文本是否加粗
     */
    fun setText(
        resource: EvaResource,
        result: (String?, String?, Boolean?) -> Unit
    )
}