package com.yy.yyevav2.log

import android.util.Log

object EvaLog {
    private var logImpl: ILog? = object : ILog {

        override fun d(tag: String, msg: String) {
            Log.d(tag, msg)
        }

        override fun v(tag: String, msg: String) {
            Log.v(tag, msg)
        }

        override fun i(tag: String, msg: String) {
            Log.i(tag, msg)
        }

        override fun w(tag: String, msg: String) {
            Log.w(tag, msg)
        }

        override fun e(tag: String, msg: String) {
            Log.e(tag, msg)
        }
    }

    fun setLogImpl(log: ILog) {
        logImpl = log
    }

    fun d(tag: String, msg: String) {
        logImpl?.d(tag, msg)
    }

    fun v(tag: String, msg: String) {
        logImpl?.v(tag, msg)
    }

    fun i(tag: String, msg: String) {
        logImpl?.i(tag, msg)
    }

    fun w(tag: String, msg: String) {
        logImpl?.w(tag, msg)
    }

    fun e(tag: String, msg: String) {
        logImpl?.e(tag, msg)
    }
}