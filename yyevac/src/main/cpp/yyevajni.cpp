#include <jni.h>
#include <string>

#include <android/bitmap.h>
#include <android/native_window_jni.h>
#include <engine/rendercontroller.h>
#include <record/recordercontroller.h>
#include <util/elog.h>
#include <map>
#include <mutex>

#define LOG_TAG "YYEVAJNI"
#define ELOGE(...) yyeva::ELog::get()->e(LOG_TAG, __VA_ARGS__)
#define ELOGV(...) yyeva::ELog::get()->i(LOG_TAG, __VA_ARGS__)

#define YYEVA(sig) Java_com_yy_yyeva_util_EvaJniUtil_##sig


using namespace std;
using namespace yyeva;
map<int, shared_ptr<RenderController>> renderMap;
map<int, shared_ptr<RecorderController>> recordMap;
int renderId = 0;
mutex mtx;
jobject elog;

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
        jobject instance, jint controllerId) {
    if (renderMap.find(controllerId) == renderMap.end()) {
        ELOGE("getExternalTexture controller %d not found", controllerId);
        return -1; //异常
    }
    return renderMap[controllerId]->getExternalTexture();
}


JNIEXPORT void JNICALL YYEVA(updateViewPoint)(
        JNIEnv *env,
        jobject instance, jint controllerId,
        jint width, jint height) {
    if (controllerId == -1) {
        ELOGE("updateViewPoint controller not init");
        return;
    }
    if (renderMap.find(controllerId) == renderMap.end()) {
        ELOGE("updateViewPoint controller %d not found", controllerId);
        return;
    }
    if (width>0 && height>0) {
        renderMap[controllerId]->updateViewPoint(width, height);
    }
}

JNIEXPORT void JNICALL YYEVA(videoSizeChange)(
        JNIEnv *env,
        jobject instance, jint controllerId,
        jint newWidth, jint newHeight) {
    if (controllerId == -1) {
        ELOGE("videoSizeChange controller not init");
        return;
    }
    if (renderMap.find(controllerId) == renderMap.end()) {
        ELOGE("videoSizeChange controller %d not found", controllerId);
        return;
    }
    renderMap[controllerId]->videoSizeChange(newWidth, newHeight);
}

JNIEXPORT jint JNICALL YYEVA(initRender)(
        JNIEnv *env, jobject instance, jint controllerId,
        jobject surface, jboolean isNeedYUV, jboolean isNormalMp4, jboolean isVideoRecord) {
    ELOGV("initRender");
    lock_guard<mutex> auto_lock(mtx);
    //创建window
    ANativeWindow *window = ANativeWindow_fromSurface(env,surface);
    if (window == nullptr) {
        ELOGE("window is nullptr");
        return controllerId;
    }
    int id = controllerId;
    if (controllerId == -1) {
        renderId += 1;
        id = renderId;
        auto* controller = new RenderController();
        controller->initRender(window, isNeedYUV, isNormalMp4);
        renderMap.insert(make_pair(id, controller));
        controller->setVideoRecord(isVideoRecord);
    } else if (renderMap.find(controllerId) != renderMap.end()) {
        if (renderMap[controllerId]->getExternalTexture() == -1) {  //防止重复初始化
            renderMap[controllerId]->initRender(window, isNeedYUV, isNormalMp4);
            renderMap[controllerId]->setVideoRecord(isVideoRecord);
        } else {
            ELOGE("initRender init repeat");
        }
    } else {
        ELOGE("initRender controller %d not found", controllerId);
    }

    return id;

//    if (controller == nullptr) {
//        controller = new RenderController();
//    }
//    if (controller != nullptr && controller->getExternalTexture() == -1) {
//        return controller->initRender(window, isNeedYUV);
//    } else {
//        return controller->getExternalTexture();
//    }
}

JNIEXPORT void JNICALL YYEVA(setRenderConfig)(
        JNIEnv *env, jobject instance, jint controllerId, jstring json) {
    const char *cJson = env->GetStringUTFChars(json, JNI_FALSE);
    if (controllerId == -1) {
        ELOGE("setRenderConfig controller not init");
        return;
    }
    if (renderMap.find(controllerId) == renderMap.end()) {
        ELOGE("setRenderConfig controller %d not found", controllerId);
        return;
    }
    auto config = EvaAnimeConfig::parse(cJson);
    if (config != nullptr) {
        renderMap[controllerId]->setRenderConfig(config);
    }
    env->ReleaseStringUTFChars(json, cJson);
}

