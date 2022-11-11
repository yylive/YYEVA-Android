//
// Created by zengjiale on 2022/8/29.
//

#include "evalistenermanager.h"

EvaListenerManager::~EvaListenerManager() {
    listener = nullptr;
    javaVM = nullptr;
}

bool EvaListenerManager::onVideoConfigReady(list<EvaSrc> &dataList) {
    ELOGV("onVideoConfigReady");
    JNIEnv *env;
    bool isNeedDetach = false;
    if (!attachCurrentThread(env, isNeedDetach)) {
        return false;
    }
    jclass list_cls = env->FindClass("java/util/ArrayList");
    jmethodID list_init_method = env->GetMethodID(list_cls, "<init>", "()V");
    jobject list_obj = env->NewObject(list_cls, list_init_method);
    jmethodID list_add_method = env->GetMethodID(list_cls, "add", "(Ljava/lang/Object;)Z");

    list<EvaSrc>::iterator it;
    for (it = dataList.begin(); it != dataList.end(); it++) {
        jobject evaSrcSimObj = createEvaSrcSimObj(env, *it);
        env->CallBooleanMethod(list_obj, list_add_method, evaSrcSimObj);
        env->DeleteLocalRef(evaSrcSimObj);
    }
    jclass clazz_listener = env->GetObjectClass(listener);
    jmethodID methodId = env->GetMethodID(clazz_listener, "onVideoConfigReady",
                                          "(Ljava/util/ArrayList;)Z");
    env->CallBooleanMethod(listener, methodId, list_obj);
    env->DeleteLocalRef(list_obj);
    if (isNeedDetach) {
        javaVM->DetachCurrentThread();
    }
    return true;
}

void EvaListenerManager::onVideoStart() {
    ELOGV("onVideoStart");
    callVoidMethod("onVideoStart", "()V");
}

void EvaListenerManager::onVideoRender(int frameIndex) {
    ELOGV("onVideoRender %i",frameIndex);
    callVoidMethod("onVideoRender", "(I)V", frameIndex);
}

void EvaListenerManager::onVideoComplete() {
    ELOGV("onVideoComplete");
    callVoidMethod("onVideoComplete", "()V");
}

void EvaListenerManager::onVideoDestroy() {
    ELOGV("onVideoDestroy");
    callVoidMethod("onVideoDestroy", "()V");
}

void EvaListenerManager::onFailed(int errorType, string errorMsg) {
    JNIEnv *env;
    bool isNeedDetach = false;
    if (!attachCurrentThread(env, isNeedDetach)) {
        return;
    }
    jstring jmsg = env->NewStringUTF(errorMsg.c_str());
    jclass clazz_listener = env->GetObjectClass(listener);
    jmethodID methodId = env->GetMethodID(clazz_listener, "onFailed", "(ILjava/lang/String;)V");
    env->CallVoidMethod(listener, methodId, errorType, jmsg);
    env->DeleteLocalRef(jmsg);
    if (isNeedDetach) {
        javaVM->DetachCurrentThread();
    }
}

void EvaListenerManager::touchCallback(int x, int y, EvaSrc &evaSrcSim) {
    JNIEnv *env;
    bool isNeedDetach = false;
    if (!attachCurrentThread(env, isNeedDetach)) {
        return;
    }
    jclass clazz_listener = env->GetObjectClass(listener);
    jmethodID methodId = env->GetMethodID(clazz_listener, "touchCallback",
                                          "(IILcom/yy/yyevav2/impl/EvaSrcSim;)V");
    jobject evaSrcObj = createEvaSrcSimObj(env, evaSrcSim);
    env->CallVoidMethod(listener, methodId, x, y, evaSrcObj);
    env->DeleteLocalRef(evaSrcObj);
    if (isNeedDetach) {
        javaVM->DetachCurrentThread();
    }
}


void
EvaListenerManager::callVoidMethod(const char *methodName,
                                   const char *sign, ...) {
    JNIEnv *env;
    bool isNeedDetach = false;
    if (!attachCurrentThread(env, isNeedDetach)) {
        return;
    }
    jclass clazz_listener = env->GetObjectClass(listener);
    jmethodID methodId = env->GetMethodID(clazz_listener, methodName, sign);
    va_list ap;
    va_start(ap, sign);
    env->CallVoidMethodV(listener, methodId, ap);
    va_end(ap);
    if (isNeedDetach) {
        javaVM->DetachCurrentThread();
    }
}

