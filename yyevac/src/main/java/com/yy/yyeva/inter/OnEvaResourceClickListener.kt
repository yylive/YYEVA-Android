package com.yy.yyeva.inter

import com.yy.yyeva.mix.EvaResource

interface OnEvaResourceClickListener {

    // 返回被点击的资源
    fun onClick(resource: EvaResource)
}