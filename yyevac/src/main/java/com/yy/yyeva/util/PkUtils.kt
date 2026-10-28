package com.yy.yyeva.util

import android.content.Context
import android.content.pm.PackageManager
import android.text.TextUtils


object PkUtils {
    const val TAG = "PkUtils"
    fun getPackageName(context: Context): String? {
        try {
            val pm = context.packageManager
            val pkInfo = pm.getPackageInfo(context.packageName, 0)
            val applicationInfo = pkInfo.applicationInfo
            return applicationInfo.packageName
        } catch (e: Exception) {
            ELog.e(TAG, e.toString())
        }
        return null
    }

    fun getAppName(context: Context): String? {
        val pm = context.packageManager
        try {
            val packageInfo = pm.getPackageInfo(context.packageName, 0)
            val applicationInfo = packageInfo.applicationInfo
            // 获取应用名称的方式可能因应用而异，以下是两种常见情况
            // 情况一：如果应用有label属性设置，通过loadLabel获取名称
            val appLabel = pm.getApplicationLabel(applicationInfo)
            if (!TextUtils.isEmpty(appLabel)) {
                return appLabel.toString()
            }
            // 情况二：如果label属性未设置，尝试从applicationInfo的name属性获取
            val appName = applicationInfo.name
            if (!TextUtils.isEmpty(appName)) {
                return appName
            }
        } catch (e: PackageManager.NameNotFoundException) {
            e.printStackTrace()
        }
        return null
    }
}