JNIEXPORT void JNICALL YYEVA(defaultConfig)(
        JNIEnv *env, jobject instance, jint controllerId, jint _videoWidth, jint _videoHeight, jint _defaultVideoMode) {
    if (controllerId == -1) {
        ELOGE("defaultConfig controller not init");
        return;
    }
    if (renderMap.find(controllerId) == renderMap.end()) {
        ELOGE("defaultConfig controller %d not found", controllerId);
        return;
    }
    shared_ptr<EvaAnimeConfig> config = EvaAnimeConfig::defaultConfig((int)_videoWidth, (int)_videoHeight, (int)_defaultVideoMode);

    if (config != nullptr) {
        renderMap[controllerId]->setRenderConfig(config);
    }
}

JNIEXPORT void JNICALL YYEVA(renderFrame)(
        JNIEnv *env, jobject instance, jint controllerId) {
    lock_guard<mutex> auto_lock(mtx);
    if (controllerId == -1) {
        ELOGE("renderFrame controller not init");
        return;
    }
    if (renderMap.find(controllerId) == renderMap.end()) {
        ELOGE("renderFrame controller %d not found", controllerId);
        return;
    }
    renderMap[controllerId]->renderFrame();
}

JNIEXPORT void JNICALL YYEVA(renderSwapBuffers)(
        JNIEnv *env, jobject instance, jint controllerId) {
    if (controllerId == -1) {
        ELOGE("renderSwapBuffers controller not init");
        return;
    }
    if (renderMap.find(controllerId) == renderMap.end()) {
        ELOGE("renderSwapBuffers controller %d not found", controllerId);
        return;
    }
    renderMap[controllerId]->renderSwapBuffers();
}

JNIEXPORT void JNICALL YYEVA(renderClearFrame)(
        JNIEnv *env, jobject instance, jint controllerId) {
    if (controllerId == -1) {
        ELOGE("renderClearFrame controller not init");
        return;
    }
    if (renderMap.find(controllerId) == renderMap.end()) {
        ELOGE("renderClearFrame controller %d not found", controllerId);
        return;
    }
    renderMap[controllerId]->renderClearFrame();
}

JNIEXPORT void JNICALL YYEVA(releaseTexture)(
        JNIEnv *env, jobject instance, jint controllerId) {
    if (controllerId == -1) {
        ELOGE("releaseTexture controller not init");
        return;
    }
    if (renderMap.find(controllerId) == renderMap.end()) {
        ELOGE("releaseTexture controller %d not found", controllerId);
        return;
    }

    renderMap[controllerId]->releaseTexture();
}

JNIEXPORT void JNICALL YYEVA(destroyRender)(
        JNIEnv *env, jobject instance, jint controllerId) {
    lock_guard<mutex> auto_lock(mtx);
    if (controllerId == -1) {
        ELOGE("destroyRender controller not init");
        return;
    }
    if (renderMap.find(controllerId) == renderMap.end()) {
        ELOGE("destroyRender controller %d not found", controllerId);
        return;
    }
    renderMap[controllerId]->destroyRender();
    renderMap.erase(controllerId);
}

JNIEXPORT jint JNICALL YYEVA(mixConfigCreate)(
        JNIEnv *env, jobject instance, jint controllerId, jstring json) {
    lock_guard<mutex> auto_lock(mtx);
    const char *cJson = env->GetStringUTFChars(json, JNI_FALSE);
    shared_ptr<EvaAnimeConfig> config = EvaAnimeConfig::parse(cJson);
    int id = controllerId;
    if (controllerId == -1) {
        ELOGV("mixConfigCreate controller not init");
        renderId += 1;
        id = renderId;
//        auto* controller = new RenderController();
        auto controller = make_shared<RenderController>();
        renderMap.insert(make_pair(id, controller));
        renderMap[id]->mixConfigCreate(config);
    } else if (renderMap.find(controllerId) != renderMap.end()){
        renderMap[controllerId]->mixConfigCreate(config);
    } else {
        ELOGE("mixConfigCreate controller %d not found", controllerId);
    }
    env->ReleaseStringUTFChars(json, cJson);
    return id;
}

