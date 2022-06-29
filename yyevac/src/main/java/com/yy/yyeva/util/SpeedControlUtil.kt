package com.yy.yyeva.util

class SpeedControlUtil {

    companion object {
        private const val TAG = "${EvaConstant.TAG}.SpeedControlUtil"
    }
    private val ONE_MILLION = 1000000L

    private var prevPresentUsec: Long = 0
    private var prevMonoUsec: Long = 0
    private var fixedFrameDurationUsec: Long = 0
    private var loopReset = true

    //设置帧率
    fun setFixedPlaybackRate(fps: Int) {
        if (fps <=0) return
        fixedFrameDurationUsec = ONE_MILLION / fps
    }
    //控制帧率时间
    fun preRender(presentationTimeUsec: Long) {
        if (prevMonoUsec == 0L) {
            prevMonoUsec = System.nanoTime() / 1000
            prevPresentUsec = presentationTimeUsec
        } else {
            var frameDelta: Long
            if (loopReset) {
                prevPresentUsec = presentationTimeUsec - ONE_MILLION / 30
                loopReset = false
            }
            frameDelta = if (fixedFrameDurationUsec != 0L) {
                fixedFrameDurationUsec
            } else {
                presentationTimeUsec - prevPresentUsec
            }
            when {
                frameDelta < 0 -> frameDelta = 0
                frameDelta > 10 * ONE_MILLION -> frameDelta = 5 * ONE_MILLION
            }

            val desiredUsec = prevMonoUsec + frameDelta
            var nowUsec = System.nanoTime() / 1000
            while (nowUsec < desiredUsec - 100 ) {
                var sleepTimeUsec = desiredUsec - nowUsec
                if (sleepTimeUsec > 500000) {
                    sleepTimeUsec = 500000
                }
                try {
                    Thread.sleep(sleepTimeUsec / 1000, (sleepTimeUsec % 1000).toInt() * 1000)
                } catch (e: InterruptedException) {
                    ELog.e(TAG, "e=$e", e)
                }
                nowUsec = System.nanoTime() / 1000
            }

            prevMonoUsec += frameDelta
            prevPresentUsec += frameDelta
        }
    }

    fun reset() {
        prevPresentUsec = 0
        prevMonoUsec = 0
    }
}