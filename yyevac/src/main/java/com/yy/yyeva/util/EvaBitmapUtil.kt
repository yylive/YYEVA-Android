package com.yy.yyeva.util

import android.graphics.*
import android.text.TextPaint
import com.yy.yyeva.mix.EvaSrc

object EvaBitmapUtil {

    fun createEmptyBitmap() : Bitmap {
        return Bitmap.createBitmap(16, 16, Bitmap.Config.ARGB_8888).apply {
            eraseColor(Color.TRANSPARENT)
        }
    }

    fun createTxtBitmap(src: EvaSrc, tf: Typeface? = null): Bitmap {
        val w = src.w
        val h = src.h
        // 这里使用ALPHA_8 在opengl渲染的时候图像出现错位
        val bitmap = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888)
        val canvas = Canvas(bitmap)
        val rect = Rect(0, 0, w, h)
        val bounds = Rect()
        var sizeR = 0.8f
        val paint = TextPaint().apply {
//            textSize = h * sizeR
            textSize = src.frontSize.toFloat()
            textAlign = when (src.textAlign) {
                "right" -> {
                    Paint.Align.RIGHT
                }
                "left" -> {
                    Paint.Align.LEFT
                }
                else -> {
                    Paint.Align.CENTER
                }
            }
            style = Paint.Style.FILL
            isAntiAlias = true
            if (tf != null) {
                typeface = tf
            } else if (src.style == EvaSrc.Style.BOLD) {
                typeface = Typeface.create(Typeface.DEFAULT, Typeface.BOLD)
            }
            color = src.color
        }
        val text = src.txt
        while (sizeR > 0.1f) {
            paint.getTextBounds(text, 0, text.length, bounds)
            if (bounds.width() <= rect.width()) {
                break
            }
            sizeR -= 0.1f
//            paint.textSize = h * sizeR
            paint.textSize = src.frontSize.toFloat()
        }
        val fontMetrics = paint.fontMetricsInt
        val top = fontMetrics.top
        val bottom = fontMetrics.bottom
        val baseline = rect.centerY() - top/2 - bottom/2
        when (src.textAlign) {
            "left" -> {
                canvas.drawText(text, rect.left.toFloat(), baseline.toFloat(), paint)
            }
            "right" -> {
                canvas.drawText(text, rect.right.toFloat(), baseline.toFloat(), paint)
            }
            else -> {
                canvas.drawText(text, rect.centerX().toFloat(), baseline.toFloat(), paint)
            }
        }

        return bitmap
    }

}