JNIEXPORT void JNICALL YYEVA(setBgBitmap)(
        JNIEnv *env, jobject instance, jint controllerId, jobject bitmap) {
    if (controllerId == -1) {
        ELOGE("setBgBitmap controller not init");
        return;
    }
    if (renderMap.find(controllerId) == renderMap.end()) {
        ELOGE("setBgBitmap controller %d not found", controllerId);
        return;
    }
    if (bitmap == NULL) {
        return;
    }

    AndroidBitmapInfo bitmapInfo;
    memset(&bitmapInfo, 0, sizeof(bitmapInfo));
    int result = AndroidBitmap_getInfo(env, bitmap, &bitmapInfo);
    if (result != ANDROID_BITMAP_RESULT_SUCCESS) {
        AndroidBitmap_unlockPixels(env, bitmap);
        return;
    }

    unsigned char *pixels;
    result = AndroidBitmap_lockPixels(env, bitmap, reinterpret_cast<void **>(&pixels));
    if (result != ANDROID_BITMAP_RESULT_SUCCESS) {
        AndroidBitmap_unlockPixels(env, bitmap);
        return;
    }

    renderMap[controllerId]->setBgImage(pixels, &bitmapInfo);

    AndroidBitmap_unlockPixels(env, bitmap);
}


JNIEXPORT void JNICALL YYEVA(setSrcBitmap)(
        JNIEnv *env, jobject instance, jint controllerId, jstring srcId, jobject bitmap, jstring scaleMode) {
    if (controllerId == -1) {
        ELOGE("setSrcBitmap controller not init");
        return;
    }
    if (renderMap.find(controllerId) == renderMap.end()) {
        ELOGE("setBgBitmap controller %d not found", controllerId);
        return;
    }
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

    const char *id = env->GetStringUTFChars(srcId, JNI_FALSE);
    const string scaleM = env->GetStringUTFChars(scaleMode, JNI_FALSE);
    renderMap[controllerId]->setSrcBitmap(id, pixels, &bitmapInfo, scaleM);

    AndroidBitmap_unlockPixels(env, bitmap);
}

JNIEXPORT void JNICALL YYEVA(setSrcTxt)(
        JNIEnv *env, jobject instance, jint controllerId, jstring srcId, jstring txt) {
    if (txt == NULL) {
        return;
    }
    if (controllerId == -1) {
        ELOGE("setSrcTxt controller not init");
        return;
    }
    if (renderMap.find(controllerId) == renderMap.end()) {
        ELOGE("setSrcTxt controller %d not found", controllerId);
        return;
    }

    const char *id = env->GetStringUTFChars(srcId, JNI_FALSE);
    const char *t = env->GetStringUTFChars(txt, JNI_FALSE);
    renderMap[controllerId]->setSrcTxt(id, t);
}


JNIEXPORT void JNICALL YYEVA(mixRenderCreate)(
        JNIEnv *env, jobject instance, jint controllerId) {
    if (controllerId == -1) {
        ELOGE("mixRenderCreate controller not init");
        return;
    }
    if (renderMap.find(controllerId) == renderMap.end()) {
        ELOGE("mixRenderCreate controller %d not found", controllerId);
        return;
    }
    renderMap[controllerId]->mixRenderCreate();
}

JNIEXPORT void JNICALL YYEVA(mixRendering) (
        JNIEnv *env, jobject instance, jint controllerId, jint frameIndex) {
    lock_guard<mutex> auto_lock(mtx);
    if (controllerId == -1) {
        ELOGE("mixRendering controller not init");
        return;
    }
    if (renderMap.find(controllerId) == renderMap.end()) {
        ELOGE("mixRendering controller %d not found", controllerId);
        return;
    }
    renderMap[controllerId]->mixRendering(frameIndex);
}

