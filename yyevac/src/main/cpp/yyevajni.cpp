#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/bitmap.h>
#include <android/native_window_jni.h>
#include <engine/rendercontroller.h>

#define LOG_TAG "YYEVAJNI"
#define ELOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define ELOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

#define YYEVA(sig) Java_com_yy_yyeva_util_EvaJniUtil_##sig

RenderController* controller;

extern "C"{

JNIEXPORT void JNICALL YYEVA(nativeProcessBitmap)(
        JNIEnv *env,
        jobject instance,
        jobject bitmap) {
    if (bitmap == NULL) {
        return;
    }
    AndroidBitmapInfo  bitmapInfo;
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
//    AndroidBitmap_unlockPixels(env, bitmap);
    AndroidBitmap_unlockPixels(env, bitmap);
}

JNIEXPORT jint JNICALL YYEVA(getExternalTexture)(
        JNIEnv *env,
        jobject instance) {
    if (controller != nullptr) {
        return controller->getExternalTexture();
    }
    return -1; //异常
}


JNIEXPORT void JNICALL YYEVA(updateViewPoint)(
        JNIEnv *env,
        jobject instance,
        jint width, jint height) {
    if (controller != nullptr && width>0 && height>0) {
        controller->updateViewPoint(width, height);
    }
}

JNIEXPORT void JNICALL YYEVA(videoSizeChange)(
        JNIEnv *env,
        jobject instance,
        jint newWidth, jint newHeight) {
    if (controller != nullptr) {
        controller->videoSizeChange(newWidth, newHeight);
    }
}

JNIEXPORT jint JNICALL YYEVA(initRender)(
        JNIEnv *env, jobject instance, jobject surface, jboolean isNeedYUV) {
    //创建window
    ANativeWindow *window = ANativeWindow_fromSurface(env,surface);
    if (window == nullptr) {
        ELOGE("window is nullptr");
        return -1;
    }
    if (controller == nullptr) {
        controller = new RenderController();
    }
    if (controller != nullptr && controller->getExternalTexture() == -1) {
        return controller->initRender(window, isNeedYUV);
    } else {
        return controller->getExternalTexture();
    }
}

JNIEXPORT void JNICALL YYEVA(setRenderConfig)(
        JNIEnv *env, jobject instance, jstring json) {
    const char *cJson = env->GetStringUTFChars(json, JNI_FALSE);
    EvaAnimeConfig* config = EvaAnimeConfig::parse(cJson);
    if (controller != nullptr && config != NULL) {
        controller->setRenderConfig(config);
    }
}

JNIEXPORT void JNICALL YYEVA(defaultConfig)(
        JNIEnv *env, jobject instance, jint _videoWidth, jint _videoHeight, jint _defaultVideoMode) {
    EvaAnimeConfig* config = EvaAnimeConfig::defaultConfig((int)_videoWidth, (int)_videoHeight, (int)_defaultVideoMode);
    if (controller != nullptr && config != NULL) {
        controller->setRenderConfig(config);
    }
}

JNIEXPORT void JNICALL YYEVA(renderFrame)(
        JNIEnv *env, jobject instance) {
    if (controller != nullptr) {
        controller->renderFrame();
    }
}

JNIEXPORT void JNICALL YYEVA(renderSwapBuffers)(
        JNIEnv *env, jobject instance) {
    if (controller != nullptr) {
        controller->renderSwapBuffers();
    }
}

JNIEXPORT void JNICALL YYEVA(renderClearFrame)(
        JNIEnv *env, jobject instance) {
    if (controller != nullptr) {
        controller->renderClearFrame();
    }
}

JNIEXPORT void JNICALL YYEVA(releaseTexture)(
        JNIEnv *env, jobject instance) {
    if (controller != nullptr) {
        controller->releaseTexture();
    }
}

JNIEXPORT void JNICALL YYEVA(destroyRender)(
        JNIEnv *env, jobject instance) {
    if (controller != nullptr) {
        controller->destroyRender();
        controller = nullptr;
    }
}

JNIEXPORT void JNICALL YYEVA(mixConfigCreate)(
        JNIEnv *env, jobject instance, jstring json) {
    const char *cJson = env->GetStringUTFChars(json, JNI_FALSE);
    EvaAnimeConfig* config = EvaAnimeConfig::parse(cJson);
    if (controller == nullptr) {
        controller = new RenderController();
    }
    if (controller != nullptr) {
        controller->mixConfigCreate(config);
    }
}

JNIEXPORT void JNICALL YYEVA(setSrcBitmap)(
        JNIEnv *env, jobject instance, jstring srcId, jobject bitmap, jstring address) {
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
    if (controller != nullptr) {
        const char *id = env->GetStringUTFChars(srcId, JNI_FALSE);
        std::string addr = env->GetStringUTFChars(address, JNI_FALSE);
        controller->setSrcBitmap(id, pixels, &bitmapInfo, addr);
    }
//    AndroidBitmap_unlockPixels(env, bitmap);
    AndroidBitmap_unlockPixels(env, bitmap);
}

JNIEXPORT void JNICALL YYEVA(setSrcTxt)(
        JNIEnv *env, jobject instance, jstring srcId, jstring txt) {
    if (txt == NULL) {
        return;
    }

    if (controller != nullptr) {
        const char *id = env->GetStringUTFChars(srcId, JNI_FALSE);
        const char *t = env->GetStringUTFChars(txt, JNI_FALSE);
        controller->setSrcTxt(id, t);
    }
}


JNIEXPORT void JNICALL YYEVA(mixRenderCreate)(
        JNIEnv *env, jobject instance) {
    if (controller != nullptr) {
        controller->mixRenderCreate();
    }
}

JNIEXPORT void JNICALL YYEVA(mixRendering)(
        JNIEnv *env, jobject instance, jint frameIndex) {
    if (controller != nullptr) {
        controller->mixRendering(frameIndex);
    }
}

JNIEXPORT void JNICALL YYEVA(mixRenderDestroy)(
        JNIEnv *env, jobject instance) {
    if (controller != nullptr) {
        controller->mixRenderDestroy();
    }
}

}
