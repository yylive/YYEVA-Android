//
// Created by zengjiale on 2022/8/15.
//
#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/bitmap.h>
#include <android/native_window_jni.h>
#include <src/main/cpp/control/nativeplayer.h>

#define LOG_TAG "YYEVAJNI"
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ELOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

#define YYEVA(sig) Java_com_yy_yyevav2_util_EvaJniUtil_##sig

INativePlayer *player;
jobject listener = nullptr;

extern "C" {

JNIEXPORT jint JNICALL YYEVA(nativeCreatePlayer)(
        JNIEnv *env,
        jobject instance, jstring tag, jstring optionParams) {
    if (player == nullptr) {
        player = new NativePlayer();
        const char* paramsStr = env->GetStringUTFChars(optionParams, JNI_FALSE);
        auto *params = new OptionParams();
        params->parse(paramsStr);
        player->setParams(params);
        env->ReleaseStringUTFChars(optionParams, paramsStr);
        return 0;
    }
    return -1; //异常
}

JNIEXPORT jint JNICALL YYEVA(nativePlay)(
        JNIEnv *env,
        jobject instance, jstring optionParams) {
    if (player != nullptr && optionParams != nullptr) {
        const char* paramsStr = env->GetStringUTFChars(optionParams, JNI_FALSE);
        auto *params = new OptionParams();
        params->parse(paramsStr);

        player->setParams(params);
        player->startPlay(params->mp4Address);
        env->ReleaseStringUTFChars(optionParams, paramsStr);
        return 0;
    }
    return -1; //异常
}

JNIEXPORT jboolean JNICALL YYEVA(nativeIsRunning)(
        JNIEnv *env,
        jobject instance, jint nativePlayer) {
    if (player != nullptr) {
        return player->isPlaying();
    }
    return 0; //异常
}

JNIEXPORT jint JNICALL YYEVA(nativeStop)(
        JNIEnv *env,
        jobject instance, jint nativePlayer) {
    if (player != nullptr) {
        player->stop();
    }
    return -1; //异常
}

JNIEXPORT void JNICALL YYEVA(nativeRenderData)(
        JNIEnv *env,
        jobject instance) {
    if (player != nullptr) {
        player->renderFrame();
    }
}

JNIEXPORT jint JNICALL YYEVA(nativeRelease)(
        JNIEnv *env,
        jobject instance, jint nativePlayer) {
    if (player != nullptr) {
        player->release();
        player = nullptr;
    }
    if (listener) {
        env->DeleteGlobalRef(listener);
        listener = nullptr;
    }
    return 0;
}

JNIEXPORT jint JNICALL YYEVA(nativeDestroySurface)(
        JNIEnv *env,
        jobject instance, jint nativePlayer) {

    return -1; //异常
}

JNIEXPORT jint JNICALL YYEVA(nativeSetSurface)(
        JNIEnv *env,
        jobject instance, jint nativePlayer, jobject surface) {
    //创建window
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    if (window == nullptr) {
        ELOGE("window is nullptr");
        return -1;
    }
    if (player != nullptr) {
        player->setSurface(window);
    }
    if (player != nullptr) {
        return player->getTextureId();
    }
    return -1; //异常
}

JNIEXPORT void JNICALL YYEVA(nativeSetDecodeSurface)(
        JNIEnv *env,
        jobject instance, jint nativePlayer, jobject surface) {
    //创建window
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    if (window == nullptr) {
        ELOGE("window is nullptr");
    }
    if (player != nullptr) {
        player->setDecodeSurface(window);
    }
}

JNIEXPORT void JNICALL YYEVA(nativeUpdateViewPoint)(
        JNIEnv *env,
        jobject instance, jint nativePlayer, jint width, jint height) {
    if (player != nullptr) {
        player->updateViewPoint(width, height);
    }
}

JNIEXPORT void JNICALL YYEVA(nativeSetSourceParams)(
        JNIEnv *env,
        jobject instance, jstring params) {
//    if (player != nullptr) {
//        player->startFrameDecode()
//    }
}

/**
 * 添加元素图片
 * @param env
 * @param instance
 * @param name
 * @param bitmap
 */
JNIEXPORT void JNICALL YYEVA(nativeAddEffect)(
        JNIEnv *env,
        jobject instance, jstring name, jobject bitmap, jstring scaleMode) {
    if (bitmap == NULL) {
        return;
    }
    AndroidBitmapInfo bitmapInfo;
    memset(&bitmapInfo, 0, sizeof(bitmapInfo));
    int result = AndroidBitmap_getInfo(env, bitmap, &bitmapInfo);
    if (result != ANDROID_BITMAP_RESULT_SUCCESS) {
        return;
    }

    unsigned char *pixels;
    result = AndroidBitmap_lockPixels(env, bitmap, reinterpret_cast<void **>(&pixels));
    if (result != ANDROID_BITMAP_RESULT_SUCCESS) {
        return;
    }
    const char* nameStr = env->GetStringUTFChars(name, JNI_FALSE);
    if (player != nullptr) {
        const std::string scaleM = env->GetStringUTFChars(scaleMode, JNI_FALSE);
        player->setSrcBitmap(string(nameStr), pixels, &bitmapInfo, scaleM);
    }
    AndroidBitmap_unlockPixels(env, bitmap);
    env->ReleaseStringUTFChars(name, nameStr);
}


JNIEXPORT void JNICALL YYEVA(nativeSetEffectParams)(
        JNIEnv *env,
        jobject instance, jstring params) {
}

JNIEXPORT void JNICALL YYEVA(nativeSetPlayParams)(
        JNIEnv *env,
        jobject instance, jstring params) {
}

JNIEXPORT void JNICALL YYEVA(nativeSetEvaAnimListener)(
        JNIEnv *env,
        jobject instance, jobject evaAnimListener) {
    if (player != nullptr && evaAnimListener != nullptr) {
        JavaVM *javaVm;
        if (env->GetJavaVM(&javaVm) != JNI_OK) {
            return;
        }
        if (listener) {
            env->DeleteGlobalRef(listener);
        }
        listener = env->NewGlobalRef(evaAnimListener);
        player->setAnimListener(javaVm, listener);
    }
}

JNIEXPORT void JNICALL YYEVA(nativeTouchPoint)(
        JNIEnv *env,
        jobject instance, jint x, jint y) {
    if (player != nullptr) {
        player->touchPoint(x, y);
    }
}

}