JNIEXPORT void JNICALL YYEVA(mixRenderDestroy)(
        JNIEnv *env, jobject instance, jint controllerId) {
    lock_guard<mutex> auto_lock(mtx);
    if (controllerId == -1) {
        ELOGE("mixRenderDestroy controller not init");
        return;
    }
    if (renderMap.find(controllerId) == renderMap.end()) {
        ELOGE("mixRenderDestroy controller %d not found", controllerId);
        return;
    }
    renderMap[controllerId]->mixRenderDestroy();
}

JNIEXPORT void JNICALL YYEVA(setVideoRecord)(
        JNIEnv *env, jobject instance, jint controllerId, jboolean isVideoRecord) {
    if (controllerId == -1) {
        ELOGE("setVideoRecord controller not init");
        return;
    }
    if (renderMap.find(controllerId) == renderMap.end()) {
        ELOGE("setVideoRecord controller %d not found", controllerId);
        return;
    }
    renderMap[controllerId]->setVideoRecord(isVideoRecord);
}

JNIEXPORT jint JNICALL YYEVA(initVideoRecord) (
        JNIEnv *env, jobject instance, jint controllerId, jobject surface) {
    lock_guard<mutex> auto_lock(mtx);
    ANativeWindow *window = ANativeWindow_fromSurface(env,surface);
    if (window == nullptr) {
        ELOGE("window is nullptr");
        return -1;
    }
    if (renderMap.find(controllerId) != renderMap.end()) {
        auto record = make_shared<RecorderController>();
        recordMap.insert(make_pair(controllerId, record));
        recordMap[controllerId]->init(renderMap[controllerId]->getEglContext(), window,
                                      renderMap[controllerId]->getRecordFramebufferId());
    } else {
        ELOGE("initVideoRecord controller %d not found", controllerId);
    }

    return -1;
}

JNIEXPORT void JNICALL YYEVA(setRecordRenderConfig)(
        JNIEnv *env, jobject instance, jint controllerId, jstring json) {
    const char *cJson = env->GetStringUTFChars(json, JNI_FALSE);
    if (controllerId == -1) {
        ELOGE("setRecordRenderConfig controller not init");
        return;
    }
    if (recordMap.find(controllerId) == recordMap.end()) {
        ELOGE("setRecordRenderConfig controller %d not found", controllerId);
        return;
    }
    shared_ptr<EvaAnimeConfig> config = EvaAnimeConfig::parse(cJson);
    if (config != nullptr) {
        recordMap[controllerId]->setRenderConfig(config);
    }
    env->ReleaseStringUTFChars(json, cJson);
}

JNIEXPORT void JNICALL YYEVA(recordRender)(
        JNIEnv *env, jobject instance, jint controllerId, jlong time) {
    lock_guard<mutex> auto_lock(mtx);
    if (controllerId == -1) {
        ELOGE("recordRender controller not init");
        return;
    }
    if (renderMap.find(controllerId) == renderMap.end()) {
        ELOGE("recordRender renderMap controller %d not found", controllerId);
        return;
    }
    if (recordMap.find(controllerId) == recordMap.end()) {
        ELOGE("recordRender recordRender controller %d not found", controllerId);
        return;
    }

    recordMap[controllerId]->renderFrame(
            renderMap[controllerId]->getRecordFramebufferId(),
            static_cast<uint64_t>(time));
}

JNIEXPORT void JNICALL YYEVA(stopRecord)(
        JNIEnv *env, jobject instance, jint controllerId) {
    lock_guard<mutex> auto_lock(mtx);
    if (controllerId == -1) {
        ELOGE("stopRecord controller not init");
        return;
    }
    if (recordMap.find(controllerId) == recordMap.end()) {
        ELOGE("stopRecord controller %d not found", controllerId);
        return;
    }
    recordMap[controllerId]->destroy();
}

JNIEXPORT void JNICALL YYEVA(setLog)(
        JNIEnv *env,
        jobject instance, jobject log) {
    ELOGV("setLog");

    JavaVM *javaVm;
    if (env->GetJavaVM(&javaVm) != JNI_OK) {
        return;
    }
    if (elog) {
        env->DeleteGlobalRef(elog);
    }
    elog = env->NewGlobalRef(log);

    ELog::get()->setJavaVM(javaVm);
    ELog::get()->setELog(elog);
}

}
