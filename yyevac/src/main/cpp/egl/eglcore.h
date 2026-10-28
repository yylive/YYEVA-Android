

#pragma once
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3.h>
#include <EGL/eglext.h>

#include <android/native_window.h>
#include <util/elog.h>

/**
 * cangwang 2018.12.1
 */
typedef EGLBoolean (EGLAPIENTRYP EGL_PRESENTATION_TIME_ANDROIDPROC)(EGLDisplay display, EGLSurface surface, khronos_stime_nanoseconds_t time);
using namespace yyeva;
class EGLCore {
public:
    EGLCore();

    ~EGLCore();

    bool start(ANativeWindow *window);

    //使用共享context，用于录制
    void start(EGLContext context, ANativeWindow *window);

    EGLConfig chooseConfig();

    EGLConfig chooseRecordConfig();

    EGLint* getAttributes();

    EGLContext createContext(EGLDisplay eglDisplay, EGLConfig eglConfig);

    //使用共享context，用于录制
    EGLContext createContext(EGLDisplay eglDisplay, EGLConfig eglConfig, EGLContext context);

    GLboolean buildContext(ANativeWindow *window);

    void setPresentationTime(uint64_t nsecs);

    void swapBuffer();

    void release();

    EGLContext getEglContext();

protected:

private:
    EGLDisplay mDisplay;
    EGLSurface mSurface;
    EGLContext mContext;
    ANativeWindow* window;
    EGL_PRESENTATION_TIME_ANDROIDPROC eglPresentationTimeANDROID = NULL;
};