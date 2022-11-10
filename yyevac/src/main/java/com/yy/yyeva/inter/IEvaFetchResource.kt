package com.yy.yyeva.inter

import android.graphics.Bitmap
import com.yy.yyeva.mix.EvaResource

interface IEvaFetchResource {
    // 获取图片 (暂时不支持Bitmap.Config.ALPHA_8 主要是因为一些机型opengl兼容问题)
    fun setImage(resource: EvaResource, result:(Bitmap?) -> Unit)

    // 获取文字
    fun setText(resource: EvaResource, result:(String?, String?) -> Unit)

    // 资源释放通知
    fun releaseSrc(resources: List<EvaResource>)
}