package com.yy.yyevav2.log

interface ILog {
    fun d(tag: String, msg: String)
    fun v(tag: String, msg: String)
    fun i(tag: String, msg: String)
    fun w(tag: String, msg: String)
    fun e(tag: String, msg: String)
}