bool EvaListenerManager::attachCurrentThread(JNIEnv *&env, bool &isNeedDetach) {
    if (!javaVM) {
        return false;
    }
    isNeedDetach = false;
//    ELOGV("EvaListenerManager::attachCurrentThread");
    jint res = javaVM->GetEnv((void **) &env, JNI_VERSION_1_6);
    if (res != JNI_OK) {
        res = javaVM->AttachCurrentThread(&env, nullptr);
        if (JNI_OK != res) {
            return false;
        }
        isNeedDetach = true;
    }
    return true;
}

jobject EvaListenerManager::createEvaSrcSimObj(JNIEnv *env, EvaSrc &evaSrcSim) {
    jclass evaSrcSim_class = env->FindClass("com/yy/yyevav2/impl/EvaSrcSim");
    jmethodID evaSrcSimInit_method = env->GetMethodID(evaSrcSim_class, "<init>", "()V");
    jobject evaSrcSim_obj = env->NewObject(evaSrcSim_class, evaSrcSimInit_method);
    jfieldID w_id = env->GetFieldID(evaSrcSim_class, "w", "I");
    env->SetIntField(evaSrcSim_obj, w_id, evaSrcSim.w);

    jfieldID h_id = env->GetFieldID(evaSrcSim_class, "h", "I");
    env->SetIntField(evaSrcSim_obj, h_id, evaSrcSim.h);

    jfieldID bold_id = env->GetFieldID(evaSrcSim_class, "bold", "Z");
    env->SetBooleanField(evaSrcSim_obj, bold_id, false);

    jstring jTxt = env->NewStringUTF(evaSrcSim.txt.c_str());
    jfieldID txt_id = env->GetFieldID(evaSrcSim_class, "txt", "Ljava/lang/String;");
    env->SetObjectField(evaSrcSim_obj, txt_id, jTxt);
    env->DeleteLocalRef(jTxt);

    jstring jColor = env->NewStringUTF(evaSrcSim.fontColor.c_str());
    jfieldID color_id = env->GetFieldID(evaSrcSim_class, "color", "Ljava/lang/String;");
    env->SetObjectField(evaSrcSim_obj, color_id, jColor);
    env->DeleteLocalRef(jColor);

    jstring jScaleMode = env->NewStringUTF(evaSrcSim.fontColor.c_str());
    jfieldID scaleMode_id = env->GetFieldID(evaSrcSim_class, "scaleMode", "Ljava/lang/String;");
    env->SetObjectField(evaSrcSim_obj, scaleMode_id, jScaleMode);
    env->DeleteLocalRef(jScaleMode);

    jfieldID fontSize_id = env->GetFieldID(evaSrcSim_class, "fontSize", "I");
    env->SetIntField(evaSrcSim_obj, fontSize_id, evaSrcSim.fontSize);

    jstring jTextAlign = env->NewStringUTF(evaSrcSim.fontColor.c_str());
    jfieldID textAlign_id = env->GetFieldID(evaSrcSim_class, "textAlign", "Ljava/lang/String;");
    env->SetObjectField(evaSrcSim_obj, textAlign_id, jTextAlign);
    env->DeleteLocalRef(jTextAlign);

    jstring jsrcTag = env->NewStringUTF(evaSrcSim.srcTag.c_str());
    jfieldID srcTag_id = env->GetFieldID(evaSrcSim_class, "srcTag", "Ljava/lang/String;");
    env->SetObjectField(evaSrcSim_obj, srcTag_id, jsrcTag);
    env->DeleteLocalRef(jsrcTag);

    jstring jsrcId = env->NewStringUTF(evaSrcSim.srcId.c_str());
    jfieldID srcId_id = env->GetFieldID(evaSrcSim_class, "srcId", "Ljava/lang/String;");
    env->SetObjectField(evaSrcSim_obj, srcId_id, jsrcId);
    env->DeleteLocalRef(jsrcId);

    jstring jsrcType = env->NewStringUTF(evaSrcSim.srcTypeStr.c_str());
    jfieldID srcType_id = env->GetFieldID(evaSrcSim_class, "srcType", "Ljava/lang/String;");
    env->SetObjectField(evaSrcSim_obj, srcType_id, jsrcType);
    env->DeleteLocalRef(jsrcType);

    return evaSrcSim_obj;
}

