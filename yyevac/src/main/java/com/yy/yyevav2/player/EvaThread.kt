package com.yy.yyevav2.player

import android.os.Handler
import android.os.HandlerThread
import android.os.Process
import com.yy.yyevav2.log.EvaLog

private const val TAG = "EvaThread"

class EvaThread(
    private val name: String,
    private val priority: Int = Process.THREAD_PRIORITY_AUDIO
) {
    private var mThread: HandlerThread? = null
    private var mHandler: Handler? = null

    private var mIsStarted = false

    fun start() {
        synchronized(this) {
            if (!mIsStarted) {
                mIsStarted = true
                if (null == mThread) {
                    mThread = HandlerThread(name, priority).apply {
                        start()
                    }
                    mHandler = Handler(mThread!!.looper)
                }
            }
        }
        EvaLog.i(TAG, "EvaThread start")
    }

    fun release() {
        synchronized(this) {
            doInnerRelease()
        }
        EvaLog.i(TAG, "EvaThread release")
    }

    private fun doInnerRelease() {
        mIsStarted = false
        mHandler?.removeCallbacksAndMessages(null)
        mHandler = null
        mThread?.quitSafely()
        mThread = null
    }

    fun queueEvent(runnable: () -> Unit) {
        synchronized(this) {
            mHandler?.post(runnable)
        }
    }

    fun queueEventFront(runnable: () -> Unit) {
        synchronized(this) {
            mHandler?.postAtFrontOfQueue(runnable)
        }
    }
}