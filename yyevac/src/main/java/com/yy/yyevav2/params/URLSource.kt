package com.yy.yyevav2.params

import java.net.URL

class URLSource(val url: URL) : SourceParams() {

    override fun toParams(): String = ""
}