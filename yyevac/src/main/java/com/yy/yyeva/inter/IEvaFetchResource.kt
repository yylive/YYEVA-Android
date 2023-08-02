package com.yy.yyeva.inter

import android.graphics.Bitmap
import com.yy.yyeva.mix.EvaResource
import com.yy.yyeva.mix.EvaSrc

interface IEvaFetchResource {
    // 获取图片 (暂时不支持Bitmap.Config.ALPHA_8 主要是因为一些机型opengl兼容问题)
    fun setImage(resource: EvaResource, result:(Bitmap?, EvaSrc.FitType?) -> Unit)

    // 获取文字
    fun setText(resource: EvaResource, result:(EvaResource) -> Unit)

    // 资源释放通知
    fun releaseSrc(resources: List<EvaResource>)
}